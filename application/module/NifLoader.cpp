#include "NifLoader.hpp"

#include <algorithm>
#include <filesystem>
#include <sstream>
//
#include <NifFile.hpp>
#include <TGEngine.hpp>
#include <Util.hpp>
#include <bsa/tes4.hpp>
#include <graphics/vulkan/VulkanShaderPipe.hpp>

namespace tge::nif {

using namespace tge::graphics;
using namespace tge::main;
using namespace tge::shader;

NifModule* nifModule = new nif::NifModule();

static std::vector<bsa::tes4::archive> archivesLoaded;

std::vector<char> resolveFromArchives(const std::string inputPathName) {
  const std::filesystem::path fullpath(inputPathName);
  const std::string value = fullpath.parent_path().string();
  const bsa::tes4::directory::key dictionaryKey(std::move(value));
  const std::string file = fullpath.filename().string();
  const bsa::tes4::file::key fileKey(std::move(file));

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
  basicNifNode = ggm->addNode(&nodeInfo, 1);

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
    archive.read(this->assetDirectory + name);
    if (archive.empty()) {
      PLOG(plog::warning) << "Archive empty after load " << name << "!"
                          << std::endl;
      return Error::NOT_INITIALIZED;
    }
  }
  return Error::NONE;
}

void NifModule::remove(const size_t size, const size_t* ids) {
  std::vector<TRenderHolder> values;
  values.resize(size);
  for (size_t i = 0; i < size; i++) {
    const size_t currentID = ids[i];
    const auto iterator = nodeIdToRender.find(currentID);
    if (iterator == std::end(nodeIdToRender)) {
      PLOG(plog::debug) << "Current ID: " << currentID;
    }
    values[i] = iterator->second;
  }
  const auto api = getAPILayer();
  api->removeRender(values.size(), values.data());
}

struct UpdateInfo {
  std::vector<std::string>& cacheString;
  std::vector<BufferInfo>& dataInfo;
};

template <typename Type>
inline void updateOn(const UpdateInfo& info, const std::string& name,
                     std::vector<Type>& uvData) {
  info.cacheString.push_back(name);
  info.dataInfo.push_back(
      {uvData.data(), uvData.size() * sizeof(Type), DataType::VertexData});
}

static std::unordered_map<std::string, nifly::NifFile> filesByName;

