#include "NifLoader.hpp"

#include <algorithm>
#include <filesystem>
#include <span>
#include <sstream>
//
#include <NifFile.hpp>
#include <TGEngine.hpp>
#include <Util.hpp>
#include <bsa/tes4.hpp>

namespace tge::nif {

using namespace tge::graphics;
using namespace tge::main;
using namespace tge::shader;

NifModule* nifModule = new nif::NifModule();

static std::vector<bsa::tes4::archive> archivesLoaded;

std::vector<char> resolveFromArchives(const std::string inputPathName) {
  const std::filesystem::path fullpath(inputPathName);
  const std::string value = fullpath.parent_path().string();
  const bsa::tes4::directory::key dictionaryKey(value);
  const std::string file = fullpath.filename().string();
  const bsa::tes4::file::key fileKey(file);

  for (const auto& archive : archivesLoaded) {
    const auto reference = archive[dictionaryKey][fileKey];
    if (!reference) continue;
    std::vector<char> byteHolder;
    if (reference->compressed()) {
      byteHolder.resize(reference->decompressed_size());
      std::span<std::byte> byteInput((std::byte*)byteHolder.data(),
                                     byteHolder.size());
      reference->decompress_into(bsa::tes4::version::sse, byteInput);
    } else {
      byteHolder =
          std::vector<char>((char*)reference->data(),
                            (char*)reference->data() + reference->size());
    }
    return byteHolder;
  }
  return {};
}

Error NifModule::init() {
  vertexFile = util::wholeFile("assets/testNif.vert");
  fragmentsFile = util::wholeFile("assets/testNif.frag");
  const auto ggm = getGameGraphicsModule();
  tge::graphics::NodeInfo nodeInfo;
  nodeInfo.transforms.scale *= this->translationFactor;
  basicNifNode = ggm->addNode(&nodeInfo, 1)[0];

  ggm->addAssetResolver(
      [sizeOfAssetsDir = this->assetDirectory.size()](const std::string& name) {
        return resolveFromArchives(name.substr(sizeOfAssetsDir));
      });

  const auto api = ggm->getAPILayer();
  SamplerInfo samplerInfo{FilterSetting::LINEAR, FilterSetting::LINEAR,
                          AddressMode::REPEAT, AddressMode::REPEAT,
                          ggm->features.anisotropicfiltering};
  samplerID = api->pushSampler(samplerInfo);
  finishedLoading = true;
  archivesLoaded.resize(archiveNames.size());
  size_t next = 0;
  for (const auto& name : archiveNames) {
    auto& archive = archivesLoaded[next++];
    try {
      archive.read(this->assetDirectory + name);
    } catch (...) {
      PLOG_ERROR << "Archive loading failed for " << name << "!";
      return Error::NOT_INITIALIZED;
    }
    if (archive.empty()) {
      PLOG_WARNING << "Archive empty after load " << name << "!";
      return Error::NOT_INITIALIZED;
    }
  }
  return Error::NONE;
}

void NifModule::remove(const size_t size, const TNodeHolder* ids) {
  std::vector<TRenderHolder> values;
  values.reserve(2 * size);
  for (size_t i = 0; i < size; i++) {
    const TNodeHolder currentID = ids[i];
    const auto iterator = nodeIdToRender.find(currentID);
    if (iterator == std::end(nodeIdToRender)) {
      PLOG_DEBUG << "Could not find " << currentID.internalHandle
                 << " in translation table!";
      return;
    }
    values.push_back(iterator->second.first);
    values.push_back(iterator->second.second);
  }
  const auto api = getAPILayer();
  api->removeRender(values.size(), values.data());
}

struct UpdateInfo {
  std::vector<std::string>& cacheString;
  std::vector<BufferInfo>& dataInfo;
};

struct SupportStruct {
  nifly::NiShape* shape;
  void* shader;
  size_t begin;
};

struct FinishInfo {
  tge::shader::ShaderAPI* api;
  std::string assetDirectory;
  tge::graphics::GameGraphicsModule* ggm;
  TNodeHolder basicNode;
};

inline void createFromTextures(TBindingHolder holder, TSamplerHolder samplerID,
                               TTextureHolder albedoID, TTextureHolder normalID,
                               std::vector<BindingInfo>& bindingInfos) {
  BindingInfo binding;
  binding.bindingSet = holder;
  binding.type = BindingType::Texture;
  binding.data.texture.sampler = samplerID;
  binding.data.texture.texture = albedoID;
  binding.binding = 1;
  bindingInfos.push_back(binding);
  binding.data.texture.texture = normalID;
  binding.binding = 4;
  bindingInfos.push_back(binding);
  binding.data.texture.texture = TTextureHolder();
  binding.type = BindingType::Sampler;
  binding.binding = 0;
  bindingInfos.push_back(binding);
}

struct RenderInfoHolder {
  std::vector<RenderInfo> infoOpaque;
  std::vector<RenderInfo> infoNoneOpaque;
  std::vector<SupportStruct> supportOpaque;
  std::vector<SupportStruct> supportNoneOpaque;
  std::vector<size_t> supportSizeOpaque;
  std::vector<size_t> supportSizeNoneOpaque;

