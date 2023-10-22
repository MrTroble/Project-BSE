#pragma once

#include <Module.hpp>
#include <TGEngine.hpp>
#include <array>
#include <concepts>
#include <graphics/APILayer.hpp>
#include <graphics/ElementHolder.hpp>
#include <graphics/Material.hpp>
#include <span>
#include <tuple>
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

constexpr size_t MAX_IMAGES = 192;

namespace std {
template <class Left, class Right>
struct hash<std::pair<Left, Right>> {
  [[nodiscard]] inline std::size_t operator()(
      const std::pair<Left, Right>& s) const noexcept {
    const std::hash<Left> leftHash;
    const std::hash<Right> rightHash;
    return leftHash(s.first) | (rightHash(s.second) << 1);
  }
};
}  // namespace std

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
    std::vector<uint32_t> quadrants(terrain.pointSize * terrain.pointSize);
    const float uvRatio = terrain.maxUV / (float)terrain.pointSize;
    for (size_t y = 0; y < terrain.pointSize; y++) {
      const auto yConst = y * terrain.pointSize;
      for (size_t x = 0; x < terrain.pointSize; x++) {
        const auto index = x + yConst;
        uvs[index] = {x * uvRatio, y * uvRatio};
        quadrants[index] = (uint32_t)(x * 2 / terrain.pointSize) +
                           (uint32_t)(y * 2 / terrain.pointSize) * 2;
      }
    }
    const std::array bufferInfo{
        BufferInfo{uvs.data(), uvs.size() * sizeof(glm::vec2),
                   DataType::VertexData},
        BufferInfo{quadrants.data(), quadrants.size() * sizeof(uint32_t),
                   DataType::VertexData}};

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

      renderInfo.vertexBuffer.insert(std::end(renderInfo.vertexBuffer),
                                     std::begin(terrainCache.data),
                                     std::end(terrainCache.data));
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
      textureData.maxUV = terrain.maxUV;

      auto arrayID = 0;
      BindingInfo info;
      info.data.texture.sampler = sampler;
      info.binding = 1;
      info.bindingSet = renderInfo.bindingID;
      info.type = BindingType::Texture;
      size_t cornerID = 0;
      for (const auto& corner : forEachCorner(terrain.cornerSet)) {
        const auto diffuse = corner.BaseLayer.Diffuse;
        auto loadedTextures = ggm->loadTextures(
            {diffuse, corner.BaseLayer.Normal},
            diffuse.ends_with(".png") ? LoadType::STBI : LoadType::DDSPP);
        size_t index = 0;
        for (const auto texture : loadedTextures) {
          info.data.texture.texture = texture;
          info.arrayID = arrayID + index++;
          bindingInfos.push_back(info);
        }
        textureData.quadrants[cornerID].Diffuse = arrayID++;
        textureData.quadrants[cornerID].Normal = arrayID++;
        cornerID++;
      }

      info.data.texture.texture = ggm->defaultTextureID;
      for (size_t i = arrayID; i < MAX_IMAGES; i++) {
        info.arrayID = i;
        bindingInfos.push_back(info);
      }

      BindingInfo samplerInfo;
      samplerInfo.binding = 0;
      samplerInfo.bindingSet = renderInfo.bindingID;
      samplerInfo.data.texture.sampler = sampler;
      samplerInfo.data.texture.texture = TTextureHolder();
      samplerInfo.type = BindingType::Sampler;
      bindingInfos.push_back(samplerInfo);
    }

    const auto dataHolderCreated = api->pushData(bufferInfos);
    for (size_t i = 0; i < dataHolderCreated.size(); i++) {
      BindingInfo info;
      info.binding = 4;
      info.bindingSet = bindings + i;
      info.type = BindingType::UniformBuffer;
      info.data.buffer.dataID = dataHolderCreated[i];
      info.data.buffer.size = sizeof(TerrainTextureInfo);
      info.data.buffer.offset = 0;
      bindingInfos.push_back(info);
    }

    ggm->addNode(nodeInfos);
    shader->bindData(bindingInfos);

    api->pushRender(info.size(), info.data());
    return {};
  }
};

extern TerrainModule* terrainModule;
