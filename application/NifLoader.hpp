#pragma once

#include <string>
#include <cinttypes>

namespace tge::nif {

	size_t load(const std::string& name, void* shaderPipe = nullptr);

}