  std::vector<BindingInfo> bindingInfos;
  std::vector<tge::graphics::NodeInfo> nodeInfos;
  size_t shapeCount = 0;

  inline void reserve(size_t size) {
    supportOpaque.reserve(size);
    supportNoneOpaque.reserve(size);
    infoOpaque.reserve(size);
    infoNoneOpaque.reserve(size);
    supportSizeOpaque.reserve(size);
    supportSizeNoneOpaque.reserve(size);
  }

  inline void pushBack(nifly::NiShape* shape, void* shader, size_t begin,
                       const RenderInfo& renderInfo) {
    if (shape->HasAlphaProperty()) {
      supportNoneOpaque.emplace_back(shape, shader, begin);
      supportSizeNoneOpaque.push_back(begin);
      infoNoneOpaque.push_back(renderInfo);
    } else {
      supportOpaque.emplace_back(shape, shader, begin);
      supportSizeOpaque.push_back(begin);
      infoOpaque.push_back(renderInfo);
    }
    shapeCount++;
  }

  inline void finish(const tge::graphics::NodeTransform& transform,
                     nifly::NifFile& file, TSamplerHolder samplerID,
                     FinishInfo& finishInfo) {
    bindingInfos.reserve(shapeCount * 3);
    nodeInfos.resize(shapeCount + 1);
    nodeInfos[0].transforms = transform;
    nodeInfos[0].parentHolder = finishInfo.basicNode;
    auto nodeIterator = nodeInfos.begin() + 1;
    for (auto& [infoIn, support] :
         {std::make_pair(std::span(infoOpaque), std::span(supportOpaque)),
          std::make_pair(std::span(infoNoneOpaque),
                         std::span(supportNoneOpaque))}) {
      for (size_t i = 0; i < infoIn.size(); i++) {
        const auto shape = support[i].shape;
        auto& info = infoIn[i];
        info.bindingID = finishInfo.api->createBindings(support[i].shader)[0];
        const auto translate = shape->transform.translation;
        const auto scale = shape->transform.scale;
        const auto rotate = shape->transform.rotation;
        auto& nodeInfo = *nodeIterator++;

        nodeInfo.parent = 0;
        nodeInfo.bindingID = info.bindingID;
        nodeInfo.transforms.translation =
            glm::vec3(translate.x, translate.y, translate.z);
        nodeInfo.transforms.scale = glm::vec3(scale);
        glm::vec3 rotationVector;
        rotate.ToEulerAngles(rotationVector.x, rotationVector.y,
                             rotationVector.z);
        nodeInfo.transforms.rotation = glm::quat(rotationVector);

        auto shaderData = file.GetShader(shape);
        if (shaderData) {
          const auto indexTexData = shaderData->TextureSetRef();
          const auto ref = file.GetHeader().GetBlock(indexTexData);
          if (ref != nullptr && ref->textures.size() > 1) {
            const auto base =
                finishInfo.assetDirectory + ref->textures[0].get();
            const auto normal =
                finishInfo.assetDirectory + ref->textures[1].get();
            std::lock_guard guard(finishInfo.ggm->protectTexture);
            const auto albedoID = finishInfo.ggm->textureMap[base];
            const auto normalID = finishInfo.ggm->textureMap[normal];
            createFromTextures(info.bindingID, samplerID, albedoID, normalID,
                               bindingInfos);
            continue;
          }
        }
      }
    }
    return;
  }
};

template <typename Type>
inline void updateOn(const UpdateInfo& info, const std::string& name,
                     const std::vector<Type>* uvData) {
  if (uvData == nullptr || uvData->empty()) return;
  info.cacheString.push_back(name);
  info.dataInfo.emplace_back(uvData->data(), uvData->size() * sizeof(Type),
                             DataType::VertexData);
}

static std::unordered_map<std::string, nifly::NifFile> filesByName;

struct HelperNoneOpaque {
  std::vector<RenderInfo> opaque;
  std::vector<RenderInfo> noneOpaque;
  std::vector<size_t> beginOpaque;
  std::vector<size_t> beginNoneOpaque;
};

std::vector<std::vector<TNodeHolder>> NifModule::load(const size_t count,
                                                      const LoadNif* loads,
                                                      void* shaderPipe) {
  if (!finishedLoading) {
    std::cerr << "Call nif before loaded!";
    return {};
  }
  const auto api = getAPILayer();
  const auto ggm = getGameGraphicsModule();
  const auto sha = api->getShaderAPI();
  std::vector<std::vector<TNodeHolder>> nodeCache;
  nodeCache.reserve(count);

  std::unordered_set<std::string> textureNames;
  textureNames.reserve(32000);

  for (size_t i = 0; i < count; i++) {
    if (filesByName.contains(loads[i].file)) continue;
    const std::filesystem::path fullpath(loads[i].file);
    const std::filesystem::path assetPath(assetDirectory + loads[i].file);
    nifly::NifFile file;
    if (std::filesystem::exists(assetPath)) {
      if (file.Load(assetPath) != 0) {
        PLOG_WARNING << "Found nif but failed to open " << loads[i].file << "!";
        return {};
      }
    } else {
      const auto& data = resolveFromArchives(loads[i].file);
      if (!data.empty()) {
        const std::string stringInput(data.data(), data.data() + data.size());
        std::istringstream stream(stringInput, std::ios_base::binary);
        if (file.Load(stream) != 0) {
          PLOG_WARNING << "Found nif " << loads[i].file
                       << " in archive but could not open it!";
          return {};
        }
      }

      if (!file.IsValid()) {
        PLOG_WARNING << "Could not find nif file " << loads[i].file;
        return {};
      }
    }
    for (const auto& shape : file.GetShapes()) {
      const auto textures = file.GetTexturePathRefs(shape);
      for (const auto texture : textures) {
        const auto& tex = texture.get();
        if (!tex.empty() && !textureNames.contains(tex)) {
          textureNames.insert(tex);
        }
      }
    }
    filesByName[loads[i].file] = std::move(file);
  }

  std::vector<std::string> texturePaths;
  texturePaths.resize(textureNames.size());
  std::transform(begin(textureNames), end(textureNames), begin(texturePaths),
                 [&](auto& str) { return this->assetDirectory + str; });
  const auto& values =
      ggm->loadTextures(texturePaths, tge::graphics::LoadType::DDSPP);

  std::vector<BufferInfo> dataInfos;
  dataInfos.reserve(count * count);

  std::vector<HelperNoneOpaque> allRenderInfos;
  allRenderInfos.resize(count);

  std::vector<TNodeHolder> allNodes;
  allNodes.resize(count);

  std::vector<std::vector<nifly::Triangle>> allTriangleLists;
  allTriangleLists.reserve(count * count);

  std::vector<std::vector<nifly::Vector3>> vertexHolder;
  vertexHolder.reserve(count * count);

  FinishInfo finInfo = {sha, assetDirectory, ggm, basicNifNode};

  for (size_t i = 0; i < count; i++) {
    auto& file = filesByName[loads[i].file];
    const auto& shapes = file.GetShapes();
    size_t current = 0;

    RenderInfoHolder holder;
    holder.reserve(shapes.size());

    for (auto shape : shapes) {
      RenderInfo info;
      size_t vertexSize = 0;
      const auto vertices = file.GetVertsForShape(shape);
      if (vertices == nullptr) {
        PLOG_WARNING << "Vertex Data is nullptr please investigate!";
        continue;
      }
      const auto oldSize = dataInfos.size();
      dataInfos.emplace_back(vertices->data(),
                             vertices->size() * sizeof(nifly::Vector3),
                             DataType::VertexData);

      std::vector<std::string> cacheString;
      const auto shader = file.GetShader(shape);
      auto shaderData = file.GetShader(shape);
      if (shaderData && shader->HasTextureSet()) {
        const auto indexTexData = shaderData->TextureSetRef();
        const auto ref = file.GetHeader().GetBlock(indexTexData);
        if (ref != nullptr && ref->textures.size() > 1) {
          cacheString.push_back("TEXTURES");
        }
      }
      cacheString.reserve(10);
      UpdateInfo updateInfo = {cacheString, dataInfos};

      auto uv = file.GetUvsForShape(shape);
      updateOn(updateInfo, "UV", uv);
      auto normal = file.GetNormalsForShape(shape);
      updateOn(updateInfo, "NORMAL", normal);
      auto color = file.GetColorsForShape(shape);
      updateOn(updateInfo, "COLOR", color);

      info.vertexBuffer.resize(dataInfos.size() - oldSize);

      std::lock_guard guard(shaderCacheMutex);
      auto foundItr = shaderCache.find(cacheString);
      if (foundItr == end(shaderCache)) {
        ShaderCreateInfo createInfo = {[](size_t input) { return input; }};
        const auto pipe =
            sha->compile({{ShaderType::VERTEX, vertexFile, cacheString},
                          {ShaderType::FRAGMENT, fragmentsFile, cacheString}},
                         createInfo);
        const Material material(pipe);
        const auto materialId = api->pushMaterials(1, &material);
        foundItr =
            shaderCache.emplace(cacheString, std::pair(materialId[0], pipe))
                .first;
      }

      auto& triangles = allTriangleLists.emplace_back();
      shape->GetTriangles(triangles);
      if (!triangles.empty()) {
        dataInfos.push_back({triangles.data(),
                             triangles.size() * sizeof(nifly::Triangle),
                             DataType::IndexData});
        info.indexCount = triangles.size() * 3;
        info.indexSize = IndexSize::UINT16;
      } else {
        info.indexCount = vertexSize;
        info.indexSize = IndexSize::NONE;
      }

      info.materialId = foundItr->second.first;
      holder.pushBack(shape, foundItr->second.second, oldSize, info);
      current++;
    }

    holder.finish(loads[i].transform, file, samplerID, finInfo);

    const auto nodes = ggm->addNode(holder.nodeInfos);
    allNodes[i] = nodes[0];
    sha->bindData(holder.bindingInfos);
    allRenderInfos[i] = {std::move(holder.infoOpaque),
                         std::move(holder.infoNoneOpaque),
                         std::move(holder.supportSizeOpaque),
                         std::move(holder.supportSizeNoneOpaque)};
    nodeCache.push_back(nodes);
  }

  const auto indexBufferID = api->pushData(dataInfos.size(), dataInfos.data());

  auto startPointer = indexBufferID.data();
  for (size_t i = 0; i < count; i++) {
    auto& renderInfoTuple = allRenderInfos[i];

    const auto process = [&](auto& renderInfos, auto& begins) {
      auto beginIterator = begins.begin();
      auto internalStart = startPointer;
      for (auto& info : renderInfos) {
        internalStart = startPointer + *beginIterator;
        for (auto& index : info.vertexBuffer) {
          index = *(internalStart++);
        }
        std::vector<char> pushData;
        pushData.resize(sizeof(uint32_t));
        memcpy(pushData.data(), &internalStart->internalHandle,
               pushData.size());
        info.constRanges.push_back({pushData, shader::ShaderType::FRAGMENT});
        info.indexBuffer = *(internalStart++);
        beginIterator++;
      }
    };

    process(renderInfoTuple.opaque, renderInfoTuple.beginOpaque);
    process(renderInfoTuple.noneOpaque, renderInfoTuple.beginNoneOpaque);

    const auto pushRenderOpaque = renderInfoTuple.opaque.empty()
                                      ? TRenderHolder{}
                                      : api->pushRender(renderInfoTuple.opaque);
    const auto pushRenderNoneOpaque =
        renderInfoTuple.noneOpaque.empty()
            ? TRenderHolder{}
            : api->pushRender(renderInfoTuple.noneOpaque, {},
                              RenderTarget::TRANSLUCENT_TARGET);

    this->nodeIdToRender[allNodes[i]] =
        std::make_pair(pushRenderOpaque, pushRenderNoneOpaque);
  }

  return nodeCache;
}
}  // namespace tge::nif