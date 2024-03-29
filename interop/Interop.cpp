#include "Interop.hpp"

#include <iostream>
#include <vector>

#include "InternalInterop.hpp"

std::vector<LoadCallback> loadCallbacks{&tge::interop::load};
std::vector<FormKeyCallback> deleteCallbacks{&tge::interop::remove};
std::vector<UpdateCallback> updateCallbacks{&tge::interop::update};
std::vector<HideCallback> hideCallbacks{&tge::interop::hide};
std::vector<FormKeyCallback> selectCallbacks{&tge::interop::select};
std::vector<TerrainAddCallback> terrainCallbacks{&tge::interop::terrain};
std::vector<LoadFinishedCallback> loadFinished{};

#define ADD_OR_RETURN_ON_FAIL(cCallbackVector, cCallbackPtr)   \
  if (cCallbackPtr == nullptr) return false;                   \
  const auto endItr = end(cCallbackVector);                    \
  const auto founditr =                                        \
      std::find(begin(cCallbackVector), endItr, cCallbackPtr); \
  if (founditr != endItr) return false;                        \
  cCallbackVector.push_back(cCallbackPtr);                     \
  return true;

#define CALL_FOR_EACH(cCallbackVector, ...)   \
  for (const auto callback : cCallbackVector) \
    if (!callback(__VA_ARGS__)) return false; \
  return true;

#define ASSERT_VALID_POINTER(cCount, cPointer)         \
  if (cPointer == nullptr && cCount > 0) return false; \
  if (cCount == 0) return true;

bool addLoadCallback(LoadCallback callback) {
  ADD_OR_RETURN_ON_FAIL(loadCallbacks, callback);
}

bool addUpdateCallback(UpdateCallback callback) {
  ADD_OR_RETURN_ON_FAIL(updateCallbacks, callback);
}

bool addHideCallback(HideCallback callback) {
  ADD_OR_RETURN_ON_FAIL(hideCallbacks, callback);
}

bool addDeleteCallback(FormKeyCallback callback) {
  ADD_OR_RETURN_ON_FAIL(deleteCallbacks, callback);
}

bool addLoadFinishedCallback(LoadFinishedCallback callback) {
  ADD_OR_RETURN_ON_FAIL(loadFinished, callback);
}

bool addSelectCallback(FormKeyCallback callback) {
  ADD_OR_RETURN_ON_FAIL(selectCallbacks, callback);
}

bool addTerrainCallback(TerrainAddCallback callback) {
  ADD_OR_RETURN_ON_FAIL(terrainCallbacks, callback);
}

void* getMainWindowHandle()
{
    return tge::interop::getMainWindowHandle();
}

void callLoadFinishedCallback() {
  for (auto load : loadFinished) load();
}

bool loadReferences(uint count, ReferenceLoad* load) {
  ASSERT_VALID_POINTER(count, load);
  CALL_FOR_EACH(loadCallbacks, count, load);
}

bool updateReferences(uint count, ReferenceUpdate* keys) {
  ASSERT_VALID_POINTER(count, keys);
  CALL_FOR_EACH(updateCallbacks, count, keys);
}

bool hideReferences(uint count, FormKey* keys, bool hide) {
  ASSERT_VALID_POINTER(count, keys);
  CALL_FOR_EACH(hideCallbacks, count, keys, hide);
}

bool loadTerrain(uint count, TerrainInfo* info, float* buffer) {
  ASSERT_VALID_POINTER(count, buffer);
  CALL_FOR_EACH(terrainCallbacks, count, info, buffer);
}

bool deleteReferences(uint count, FormKey* keys) {
  ASSERT_VALID_POINTER(count, keys);
  CALL_FOR_EACH(deleteCallbacks, count, keys);
}

bool selectReferences(uint count, FormKey* keys) {
  ASSERT_VALID_POINTER(count, keys);
  CALL_FOR_EACH(selectCallbacks, count, keys);
}
