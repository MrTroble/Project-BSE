#include "InternalInterop.hpp"

#include <Util.hpp>
#include <TGEngine.hpp>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

#include "../application/module/NifLoader.hpp"

namespace tge::interop {

std::unordered_map<std::string, size_t> REFERENCE_MAP;
std::unordered_map<size_t, std::string> REFERENCE_MAP_TO_STRING;
std::mutex loadMutex;

inline glm::vec3 vectors(const vec3& vec3) {
  return glm::vec3(vec3.x, vec3.y, vec3.z);
}

inline glm::quat quats(const vec3& vec3) { return glm::quat(vectors(vec3)); }

inline tge::graphics::NodeTransform transformFromInput(
    const ReferenceTransform& transform) {
  return {vectors(transform.translation), vectors(transform.scale),
          quats(transform.rotations)};
}

struct InternalLoad {
  std::string formKey;
  std::string path;
  ReferenceTransform transform = TGE_DEFAULT_TRANSFORM;
};

bool load(const uint count, const ReferenceLoad* loads) {
  std::vector<InternalLoad> start(count);
  for (size_t i = 0; i < count; i++) {
    InternalLoad& load = start[i];
    const ReferenceLoad& old = loads[i];
    load.transform = old.transform;
    load.formKey = std::string(old.formKey);
    load.path = std::string(old.path);
  }
  std::thread thread(
      [nif = tge::nif::nifModule, loadList = std::move(start)]() {
        std::lock_guard guard(loadMutex);
        std::vector<tge::nif::LoadNif> nifLoadings(loadList.size());
        for (size_t i = 0; i < loadList.size(); i++) {
          nifLoadings[i] = {loadList[i].path,
                            transformFromInput(loadList[i].transform)};
        }
        const auto nodeIDs = nif->load(nifLoadings.size(), nifLoadings.data());
        for (size_t i = 0; i < nodeIDs.size(); i++) {
          const auto& formKey = loadList[i].formKey;
          const auto id = nodeIDs[i];
          REFERENCE_MAP[formKey] = id;
          REFERENCE_MAP_TO_STRING[id] = formKey;
        }
        callLoadFinishedCallback();
      });
  thread.detach();
  return true;
}

bool update(const uint count, const ReferenceUpdate* keys) { return false; }

bool hide(const uint count, const FormKey* keys, const bool hide) {
  return false;
}

bool remove(const uint count, const FormKey* keys) {
  std::vector<size_t> ids(count);
  for (size_t i = 0; i < count; i++) {
    const auto iterator = REFERENCE_MAP.find(keys[i]);
    if (iterator == std::end(REFERENCE_MAP)) {
      PLOG_DEBUG << "Reference is not found name=" << keys[i];
      ids[i] = SIZE_MAX;
      continue;
    }
    ids[i] = iterator->second;
  }
  tge::nif::nifModule->remove(ids.size(), ids.data());
  return true;
}

bool select(const uint count, const FormKey* keys) {
  for (size_t i = 0; i < count; i++) {
    PLOG_DEBUG << keys[i];
  }
  return true;
}

bool terrain(const uint count, const float* keys) { return false; }

bool internalSelect(const size_t count, const size_t* ids) {
  std::vector<FormKey> vector(count);
  std::transform(ids, ids + count, vector.begin(), [](const auto id) {
    return REFERENCE_MAP_TO_STRING[id].c_str();
  });
  selectReferences(count, vector.data());
  return true;
}

}  // namespace tge::interop