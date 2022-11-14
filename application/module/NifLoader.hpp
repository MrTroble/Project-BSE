#pragma once

#include <string>
#include <cinttypes>
#include <Module.hpp>
#include <vector>
#include <unordered_map>

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

		std::vector<char> vertexFile;
		std::vector<char> fragmentsFile;
		mutable std::unordered_map<std::vector<std::string>, void*> shaderCache;
		tge::main::Error init();

		size_t load(const std::string& name, const NodeTransform& baseTransform, void* shaderPipe = nullptr) const;
	};

	nif::NifModule* nifModule = new nif::NifModule();
}