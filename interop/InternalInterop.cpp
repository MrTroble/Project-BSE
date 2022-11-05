#include "InternalInterop.hpp"
#include "../application/module/NifLoader.hpp"

namespace tge::interop {

	bool load(uint count, ReferenceLoad* load)
	{
		return false;
	}

	bool update(uint count, ReferenceUpdate* keys)
	{
		return false;
	}
	
	bool hide(uint count, FormKey* keys, bool hide)
	{
		return false;
	}
	
	bool remove(uint count, FormKey* keys)
	{
		return false;
	}
}