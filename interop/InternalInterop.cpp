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
#include "../public/graphics/ElementHolder.hpp"

namespace tge::interop {

std::unordered_map<std::string, std::vector<tge::graphics::TNodeHolder>>
    REFERENCE_MAP;
std::unordered_map<tge::graphics::TNodeHolder, std::string>
    REFERENCE_MAP_TO_STRING;
std::mutex loadMutex;

inline glm::vec3 vectors(const vec3& vec3) {
  return glm::vec3(vec3.x, vec3.y, vec3.z);
}

inline glm::quat quats(const vec3& vec3) {
  auto vector = vectors(vec3);
  vector *= -1;
  return glm::quat(vector);
}

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
  std::thread thread([t = std::vector(loads, loads + count),
                      nif = tge::nif::nifModule,
                      loadList = std::move(start)]() {
    std::lock_guard guard(loadMutex);
    std::vector<tge::nif::LoadNif> nifLoadings(loadList.size());
    for (size_t i = 0; i < loadList.size(); i++) {
      nifLoadings[i] = {loadList[i].path,
                        transformFromInput(loadList[i].transform)};
    }
    const auto nodeIDs = nif->load(nifLoadings.size(), nifLoadings.data());
    for (size_t i = 0; i < nodeIDs.size(); i++) {
      const auto& formKey = loadList[i].formKey;
      const auto key = nodeIDs[i][0];
      REFERENCE_MAP[formKey] = nodeIDs[i];
      REFERENCE_MAP_TO_STRING[key] = formKey;
    }
    callLoadFinishedCallback(t.size(), t.data());
  });
  thread.detach();
  return true;
}

bool update(const uint count, const ReferenceUpdate* keys) { return false; }

bool hide(const uint count, const FormKey* keys, const bool hide) {
  return false;
}

bool remove(const uint count, const FormKey* keys) {
  std::vector<graphics::TNodeHolder> ids(count);
  for (size_t i = 0; i < count; i++) {
    const auto iterator = REFERENCE_MAP.find(keys[i]);
    if (iterator == std::end(REFERENCE_MAP)) {
      PLOG_DEBUG << "Reference is not found name=" << keys[i];
      ids[i] = SIZE_MAX;
      continue;
    }
    ids[i] = iterator->second[0];
  }
  tge::nif::nifModule->remove(ids.size(), ids.data());
  return true;
}

std::vector<tge::graphics::TNodeHolder> currentSelected;
constexpr glm::vec4 SELECT_COLOR(0.5f, 0, 0, 1.0f);
constexpr glm::vec4 NORMAL_COLOR(0.0f, 0, 0, 1.0f);
std::mutex selectionMutex;

bool select(const uint count, const FormKey* keys) {
  std::lock_guard guard(selectionMutex);
  auto api = tge::main::getAPILayer();
  auto ggm = api->getGraphicsModule();
  auto& internalValues = ggm->nodeHolder.internalValues;
  auto& transform = ggm->nodeHolder.translationTable;
  auto& statusValues = std::get<4>(internalValues);
  auto& shaderValues = std::get<5>(internalValues);
  auto& childValues = std::get<6>(internalValues);
  const auto& binding = std::get<3>(internalValues);
  for (const auto holder : currentSelected) {
    ggm->nodeHolder.change<5>(holder).data.color = NORMAL_COLOR;
    ggm->nodeHolder.change<4>(holder) = 1;
  }
  currentSelected.clear();
  PLOG_DEBUG << "Selected: " << count;
  for (size_t i = 0; i < count; i++) {
    const auto formKey = keys[i];
    PLOG_DEBUG << formKey;
    const auto iterator = REFERENCE_MAP.find(formKey);
    if (iterator == std::end(REFERENCE_MAP)) continue;
    const auto rootNode = iterator->second[0];
    std::vector<size_t> nodes = {rootNode.internalHandle};
    std::lock_guard guard(ggm->nodeHolder.mutex);
    while (!nodes.empty()) {
      const auto node = nodes.back();
      nodes.pop_back();
      const auto pos = transform[node];
      const auto& childs = childValues[pos];
      nodes.insert(nodes.end(), childs.begin(), childs.end());
      if (!binding[pos]) continue;
      shaderValues[pos].color = SELECT_COLOR;
      statusValues[pos] = 1;
      currentSelected.push_back(node);
    }
  }
  return true;
}

constexpr float AMOUNT_FINAL = 0.00142875f;
constexpr float AMOUNT_OF = 128 * AMOUNT_FINAL;

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

    cornerSets[i] = {toInternal(info.cornerSets), (uint32_t)info.point_size};

    auto heights = bufferIn + info.positionBegin;
    for (size_t y = 0; y < info.point_size; y++) {
      const auto yAmount = y * 128 + info.y;
      const auto yConst = y * info.point_size;
      for (size_t x = 0; x < info.point_size; x++) {
        const auto value = x + yConst;
        positions[value] = glm::vec3(x * 128 + info.x, yAmount, heights[value]);
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

  auto data = api->pushData(bufferHolder.size(), bufferHolder.data(), "Terrain?");

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

void* getMainWindowHandle() { return tge::main::getMainWindowHandle(); }

}  // namespace tge::interop