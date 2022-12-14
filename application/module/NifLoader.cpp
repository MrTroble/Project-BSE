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
  finishedLoading = true;
  return Error::NONE;
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

size_t NifModule::load(const std::string& name,
                       const tge::graphics::NodeTransform& baseTransform,
                       void* shaderPipe) const {
  if (!finishedLoading) {
    printf("[WARN] Call nif before loaded! %s\n", name.c_str());
    return SIZE_MAX;
  }
  const auto api = getAPILayer();
  const auto ggm = getGameGraphicsModule();
  const auto sha = api->getShaderAPI();
  nifly::NifFile file(assetDirectory + name);
  if (!file.IsValid()) {
    printf("[WARN] Invalid nif file %s\n", name.c_str());
    return SIZE_MAX;
  }
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

  std::vector<std::string> textureNames;
  textureNames.reserve(shapes.size());

  std::unordered_map<std::string, size_t> textureNamesToID;

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
    if (shader != nullptr && shader->HasTextureSet())
      cacheString.push_back("TEXTURES");
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
    const auto textures = file.GetTexturePathRefs(shape);
    for (const auto texture : textures) {
      const auto& tex = texture.get();
      if (!tex.empty()) {
        textureNames.push_back(tex);
      }
    }
    renderInfos.push_back(info);
    shapeIndex.push_back(current);
    current++;
  }
  const auto materialId =
      api->pushMaterials(materials.size(), materials.data());

  std::vector<std::string> texturePaths;
  texturePaths.resize(textureNames.size());
  std::transform(begin(textureNames), end(textureNames), begin(texturePaths),
                 [&](auto& str) { return this->assetDirectory + str; });
  const auto texturesLoaded =
      ggm->loadTextures(texturePaths, tge::graphics::LoadType::DDSPP);
  const auto indexBufferID =
      api->pushData(dataPointer.size(), dataPointer.data(), sizes.data(),
                    DataType::VertexIndexData);

  SamplerInfo samplerInfo{FilterSetting::LINEAR, FilterSetting::LINEAR,
                          AddressMode::REPEAT, AddressMode::REPEAT};
  const auto samplerID = api->pushSampler(samplerInfo);

  size_t id = texturesLoaded;
  for (const auto& name : textureNames) {
    textureNamesToID[name] = id++;
  }

  std::vector<BindingInfo> bindingInfos;
  bindingInfos.reserve(shapes.size() * 3);
  std::vector<tge::graphics::NodeInfo> nodeInfos;
  nodeInfos.resize(shapeIndex.size() + 1);
  nodeInfos[0].transforms = baseTransform;
  nodeInfos[0].parent = basicNifNode;
  for (size_t i = 0; i < shapeIndex.size(); i++) {
    const auto shape = shapes[shapeIndex[i]];
    nifly::BSTriShape* bishape = dynamic_cast<nifly::BSTriShape*>(shape);
    auto& info = renderInfos[i];
    info.materialId += materialId;
    info.bindingID =
        sha->createBindings(materials[i].costumShaderData, 1);
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
        const auto& base = ref->textures[0].get();
        const auto& normal = ref->textures[1].get();
        const auto albedoID = textureNamesToID[base];
        const auto normalID = textureNamesToID[normal];
        const BindingInfo samplerBinding{0,
                                         nodeInfo.bindingID,
                                         BindingType::Sampler,
                                         {UINT64_MAX, samplerID}};
        const BindingInfo albedoBinding{
            1, nodeInfo.bindingID, BindingType::Texture, {albedoID, samplerID}};
        const BindingInfo normalBinding{
            4, nodeInfo.bindingID, BindingType::Texture, {normalID, samplerID}};
        bindingInfos.push_back(albedoBinding);
        bindingInfos.push_back(normalBinding);
        bindingInfos.push_back(samplerBinding);
      }
    }
  }

  const auto nodes = ggm->addNode(nodeInfos.data(), nodeInfos.size());

  sha->bindData(bindingInfos.data(), bindingInfos.size());

  api->pushRender(renderInfos.size(), renderInfos.data());

  return nodes;
}
}  // namespace tge::nif