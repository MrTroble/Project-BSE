#include "Interop.hpp"
#include <iostream>
#include <vector>

std::vector<LoadCallback> loadCallbacks;
std::vector<DeleteCallback> deleteCallbacks;
std::vector<UpdateCallback> updateCallbacks;
std::vector<HideCallback> hideCallbacks;

#define ADD_OR_RETURN_ON_FAIL(cCallbackVector, cCallbackPtr) const auto endItr = end(cCallbackVector);\
const auto founditr = std::find(begin(cCallbackVector), endItr, cCallbackPtr); \
if (founditr != endItr) \
return false;\
cCallbackVector.push_back(cCallbackPtr);\
return true;

bool addLoadCallback(LoadCallback callback) {
	ADD_OR_RETURN_ON_FAIL(loadCallbacks, callback);
}

bool addUpdateCallback(UpdateCallback callback) {
	ADD_OR_RETURN_ON_FAIL(updateCallbacks, callback);
}

bool addHideCallback(HideCallback callback) {
	ADD_OR_RETURN_ON_FAIL(hideCallbacks, callback);
}

bool addDeleteCallback(DeleteCallback callback) {
	ADD_OR_RETURN_ON_FAIL(deleteCallbacks, callback);
}


void loadReferences(uint count, ReferenceLoad* load) {
	for (const auto callback : loadCallbacks)
		callback(count, load);
}

bool updateReferences(uint count, ReferenceUpdate* keys)
{
	return bool();
}

bool hideReferences(uint count, FormKey* keys, bool hide)
{
	return bool();
}

bool deleteReferences(uint count, FormKey* keys)
{
	return bool();
}
