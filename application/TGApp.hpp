#pragma once

#include "../interop/Interop.hpp"
#include "module/TGAppGUI.hpp"
#include "module/TGAppIO.hpp"

extern TGAppGUI* guiModul;
extern TGAppIO* ioModul;

constexpr uint32_t CURRENT_INIT_VERSION = 2;

struct InitConfig {
  uint32_t version = CURRENT_INIT_VERSION;
  char* assetDirectory = nullptr;
  size_t sizeOfWindowHandles = 0;
  void** windowHandles = nullptr;
};

TGE_DLLEXPORT int initTGEditor(const InitConfig* config, const char** bsaFiles,
                               const size_t size);

TGE_DLLEXPORT bool isFinished();

TGE_DLLEXPORT void waitFinishedInit();

struct SizeInformation {
  size_t sizeInformationStruct = sizeof(SizeInformation);
  size_t initConfigStruct = sizeof(InitConfig);
  size_t referenceTransformStruct = sizeof(ReferenceTransform);
  size_t referenceLoadStruct = sizeof(ReferenceLoad);
  size_t referenceUpdateStruct = sizeof(ReferenceUpdate);
  size_t textureSetStruct = sizeof(TextureSetInternal<const char*>);
  size_t alphaDataStruct = sizeof(AlphaData);
  size_t alphaLayerStruct = sizeof(AlphaLayer<TextureSetInternal<const char*>>);
  size_t quadrantStruct = sizeof(Quadrant);
  size_t cornerSetsStruct = sizeof(CornerSetsDefault);
  size_t terrainInfoStruct = sizeof(TerrainInfo);
};

TGE_DLLEXPORT SizeInformation getSizeInfo();