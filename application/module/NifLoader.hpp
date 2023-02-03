#pragma once

#include <string>
#include <cinttypes>
#include <Module.hpp>
#include <vector>
#include <unordered_map>
#include <graphics/GameGraphicsModule.hpp>

namespace std {
	template <>
	struct hash<std::vector<std::string>> {

		_NODISCARD size_t operator()(const std::vector<std::string>& value) const noexcept {
			size_t hash = 0;
			std::hash<std::string> hasher;
			for (const auto& string : value) {
				hash ^= hasher(string);
			}
			return hash;
		}
	};
}

namespace tge::nif {

	class NifModule : public tge::main::Module {
	public:
		bool finishedLoading = false;
		std::vector<char> vertexFile;
		std::vector<char> fragmentsFile;
		std::string assetDirectory;
		std::unordered_map<std::vector<std::string>, void*> shaderCache;
        float translationFactor = 0.00142875f;
        uint32_t basicNifNode;
        size_t samplerID;
        std::unordered_map<size_t, size_t> nodeIdToRender;

		tge::main::Error init();

		size_t load(const std::string& name, const tge::graphics::NodeTransform& baseTransform, void* shaderPipe = nullptr);

		void remove(const size_t size, const size_t* ids);
	};

	extern nif::NifModule* nifModule;
}