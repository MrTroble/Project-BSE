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