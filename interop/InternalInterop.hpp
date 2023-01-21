#pragma once

#include "Interop.hpp"

namespace tge::interop {

bool load(const uint count, const ReferenceLoad* load);

bool update(const uint count, const ReferenceUpdate* keys);

bool hide(const uint count, const FormKey* keys, bool hide);

bool remove(const uint count, const FormKey* keys);
}  // namespace tge::interop