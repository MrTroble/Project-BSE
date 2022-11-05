#pragma once

#include "Interop.hpp"

namespace tge::interop {

	bool load(uint count, ReferenceLoad* load);

	bool update(uint count, ReferenceUpdate* keys);

	bool hide(uint count, FormKey* keys, bool hide);
	
	bool remove(uint count, FormKey* keys);

}