#pragma once

#include <Module.hpp>
#include <graphics/APILayer.hpp>
#include <graphics/ElementHolder.hpp>
#include <TGEngine.hpp>
#include <span>

#include "../../interop/SETextureset.hpp"

DEFINE_HOLDER(Terrain);

struct TerrainInfoInternal {
  SECornerSets cornerSet;
  size_t pointSize;
};

class TerrainModule : public tge::main::Module {
 public:
  tge::graphics::APILayer* api;
  tge::graphics::GameGraphicsModule* ggm;

  tge::main::Error init() override {
    ggm = tge::main::getGameGraphicsModule();
    api = ggm->getAPILayer();
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
      renderInfo.vertexBuffer = std::vector(iterator, iterator + 3);
      renderInfo.indexBuffer = iterator[3];
      renderInfo.indexCount = (terrain.pointSize - 1) * (terrain.pointSize - 1);
      renderInfo.materialId = ggm->defaultMaterial;
      iterator += 4;
    }
    api->pushRender(info.size(), info.data());
    return {};
  }
};

extern TerrainModule* terrainModule;
