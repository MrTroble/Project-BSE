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

#undef min
#undef max

using namespace tge::main;
using namespace tge::graphics;
using namespace tge;

bool finishedLoading = false;
std::mutex waitMutex;

int initTGEditor(const InitConfig* config) {
  waitMutex.lock();
  PLOG_DEBUG << config;
  if (config == nullptr) {
    PLOG(plog::fatal) << "Config must not be null!" << std::endl;
    waitMutex.unlock();
    return -1;
  }

  if (config->version != 2) {
    PLOG(plog::fatal) << "Wrong version number in config!" << std::endl;
    waitMutex.unlock();
    return -1;
  }

  PLOG_DEBUG << config->sizeOfBSA;
  lateModules.push_back(guiModul);
  lateModules.push_back(ioModul);
  lateModules.push_back(tge::nif::nifModule);
  tge::nif::nifModule->assetDirectory = config->assetDirectory;
  tge::nif::nifModule->archiveNames.resize(config->sizeOfBSA);
  char** namelist = config->bsaFiles;
  for (auto& name : tge::nif::nifModule->archiveNames) {
    name = std::string(*(namelist++));
    PLOG_DEBUG << name;
  }
  auto& dir = tge::nif::nifModule->assetDirectory;
  if (dir.back() != END_CHARACTER) dir += END_CHARACTER;

  const auto initResult = init();
  waitMutex.unlock();
  if (initResult != main::Error::NONE) {
    PLOG(plog::fatal) << "Error in init!" << std::endl;
    return -1;
  }
  auto api = getAPILayer();

  ioModul->ggm = getGameGraphicsModule();
  guiModul->api = api;
  guiModul->ggm = ioModul->ggm;
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
  if (startResult != main::Error::NONE) {
    PLOG(plog::fatal) << "Error in start!" << std::endl;
    return -1;
  }
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
