#pragma once

#include <Module.hpp>
#include <TGEngine.hpp>
#include <graphics/APILayer.hpp>
#include <graphics/ElementHolder.hpp>
#include <graphics/Material.hpp>
#include <span>

#include "../../interop/SETextureset.hpp"

DEFINE_HOLDER(Terrain);

struct TerrainInfoInternal {
  CornerSetsInternal cornerSet;
  size_t pointSize;
};

class TerrainModule : public tge::main::Module {
 public:
  tge::graphics::APILayer* api;
  tge::graphics::GameGraphicsModule* ggm;
  size_t binding;
  tge::graphics::TPipelineHolder materialHolder;
  tge::graphics::TNodeHolder basicNode;
  std::unordered_map<size_t, tge::graphics::TDataHolder> uvCache;

  tge::main::Error init() override {
    using namespace tge::graphics;
    ggm = tge::main::getGameGraphicsModule();
    api = ggm->getAPILayer();
    auto shaderApi = api->getShaderAPI();
    tge::shader::ShaderCreateInfo createInfo;
    createInfo.inputLayoutTranslation = [](auto in) { return in; };
    const auto shader = shaderApi->loadShaderPipeAndCompile(
        {"assets/terrain.vert", "assets/terrain.frag"}, createInfo);
    binding = shaderApi->createBindings(shader);
    Material material(shader);
    const std::vector materials{material};
    materialHolder = api->pushMaterials(materials)[0];
    std::vector<NodeInfo> nodeInfos(1);
    nodeInfos[0].bindingID = binding;
    basicNode = ggm->addNode(nodeInfos)[0];
    return tge::main::Error::NONE;
  }

  std::vector<TTerrainHolder> loadTerrainSE(
      const std::span<const TerrainInfoInternal> cornerSets,
      const std::span<const tge::graphics::TDataHolder> dataHolder) {
    using namespace tge::graphics;
    auto iterator = dataHolder.begin();
    std::vector<RenderInfo> info;
    info.resize(cornerSets.size());
    auto render = info.begin();
    for (const auto& terrain : cornerSets) {
      auto& renderInfo = *render;
      renderInfo.bindingID = binding;
      renderInfo.vertexBuffer = std::vector(iterator, iterator + 3);
      auto uvIterator = uvCache.find(terrain.pointSize);
      if (uvIterator == std::end(uvCache)) {
        std::vector<glm::vec2> uvs(terrain.pointSize * terrain.pointSize);
        const auto maxUV = 24;
        const float uvRatio = maxUV / (float)terrain.pointSize;
        for (size_t y = 0; y < terrain.pointSize; y++) {
          const auto yConst = y * terrain.pointSize;
          for (size_t x = 0; x < terrain.pointSize; x++) {
            const auto index = x + yConst;
            uvs[index] = {x * uvRatio, y * uvRatio};
          }
        }
        const BufferInfo bufferInfo{uvs.data(), uvs.size() * sizeof(glm::vec2),
                                    DataType::VertexData};
        const auto data = api->pushData(1, &bufferInfo);
        uvIterator = uvCache.insert({terrain.pointSize, data[0]}).first;
      }
      renderInfo.vertexBuffer.push_back(uvIterator->second);
      renderInfo.indexBuffer = iterator[3];
      renderInfo.indexSize = IndexSize::UINT16;
      renderInfo.indexCount =
          (terrain.pointSize - 1) * (terrain.pointSize - 1) * 6;
      renderInfo.materialId = materialHolder;
      iterator += 4;
    }

    api->pushRender(info.size(), info.data());
    return {};
  }
};

extern TerrainModule* terrainModule;
