#include "InternalInterop.hpp"

#include <TGEngine.hpp>
#include <Util.hpp>
#include <array>
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

constexpr size_t AMOUNT_OF = 64;

bool terrain(const uint count, const TerrainInfo* infos, float* bufferIn) {
  using namespace tge::graphics;
  std::vector<BufferInfo> bufferHolder;
  std::vector<std::vector<glm::vec3>> positionHolder;
  std::vector<std::vector<uint32_t>> indexBufferHolder;
  bufferHolder.reserve(count * 4);
  indexBufferHolder.resize(count);
  positionHolder.resize(count);
  for (size_t i = 0; i < count; i++) {
    const TerrainInfo& info = infos[i];
    const auto pointCount = info.point_size * info.point_size;
    auto& indexes = indexBufferHolder[i];
    indexes.reserve(pointCount);
    auto& positions = positionHolder[i];
    positions.resize(pointCount);

    auto heights = bufferIn + info.positionBegin;
    for (size_t x = 0; x < info.point_size; x++) {
      for (size_t y = 0; y < info.point_size; y++) {
        const auto value = x + y * info.point_size;
        positions[value] = glm::vec3(x * AMOUNT_OF + info.x, heights[value],
                                     y * AMOUNT_OF + info.y);
      }
    }

    for (size_t x = 0; x < info.point_size - 1; x++) {
      for (size_t y = 0; y < info.point_size - 1; y++) {
        const auto next = y * info.point_size;
        const auto nextLine = (y + 1) * info.point_size;
        indexes.push_back(x + next);
        indexes.push_back(x + nextLine);
        indexes.push_back(x + nextLine + 1);
        indexes.push_back(x + next + 1);
      }
    }

    const auto byteSize = pointCount * sizeof(float);
    bufferHolder.emplace_back(positions.data(), byteSize, DataType::VertexData);
    bufferHolder.emplace_back(bufferIn + info.normalBegin, byteSize * 3,
                              DataType::VertexData);
    bufferHolder.emplace_back(bufferIn + info.colorBegin, byteSize * 3,
                              DataType::VertexData);
    bufferHolder.emplace_back(indexes.data(), indexes.size(),
                              DataType::IndexData);
  }
  auto api = tge::main::getAPILayer();

  auto data = api->pushData(bufferHolder.size(), bufferHolder.data());

  std::thread thread(
      [dataHolder = std::move(data),
       terrains = std::vector(infos, infos + count), api = api]() {
        const auto ggm = api->getGraphicsModule();
        auto iterator = dataHolder.begin();
        std::vector<RenderInfo> info;
        info.resize(terrains.size());
        auto render = info.begin();
        for (const auto& terrain : terrains) {
          auto& renderInfo = *render;
          renderInfo.vertexBuffer = std::vector(iterator, iterator + 3);
          renderInfo.indexBuffer = iterator[3];
          renderInfo.indexCount =
              (terrain.point_size - 1) * (terrain.point_size - 1);
          renderInfo.materialId = ggm->defaultMaterial;
          iterator += 4;
        }
        api->pushRender(info.size(), info.data());
      });
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