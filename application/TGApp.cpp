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
#include "NifLoader.hpp"
#include <glm/glm.hpp>
#undef min
#undef max

using namespace tge::main;
using namespace tge::graphics;
using namespace tge;

int main(const int count, const char **strings)
{
	nif::NifModule* nifModule = new nif::NifModule();
	lateModules.push_back(guiModul);
	lateModules.push_back(ioModul);	
	lateModules.push_back(nifModule);

	const auto initResult = init();
	if (initResult != main::Error::NONE)
	{
		printf("Error in init!");
		return -1;
	}
	auto api = getAPILayer();

	ioModul->ggm = getGameGraphicsModule();
	guiModul->api = api;
	guiModul->ggm = ioModul->ggm;
	ioModul->ggm->updateViewMatrix(glm::perspective(glm::radians(45.0f), 1.0f, 0.01f, 10000.0f));

	ioModul->nodeID = nifModule->load("assets/wrhouse02.nif");
	guiModul->nodeID = nifModule->load("assets/wrhouse02.nif");

	auto &light = guiModul->light;
	light.color = glm::vec3(1, 1, 1);
	light.pos = glm::vec3(0, 10, 0);
	light.intensity = 1.0f;
	guiModul->lightID = api->pushLights(1, &light);

	const auto startResult = start();
	if (startResult != main::Error::NONE)
	{
		printf("Error in start!");
		return -1;
	}

	return (uint32_t)startResult;
}
