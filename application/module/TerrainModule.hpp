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
  SECornerSetsInternal cornerSet;
  size_t pointSize;
};

class TerrainModule : public tge::main::Module {
 public:
  tge::graphics::APILayer* api;
  tge::graphics::GameGraphicsModule* ggm;
  size_t binding;
  tge::graphics::TPipelineHolder materialHolder;
  size_t basicNode;

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
    basicNode = ggm->addNode(nodeInfos);
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
      renderInfo.indexBuffer = iterator[3];
      renderInfo.indexSize = IndexSize::UINT16;
      renderInfo.indexCount = (terrain.pointSize - 1) * (terrain.pointSize - 1) * 6;
      renderInfo.materialId = materialHolder;
      iterator += 4;
    }

    api->pushRender(info.size(), info.data());
    return {};
  }
};

extern TerrainModule* terrainModule;
