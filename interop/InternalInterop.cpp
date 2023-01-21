#include "InternalInterop.hpp"

#include <TGEngine.hpp>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

#include "../application/module/NifLoader.hpp"

namespace tge::interop {

std::unordered_map<std::string, size_t> REFERENCE_MAP;
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
        const auto ggm = tge::main::getGameGraphicsModule();
        for (const auto& load : loadList) {
          std::lock_guard guard(loadMutex);
          const auto newTranform = transformFromInput(load.transform);
          const auto nodeID = nif->load(load.path, newTranform);
          if (nodeID == SIZE_MAX) return;
          REFERENCE_MAP[load.formKey] = nodeID;
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
    ids[i] = REFERENCE_MAP[keys[i]];
  }
  tge::nif::nifModule->remove(ids.size(), ids.data());
  return true;
}

}  // namespace tge::interop