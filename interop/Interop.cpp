#include "Interop.hpp"
#include <iostream>

ReferenceKey loadReferences(unsigned int count, RefernceLoad* load) {
	std::cout << count << " TEST TEST";
	return 0;
}

bool updateReferences(uint count, RefernceUpdate* keys)
{
	return bool();
}

bool hideReferences(uint count, ReferenceKey* keys)
{
	return bool();
}

bool deleteReferences(uint count, ReferenceKey* keys)
{
	return bool();
}
