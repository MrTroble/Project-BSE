#define SPR_NO_DEBUG_OUTPUT 1
#include "TGApp.hpp"

#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>

#include <IO/IOModule.hpp>
#include <TGEngine.hpp>
#include <Util.hpp>
#include <array>
#include <chrono>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <graphics/GameGraphicsModule.hpp>
#include <iostream>
#include <limits>
#include <string>
#include <thread>

#include "module/NifLoader.hpp"
#include "module/TGAppGUI.hpp"
#include "module/TGAppIO.hpp"
#include "module/TerrainModule.hpp"

#undef min
#undef max

using namespace tge::main;
using namespace tge::graphics;
using namespace tge;

bool finishedLoading = false;
std::mutex waitMutex;

int initTGEditor(const InitConfig* config, const char** bsaFiles,
                 const size_t sizeOfBSAs) {
  waitMutex.lock();

  if (config == nullptr) {
    std::cerr << "Config must not be null!" << std::endl;
    waitMutex.unlock();
    return -1;
  }

  if (config->version != CURRENT_INIT_VERSION) {
    std::cerr << "Wrong version number in config!" << std::endl;
    waitMutex.unlock();
    return -1;
  }

  lateModules.push_back(ioModul);
  lateModules.push_back(guiModul);
  lateModules.push_back(tge::nif::nifModule);
  lateModules.push_back(terrainModule);
  terrainModule->api = getAPILayer();
  tge::nif::nifModule->assetDirectory = config->assetDirectory;
  tge::nif::nifModule->archiveNames.resize(sizeOfBSAs);
  const char** namelist = bsaFiles;
  for (auto& name : tge::nif::nifModule->archiveNames) {
    name = std::string(*(namelist++));
  }
  auto& dir = tge::nif::nifModule->assetDirectory;
  if (dir.back() != END_CHARACTER) dir += END_CHARACTER;

  const auto initResult = init(config->featureSet);
  waitMutex.unlock();
  if (initResult != main::Error::NONE) {
    PLOG_FATAL << "Error in init!";
    return -1;
  }
  auto api = getAPILayer();

  ioModul->ggm = getGameGraphicsModule();
  guiModul->api = api;
  guiModul->winModule = ioModul->ggm->getWindowModule();
  const auto extent = api->getRenderExtent();
  ioModul->ggm->updateViewMatrix(glm::perspective(
      glm::radians(45.0f), extent.x / extent.y, 0.01f, 10000.0f));

  auto& light = guiModul->light;
  light.color = glm::vec3(1, 1, 1);
  light.pos = glm::vec3(0, 10, 0);
  light.intensity = 1.0f;
  guiModul->lightID = api->pushLights(1, &light);

  finishedLoading = true;
  const auto startResult = start();
  finishedLoading = false;
  return (uint32_t)startResult;
}

bool isFinished() { return finishedLoading; }

void waitFinishedInit() {
  using namespace std::chrono_literals;
  do {
    std::this_thread::sleep_for(10ms);
    std::lock_guard aquired(waitMutex);
  } while (!isFinished());
}

TGE_DLLEXPORT SizeInformation getSizeInfo() { return {}; }

void updateKeybindings(const KeyBindings bindings) {
    std::copy(std::begin(bindings.bindingList), std::begin(bindings.bindingList) + IOFunction::_size(), functionBindings.data());
}

void getKeybindings(KeyBindings* bindings) {
    std::copy(functionBindings.begin(), functionBindings.end(), std::begin(bindings->bindingList));
}

void enumerateKeyBindingNames(const char** stringsToWrite, size_t* amount) {
    if (stringsToWrite == nullptr) {
        *amount = IOFunction::_size();
        return;
    }
    size_t counter = 0;
    for (const auto function : IOFunction::_values())
    {
        *stringsToWrite++ = function._to_string();
        counter++;
        if (counter == *amount) break;
    }
}