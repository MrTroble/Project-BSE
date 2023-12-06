#pragma once

#include <stddef.h>

#include "Interop.hpp"

namespace tge::interop {

bool load(const uint count, const ReferenceLoad* load);

bool update(const uint count, const ReferenceUpdate* keys);

bool hide(const uint count, const FormKey* keys, bool hide);

bool remove(const uint count, const FormKey* keys);

bool select(const uint count, const FormKey* keys);

bool terrain(const uint count, const TerrainInfo* info, float* buffer);

bool internalSelect(const size_t count, const size_t* ids);

void* getMainWindowHandle();

}  // namespace tge::interop