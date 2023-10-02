#include "InternalInterop.hpp"

#include <TGEngine.hpp>
#include <Util.hpp>
#include <array>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

#include "../application/TGApp.hpp"
#include "../application/module/NifLoader.hpp"
#include "../application/module/TerrainModule.hpp"

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

constexpr float AMOUNT_OF = 64 * 0.00142875f;

bool terrain(const uint count, const TerrainInfo* infos, float* bufferIn) {
  using namespace tge::graphics;
  std::vector<BufferInfo> bufferHolder;
  std::vector<std::vector<glm::vec3>> positionHolder;
  std::vector<std::vector<uint16_t>> indexBufferHolder;
  bufferHolder.reserve(count * 4);
  indexBufferHolder.resize(count);
  positionHolder.resize(count);
  std::vector<TerrainInfoInternal> cornerSets;
  cornerSets.resize(count);
  for (size_t i = 0; i < count; i++) {
    const TerrainInfo& info = infos[i];
    const auto pointCount = info.point_size * info.point_size;
    auto& indexes = indexBufferHolder[i];
    indexes.reserve(pointCount * 6);
    auto& positions = positionHolder[i];
    positions.resize(pointCount);

    cornerSets[i] = {info.cornerSets, info.point_size};

    auto heights = bufferIn + info.positionBegin;
    for (size_t y = 0; y < info.point_size; y++) {
      const auto yAmount = y * AMOUNT_OF + info.y;
      const auto yConst = y * info.point_size;
      for (size_t x = 0; x < info.point_size; x++) {
        const auto value = x + yConst;
        positions[value] =
            glm::vec3(x * AMOUNT_OF + info.x, yAmount, heights[value]);
      }
    }

    for (size_t x = 0; x < info.point_size - 1; x++) {
      for (size_t y = 0; y < info.point_size - 1; y++) {
        const auto currentY = y * info.point_size;
        const auto bottomLeft = x + currentY;
        const auto topLeft = x + currentY + info.point_size;
        const auto topRight = topLeft + 1;
        const auto bottomRight = bottomLeft + 1;

        indexes.push_back(topRight);
        indexes.push_back(topLeft);
        indexes.push_back(bottomLeft);
        indexes.push_back(bottomLeft);
        indexes.push_back(bottomRight);
        indexes.push_back(topRight);
      }
    }

    const auto byteSize = pointCount * sizeof(float) * 3;
    bufferHolder.emplace_back(positions.data(), byteSize, DataType::VertexData);
    bufferHolder.emplace_back(bufferIn + info.normalBegin, byteSize,
                              DataType::VertexData);
    bufferHolder.emplace_back(bufferIn + info.colorBegin, byteSize,
                              DataType::VertexData);
    bufferHolder.emplace_back(indexes.data(), indexes.size() * sizeof(uint16_t),
                              DataType::IndexData);
  }
  auto api = tge::main::getAPILayer();

  auto data = api->pushData(bufferHolder.size(), bufferHolder.data());

  std::thread thread(&TerrainModule::loadTerrainSE, terrainModule, cornerSets,
                     data);
  thread.detach();
  return true;
}

bool internalSelect(const size_t count, const size_t* ids) {
  std::vector<FormKey> vector(count);
  std::transform(ids, ids + count, vector.begin(), [](const auto id) {
    return REFERENCE_MAP_TO_STRING[id].c_str();
  });
  selectReferences(count, vector.data());
  return true;
}

}  // namespace tge::interop