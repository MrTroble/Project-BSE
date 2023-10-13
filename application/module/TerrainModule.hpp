#pragma once

#include <Module.hpp>
#include <TGEngine.hpp>
#include <graphics/APILayer.hpp>
#include <graphics/ElementHolder.hpp>
#include <graphics/Material.hpp>
#include <span>
#include <array>
#include <vector>

#include "../../interop/SETextureset.hpp"

DEFINE_HOLDER(Terrain);

struct TerrainInfoInternal {
  CornerSetsInternal cornerSet;
  uint32_t pointSize;
  float maxUV = 24.0f;
};

struct TerrainCache {
  std::vector<tge::graphics::TDataHolder> data;
};

struct TerrainTextureInfo {
  TextureSetInternal<uint32_t> quadrants[4];
  float maxUV;
};

constexpr auto TF = sizeof(TerrainTextureInfo);

constexpr size_t MAX_IMAGES = 192;

class TerrainModule : public tge::main::Module {
 public:
  tge::graphics::APILayer* api;
  tge::graphics::GameGraphicsModule* ggm;
  tge::graphics::TPipelineHolder materialHolder;
  std::unordered_map<std::pair<size_t, float>, TerrainCache> uvCache;
  tge::shader::ShaderPipe shaderPipe;
  tge::graphics::TSamplerHolder sampler;

  tge::main::Error init() override {
    using namespace tge::graphics;
    ggm = tge::main::getGameGraphicsModule();
    api = ggm->getAPILayer();
    auto shaderApi = api->getShaderAPI();
    tge::shader::ShaderCreateInfo createInfo;
    createInfo.inputLayoutTranslation = [](auto in) { return in; };
    shaderPipe = shaderApi->loadShaderPipeAndCompile(
        {"assets/terrain.vert", "assets/terrain.frag"}, createInfo);
    Material material(shaderPipe);
    const std::vector materials{material};
    materialHolder = api->pushMaterials(materials)[0];
    SamplerInfo info;
    info.uMode = AddressMode::REPEAT;
    info.vMode = AddressMode::REPEAT;
    sampler = api->pushSampler(info);
    return tge::main::Error::NONE;
  }

  inline auto createCache(const TerrainInfoInternal& terrain) {
    using namespace tge::graphics;
    std::vector<glm::vec2> uvs(terrain.pointSize * terrain.pointSize);
    const float uvRatio = terrain.maxUV / (float)terrain.pointSize;
    for (size_t y = 0; y < terrain.pointSize; y++) {
      const auto yConst = y * terrain.pointSize;
      for (size_t x = 0; x < terrain.pointSize; x++) {
        const auto index = x + yConst;
        uvs[index] = {x * uvRatio, y * uvRatio};
      }
    }
    const std::array bufferInfo{BufferInfo{
        uvs.data(), uvs.size() * sizeof(glm::vec2), DataType::VertexData}};

    TerrainCache cache;
    cache.data = api->pushData(bufferInfo);
    return uvCache.insert({{terrain.pointSize, terrain.maxUV}, cache}).first;
  }

  std::vector<TTerrainHolder> loadTerrainSE(
      const std::span<const TerrainInfoInternal> cornerSets,
      const std::span<const tge::graphics::TDataHolder> dataHolder) {
    using namespace tge::graphics;
    using namespace tge::shader;
    auto iterator = dataHolder.begin();
    std::vector<RenderInfo> info;
    info.resize(cornerSets.size());
    auto render = info.begin();
    auto shader = api->getShaderAPI();
    const auto bindings = shader->createBindings(shaderPipe, cornerSets.size());
    auto bindingIterator = bindings;

    std::vector<NodeInfo> nodeInfos;
    nodeInfos.reserve(cornerSets.size());
    std::vector<TerrainTextureInfo> terrainTextureInfos;
    terrainTextureInfos.resize(cornerSets.size());
    std::vector<BufferInfo> bufferInfos;
    bufferInfos.resize(cornerSets.size());
    std::memset((char*)terrainTextureInfos.data(), 0,
                terrainTextureInfos.size() * sizeof(TerrainTextureInfo));
    std::vector<BindingInfo> bindingInfos;
    bindingInfos.reserve((MAX_IMAGES + 2) * cornerSets.size());
    auto textureInfo = terrainTextureInfos.begin();
    auto bufferInfo = bufferInfos.begin();
    for (const auto& terrain : cornerSets) {
      auto& renderInfo = *render;
      renderInfo.vertexBuffer = std::vector(iterator, iterator + 3);
      auto uvIterator = uvCache.find({terrain.pointSize, terrain.maxUV});
      if (uvIterator == std::end(uvCache)) {
        uvIterator = createCache(terrain);
      }
      const auto& terrainCache = uvIterator->second;
      renderInfo.bindingID = bindingIterator++;

      renderInfo.vertexBuffer.push_back(terrainCache.data[0]);
      renderInfo.indexBuffer = iterator[3];
      renderInfo.indexSize = IndexSize::UINT16;
      renderInfo.indexCount =
          (terrain.pointSize - 1) * (terrain.pointSize - 1) * 6;
      renderInfo.materialId = materialHolder;
      iterator += 4;

      nodeInfos.emplace_back(renderInfo.bindingID);

      BufferInfo& buffer = *bufferInfo++;
      buffer.data = &textureInfo[0];
      buffer.size = sizeof(TerrainTextureInfo);
      buffer.type = DataType::Uniform;

      TerrainTextureInfo& textureData = *textureInfo++;

      auto arrayID = 0;
      BindingInfo info;
      info.binding = 1;
      info.bindingSet = renderInfo.bindingID;
      info.type = BindingType::Texture;
      size_t cornerID = 0;
      for (const auto& corner : forEachCorner(terrain.cornerSet)) {
        auto loadedTextures = ggm->loadTextures(
            {corner.BaseLayer.Diffuse, corner.BaseLayer.Normal},
            LoadType::DDSPP);
        for (const auto texture : loadedTextures) {
          info.data.texture.texture = texture;
          info.arrayID = arrayID;
          bindingInfos.push_back(info);
        }
        textureData.quadrants[cornerID].Diffuse = arrayID++;
        textureData.quadrants[cornerID].Normal = arrayID++;
      }

      info.data.texture.texture = ggm->defaultTextureID;
      for (size_t i = arrayID; i < MAX_IMAGES; i++) {
        info.arrayID = i;
        bindingInfos.push_back(info);
      }

      BindingInfo samplerInfo;
      samplerInfo.binding = 0;
      samplerInfo.bindingSet = renderInfo.bindingID;
      samplerInfo.type = BindingType::Sampler;
      samplerInfo.data.texture.sampler = sampler;
      bindingInfos.push_back(samplerInfo);
    }

    const auto dataHolderCreated = api->pushData(bufferInfos);
    for (size_t i = 0; i < dataHolder.size(); i++) {
      BindingInfo info;
      info.binding = 4;
      info.bindingSet = bindings + i;
      info.type = BindingType::UniformBuffer;
      info.data.buffer.dataID = dataHolderCreated[i];
      info.data.buffer.size = sizeof(TerrainTextureInfo);
      bindingInfos.push_back(info);
    }

    ggm->addNode(nodeInfos);
    shader->bindData(bindingInfos);

    api->pushRender(info.size(), info.data());
    return {};
  }
};

extern TerrainModule* terrainModule;
