#pragma once

#include <string>
#include <cinttypes>
#include <Module.hpp>

namespace tge::nif {

	class NifModule : public tge::main::Module {

	public:
		tge::main::Error init();

		size_t load(const std::string& name, void* shaderPipe = nullptr) const;
	};

}