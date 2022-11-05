#pragma once

#include "module/TGAppGUI.hpp"
#include "module/TGAppIO.hpp"
#include "../interop/Interop.hpp"

extern TGAppGUI *guiModul;
extern TGAppIO *ioModul;

TGE_DLLEXPORT int initTGEditor(const int count, const char** strings);
