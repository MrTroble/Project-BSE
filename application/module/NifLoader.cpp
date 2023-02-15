#include "NifLoader.hpp"

#include <NifFile.hpp>
#include <TGEngine.hpp>
#include <Util.hpp>
#include <algorithm>
#include <graphics/vulkan/VulkanShaderPipe.hpp>
#include <string>

namespace tge::nif {

using namespace tge::graphics;
using namespace tge::main;
using namespace tge::shader;

NifModule* nifModule = new nif::NifModule();

Error NifModule::init() {
  vertexFile = util::wholeFile("assets/testNif.vert");
  fragmentsFile = util::wholeFile("assets/testNif.frag");
  const auto ggm = getGameGraphicsModule();
  tge::graphics::NodeInfo nodeInfo;
  nodeInfo.transforms.scale *= this->translationFactor;
  basicNifNode = ggm->addNode(&nodeInfo, 1);

  const auto api = ggm->getAPILayer();
  SamplerInfo samplerInfo{FilterSetting::LINEAR, FilterSetting::LINEAR,
                          AddressMode::REPEAT, AddressMode::REPEAT,
                          ggm->features.anisotropicfiltering};
  samplerID = api->pushSampler(samplerInfo);
  finishedLoading = true;
  return Error::NONE;
}

void NifModule::remove(const size_t size, const size_t* ids) {
  std::vector<size_t> values;
  values.resize(size);
  for (size_t i = 0; i < size; i++) {
    values[i] = nodeIdToRender[ids[i]];
  }
  const auto api = getAPILayer();
  api->removeRender(values.size(), values.data());
}

struct UpdateInfo {
  std::vector<std::string>& cacheString;
  std::vector<const void*>& dataPointer;
  std::vector<size_t>& sizes;
  std::vector<size_t>& vertexBuffer;
};

template <typename Type>
inline void updateOn(const UpdateInfo& info, const std::string& name,
                     const std::vector<Type>& uvData) {
  info.cacheString.push_back(name);
  info.vertexBuffer.push_back(info.dataPointer.size());
  info.dataPointer.push_back(uvData.data());
  info.sizes.push_back(uvData.size() * sizeof(Type));
}

std::vector<size_t> NifModule::load(const size_t count, const LoadNif* loads,
                       void* shaderPipe) {
  if (!finishedLoading) {
    printf("[WARN] Call nif before loaded!\n");
    return {};
  }
  const auto api = getAPILayer();
  const auto ggm = getGameGraphicsModule();
  const auto sha = api->getShaderAPI();
  std::vector<size_t> nodeCache;
  nodeCache.reserve(count);

  std::unordered_set<std::string> textureNames;
  textureNames.reserve(32000);

  std::unordered_map<std::string, nifly::NifFile> filesByName;

  for (size_t i = 0; i < count; i++) {
    nifly::NifFile file(assetDirectory + loads[i].file);
    if (!file.IsValid()) {
      printf("[WARN] Invalid nif file %s\n", loads[i].file.c_str());
      return {};
    }
    filesByName[loads[i].file] = file;
    for (const auto& shape : file.GetShapes()) {
      const auto textures = file.GetTexturePathRefs(shape);
      for (const auto texture : textures) {
        const auto& tex = texture.get();
        if (!tex.empty() || !textureNames.contains(tex)) {
          textureNames.insert(tex);
        }
      }
    }
  }

  std::vector<std::string> texturePaths;
  texturePaths.resize(textureNames.size());
  std::transform(begin(textureNames), end(textureNames), begin(texturePaths),
                 [&](auto& str) { return this->assetDirectory + str; });
  ggm->loadTextures(texturePaths, tge::graphics::LoadType::DDSPP);

  for (size_t i = 0; i < count; i++) {
    const auto& file = filesByName[loads[i].file];
    const auto& shapes = file.GetShapes();

    std::vector<RenderInfo> renderInfos;
    renderInfos.reserve(shapes.size());
    std::vector<size_t> shapeIndex;
    shapeIndex.reserve(shapes.size());

    std::vector<const void*> dataPointer;
    std::vector<size_t> sizes;
    size_t current = 0;
    dataPointer.reserve(shapes.size() * 5);
    sizes.reserve(shapes.size() * 5);
    std::vector<std::vector<std::Triangle> > triangleLists;
    triangleLists.resize(shapes.size());

    std::vector<Material> materials;
    materials.reserve(shapes.size());

    const auto nextNodeID = ggm->nextNodeID();

    for (auto shape : shapes) {
      nifly::BSTriShape* bishape = dynamic_cast<nifly::BSTriShape*>(shape);
      if (!bishape) {
        printf("[WARN]: No BSTriShape!\n");
        continue;
      }
      RenderInfo info;

      const auto& verticies = bishape->UpdateRawVertices();
      info.vertexBuffer.push_back(dataPointer.size());
      dataPointer.push_back(verticies.data());
      sizes.push_back(verticies.size() * sizeof(nifly::Vector3));

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
      UpdateInfo updateInfo = {cacheString, dataPointer, sizes,
                               info.vertexBuffer};

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
      info.materialId = materials.size();
      materials.push_back(Material(ptr));

      auto& triangles = triangleLists[current];
      shape->GetTriangles(triangles);
      if (!triangles.empty()) {
        info.indexBuffer = dataPointer.size();
        sizes.push_back(triangles.size() * sizeof(std::Triangle));
        dataPointer.push_back(triangles.data());
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

    const auto indexBufferID =
        api->pushData(dataPointer.size(), dataPointer.data(), sizes.data(),
                      DataType::VertexIndexData);

    std::vector<BindingInfo> bindingInfos;
    bindingInfos.reserve(shapes.size() * 3);
    std::vector<tge::graphics::NodeInfo> nodeInfos;
    nodeInfos.resize(shapeIndex.size() + 1);
    nodeInfos[0].transforms = loads[i].transform;
    nodeInfos[0].parent = basicNifNode;
    for (size_t i = 0; i < shapeIndex.size(); i++) {
      const auto shape = shapes[shapeIndex[i]];
      nifly::BSTriShape* bishape = dynamic_cast<nifly::BSTriShape*>(shape);
      auto& info = renderInfos[i];
      info.materialId += materialId;
      info.bindingID = sha->createBindings(materials[i].costumShaderData, 1);
      info.indexBuffer += indexBufferID;
      for (auto& index : info.vertexBuffer) {
        index += indexBufferID;
      }
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
          bindingInfos.push_back({1,
                                  nodeInfo.bindingID,
                                  BindingType::Texture,
                                  {albedoID, samplerID}});
          bindingInfos.push_back({4,
                                  nodeInfo.bindingID,
                                  BindingType::Texture,
                                  {normalID, samplerID}});
          bindingInfos.push_back({0,
                                  nodeInfo.bindingID,
                                  BindingType::Sampler,
                                  {UINT64_MAX, samplerID}});
        }
      }
    }

    const auto nodes = ggm->addNode(nodeInfos.data(), nodeInfos.size());

    sha->bindData(bindingInfos.data(), bindingInfos.size());

    const auto pushRender =
        api->pushRender(renderInfos.size(), renderInfos.data());

    this->nodeIdToRender[nodes] = pushRender;
    nodeCache.push_back(nodes);
  }

  return nodeCache;
}
}  // namespace tge::nif