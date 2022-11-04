#include "Interop.hpp"
#include <iostream>
#include <vector>

std::vector<LoadCallback> loadCallbacks;

void loadReferences(uint count, RefernceLoad* load) {
	for (const auto callback : loadCallbacks)
		callback(count, load);
}

bool updateReferences(uint count, RefernceUpdate* keys)
{
	return bool();
}

bool addLoadHook(LoadCallback callback)
{
	loadCallbacks.push_back(callback);
	return true;
}

bool hideReferences(uint count, FormKey* keys, bool hide)
{
	return bool();
}

bool deleteReferences(uint count, FormKey* keys)
{
	return bool();
}
