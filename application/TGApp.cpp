#define SPR_NO_DEBUG_OUTPUT 1
#include "TGApp.hpp"
#include "module/TGAppGUI.hpp"
#include <IO/IOModule.hpp>
#include <TGEngine.hpp>
#include <Util.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <graphics/GameGraphicsModule.hpp>
#include "module/TGAppIO.hpp"
#include <array>
#include <cmath>
#include <limits>
#include <string>
#undef min
#undef max

using namespace tge::main;
using namespace tge::graphics;
using namespace tge;

int main(const int count, const char **strings)
{
	lateModules.push_back(guiModul);
	lateModules.push_back(ioModul);

	const auto initResult = init();
	if (initResult != main::Error::NONE)
	{
		printf("Error in init!");
		return -1;
	}
	auto api = getAPILayer();

	auto &light = guiModul->light;
	light.color = glm::vec3(1, 1, 1);
	light.pos = glm::vec3(0, 10, 0);
	light.intensity = 1.0f;
	api->pushLights(1, &light);

	const auto startResult = start();
	if (startResult != main::Error::NONE)
	{
		printf("Error in start!");
		return -1;
	}

	return (uint32_t)startResult;
}
