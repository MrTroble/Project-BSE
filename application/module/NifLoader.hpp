#pragma once

#include <Module.hpp>
#include <cinttypes>
#include <graphics/GameGraphicsModule.hpp>
#include <string>
#include <unordered_map>
#include <vector>

namespace std {
template <>
struct hash<std::vector<std::string>> {
  _NODISCARD size_t
  operator()(const std::vector<std::string>& value) const noexcept {
    size_t hash = 0;
    std::hash<std::string> hasher;
    for (const auto& string : value) {
      hash ^= hasher(string);
    }
    return hash;
  }
};
}  // namespace std

namespace tge::nif {

struct LoadNif {
  std::string file;
  tge::graphics::NodeTransform transform;
};

struct LoadedModelInformation {
  size_t render = 0;
  size_t nifHandle = 0;
  std::vector<size_t> buffer;
  std::vector<size_t> textures;
  size_t referenceCount = 0;
};

class NifModule : public tge::main::Module {
 public:
  bool finishedLoading = false;
  std::vector<char> vertexFile;
  std::vector<char> fragmentsFile;
  std::string assetDirectory;
  std::vector<std::string> archiveNames;
  std::mutex shaderCacheMutex;
  std::unordered_map<std::vector<std::string>,
                     std::pair<tge::graphics::TPipelineHolder, void*>>
      shaderCache;
  std::unordered_map<std::string, LoadedModelInformation> loadInformation;
  float translationFactor = 0.00142875f;
  graphics::TNodeHolder basicNifNode;
  graphics::TSamplerHolder samplerID;
  std::unordered_map<graphics::TNodeHolder, std::pair<graphics::TRenderHolder,
                                                      graphics::TRenderHolder>>
      nodeIdToRender;

  tge::main::Error init();

  std::vector<std::vector<graphics::TNodeHolder>> load(
      const size_t count, const LoadNif* loads, void* shaderPipe = nullptr);

  void remove(const size_t size, const graphics::TNodeHolder* ids);
};

extern nif::NifModule* nifModule;
}  // namespace tge::nif