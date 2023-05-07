#pragma once

#include "module/TGAppGUI.hpp"
#include "module/TGAppIO.hpp"
#include "../interop/Interop.hpp"

extern TGAppGUI *guiModul;
extern TGAppIO *ioModul;

struct InitConfig {
	uint32_t version = 1;
	char* assetDirectory = nullptr;
    size_t sizeOfBSA = 0;
    char** bsaFiles = nullptr;
    size_t sizeOfWindowHandles = 0;
    void** windowHandles  = nullptr;
};

TGE_DLLEXPORT int initTGEditor(const InitConfig* config);

TGE_DLLEXPORT bool isFinished();

TGE_DLLEXPORT void waitFinishedInit();
