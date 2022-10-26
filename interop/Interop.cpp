#include "Interop.hpp"
#include <iostream>

ReferenceKey loadReferences(unsigned int count, RefernceLoad* load) {
	std::cout << count << " TEST TEST";
	return 0;
}

TGE_DLLEXPORT ReferenceKey loadReferences(uint count, RefernceLoad* load)
{
	return ReferenceKey();
}

TGE_DLLEXPORT bool updateReferences(uint count, RefernceUpdate* keys)
{
	return bool();
}

TGE_DLLEXPORT bool hideReferences(uint count, ReferenceKey* keys)
{
	return bool();
}

TGE_DLLEXPORT bool deleteReferences(uint count, ReferenceKey* keys)
{
	return bool();
}