std::vector<size_t> NifModule::load(const size_t count, const LoadNif* loads,
                                    void* shaderPipe) {
  if (!finishedLoading) {
    PLOG(plog::warning) << "Call nif before loaded!" << std::endl;
    return {};
  }
  const auto api = getAPILayer();
  const auto ggm = getGameGraphicsModule();
  const auto sha = api->getShaderAPI();
  std::vector<size_t> nodeCache;
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
        PLOG(plog::warning)
            << "Found nif but failed to open " << loads[i].file << "!" << std::endl;
        return {};
      }
    } else {
      const auto& data = resolveFromArchives(loads[i].file);
      if (!data.empty()) {
        const std::string stringInput(data.data(), data.data() + data.size());
        std::istringstream stream(stringInput, std::ios_base::binary);
        if (file.Load(stream) != 0) {
          PLOG(plog::warning)
              << "Found nif " << loads[i].file
              << " in archive but could not open it!" << std::endl;
          return {};
        }
      }

      if (!file.IsValid()) {
        PLOG(plog::warning)
            << "Could not find nif file " << loads[i].file << std::endl;
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

  std::vector<std::vector<RenderInfo>> allRenderInfos;
  allRenderInfos.resize(count);

  std::vector<size_t> allNodes;
  allNodes.resize(count);

  std::vector<std::vector<std::Triangle>> allTriangleLists;
  allTriangleLists.reserve(count * count);

  for (size_t i = 0; i < count; i++) {
    const auto& file = filesByName[loads[i].file];
    const auto& shapes = file.GetShapes();
    size_t current = 0;

    std::vector<RenderInfo> renderInfos;
    renderInfos.reserve(shapes.size());
    std::vector<size_t> shapeIndex;
    shapeIndex.reserve(shapes.size());

    std::vector<Material> materials;
    materials.reserve(shapes.size());

    const auto nextNodeID = ggm->nextNodeID();

    for (auto shape : shapes) {
      nifly::BSTriShape* bishape = dynamic_cast<nifly::BSTriShape*>(shape);
      if (!bishape) {
        PLOG(plog::warning) << "No BSTriShape!";
        continue;
      }
      RenderInfo info;

      const auto oldSize = dataInfos.size();
      auto& verticies = bishape->UpdateRawVertices();
      dataInfos.push_back({verticies.data(),
                           verticies.size() * sizeof(nifly::Vector3),
                           DataType::VertexData});

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

      size_t bindingCount = 0;
      if (bishape->HasUVs()) {
        updateOn(updateInfo, "UV", bishape->UpdateRawUvs());
        bindingCount++;
      }
      if (bishape->HasNormals()) {
        updateOn(updateInfo, "NORMAL", bishape->UpdateRawNormals());
        bindingCount++;
      }
      if (bishape->HasVertexColors()) {
        updateOn(updateInfo, "COLOR", bishape->UpdateRawColors());
        bindingCount++;
      }
      info.vertexBuffer.resize(dataInfos.size() - oldSize);

      auto foundItr = shaderCache.find(cacheString);
      if (foundItr == end(shaderCache)) {
        const auto pipe =
            sha->compile({{ShaderType::VERTEX, vertexFile, cacheString},
                          {ShaderType::FRAGMENT, fragmentsFile, cacheString}});
        shaderCache[cacheString] = pipe;
        foundItr = shaderCache.find(cacheString);
        tge::shader::VulkanShaderPipe* ptr =
            (tge::shader::VulkanShaderPipe*)foundItr->second;
        ptr->vertexInputBindings.clear();
        ptr->vertexInputBindings.resize(bindingCount + 1);
        ptr->vertexInputBindings[0] = vk::VertexInputBindingDescription(0, 12);
        for (auto& attribute : ptr->vertexInputAttributes) {
          attribute.binding = attribute.location;
          attribute.offset = 0;
          ptr->vertexInputBindings[attribute.location] =
              vk::VertexInputBindingDescription(
                  attribute.binding,
                  tge::shader::getSizeFromFormat(attribute.format));
        }
        ptr->inputStateCreateInfo.pVertexBindingDescriptions =
            ptr->vertexInputBindings.data();
        ptr->inputStateCreateInfo.vertexBindingDescriptionCount =
            ptr->vertexInputBindings.size();
      }
      void* ptr = foundItr->second;
      materials.push_back(Material(ptr));

      allTriangleLists.push_back({});
      auto& triangles = allTriangleLists.back();
      shape->GetTriangles(triangles);
      if (!triangles.empty()) {
        dataInfos.push_back({triangles.data(),
                             triangles.size() * sizeof(nifly::Triangle),
                             DataType::IndexData});
        info.indexCount = triangles.size() * 3;
        info.indexSize = IndexSize::UINT16;
      } else {
        info.indexCount = verticies.size();
        info.indexSize = IndexSize::NONE;
      }
      renderInfos.push_back(info);
      shapeIndex.push_back(current);
      current++;
    }
    const auto materialId =
        api->pushMaterials(materials.size(), materials.data());

    std::vector<BindingInfo> bindingInfos;
    bindingInfos.reserve(shapes.size() * 3);
    std::vector<tge::graphics::NodeInfo> nodeInfos;
    nodeInfos.resize(shapeIndex.size() + 1);
    nodeInfos[0].transforms = loads[i].transform;
    nodeInfos[0].parent = basicNifNode;
    for (size_t i = 0; i < shapeIndex.size(); i++) {
      const auto shape = shapes[shapeIndex[i]];
      auto& info = renderInfos[i];
      info.materialId = materialId[i];
      info.bindingID = sha->createBindings(materials[i].costumShaderData, 1);
      const auto translate = shape->transform.translation;
      auto& nodeInfo = nodeInfos[i + 1];

      nodeInfo.parent = nextNodeID;
      std::vector<char> pushData;
      pushData.resize(sizeof(uint32_t));
      memcpy(pushData.data(), &nextNodeID, pushData.size());
      info.constRanges.push_back({pushData, shader::ShaderType::FRAGMENT});
      nodeInfo.bindingID = info.bindingID;
      nodeInfo.transforms.translation =
          glm::vec3(translate.x, translate.y, translate.z);

      auto shaderData = file.GetShader(shape);
      if (shaderData) {
        const auto indexTexData = shaderData->TextureSetRef();
        const auto ref = file.GetHeader().GetBlock(indexTexData);
        if (ref != nullptr && ref->textures.size() > 1) {
          const auto base = assetDirectory + ref->textures[0].get();
          const auto normal = assetDirectory + ref->textures[1].get();
          std::lock_guard guard(ggm->protectTexture);
          const auto albedoID = ggm->textureMap[base];
          const auto normalID = ggm->textureMap[normal];
          BindingInfo binding;
          binding.bindingSet = nodeInfo.bindingID;
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
      }
    }

    const auto nodes = ggm->addNode(nodeInfos.data(), nodeInfos.size());
    allNodes[i] = nodes;
    sha->bindData(bindingInfos.data(), bindingInfos.size());
    allRenderInfos[i] = renderInfos;
    nodeCache.push_back(nodes);
  }

  const auto indexBufferID = api->pushData(dataInfos.size(), dataInfos.data());

  auto startPointer = indexBufferID.data();
  for (size_t i = 0; i < count; i++) {
    auto& renderInfos = allRenderInfos[i];
    if (renderInfos.empty()) continue;
    for (auto& info : renderInfos) {
      for (auto& index : info.vertexBuffer) {
        index = *(startPointer++);
      }
      info.indexBuffer = *(startPointer++);
    }

    const auto pushRender =
        api->pushRender(renderInfos.size(), renderInfos.data());

    this->nodeIdToRender[allNodes[i]] = pushRender;
  }

  return nodeCache;
}
}  // namespace tge::nif