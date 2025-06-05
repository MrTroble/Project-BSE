#pragma once

#include <IO/IOModule.hpp>
#include <array>
#define GLM_ENABLE_EXPERIMENTAL 1
#include <glm/gtx/transform.hpp>
#include <graphics/GameGraphicsModule.hpp>
#include <vector>

constexpr float offset = 2.0f;

class TGAppIO : public tge::io::IOModule {
 public:
  std::vector<size_t> selectedIDs;
  tge::graphics::GameGraphicsModule* ggm;
  size_t nodeID;
  tge::graphics::TTextureHolder imageID;
  tge::graphics::TDataHolder dataHolder;
  tge::graphics::NodeTransform transform;
  glm::vec2 vec;
  glm::vec2 deltaMouse{};
  glm::vec3 cache{};
  float scale = 1;
  std::array<bool, 255> stack = {false};
  bool pressedLeft = false;
  bool pressedMiddle = false;
  bool pressedShift = false;
  glm::vec3 inputRotationX = glm::vec3(1, 0, 0);
  glm::vec3 inputRotationY = glm::vec3(0, 1, 0);

  void getImageIDFromBackend();

  tge::main::Error init() override {
    getImageIDFromBackend();
    return tge::io::IOModule::init();
  }

  void selectInternal();

  void tick(double deltatime) override {
    tge::io::IOModule::tick(deltatime);
    const float actualOffset = (float)(offset * deltatime);
    if (stack['W']) {
      cache += inputRotationY * actualOffset;
    }
    if (stack['S']) {
      cache -= inputRotationY * actualOffset;
    }
    if (stack['A']) {
      cache += inputRotationX * actualOffset;
    }
    if (stack['D']) {
      cache -= inputRotationX * actualOffset;
    }
    if (stack['Q']) {
      cache.z += actualOffset;
    }
    if (stack['E']) {
      cache.z -= actualOffset;
    }

    if (stack['R']) {
      cache = glm::vec3(0);
      inputRotationX = glm::vec3(1, 0, 0);
      inputRotationY = glm::vec3(0, 1, 0);
    }

    if (pressedLeft) {
      pressedLeft = false;
      auto api = ggm->getAPILayer();
      const auto [imageData, internalDataHolder] =
          api->getImageData(imageID, dataHolder);
      dataHolder = internalDataHolder;
      const auto bounds = api->getRenderExtent();
      const auto dataBuffer = (float*)imageData.data();
      const auto offset = (size_t)(bounds.x * vec.y) + (size_t)vec.x;
      if (imageData.size() > offset * sizeof(float)) {
        const size_t idSelected = static_cast<size_t>(dataBuffer[offset]);
        if (!pressedShift) {
          selectedIDs.clear();
          pressedLeft = false;
        }
        const auto end = std::end(selectedIDs);
        const auto foundIter =
            std::find(std::begin(selectedIDs), end, idSelected);
        if (foundIter == end) selectedIDs.push_back(idSelected);
        selectInternal();
      }
    }
    std::fill(begin(stack), end(stack), false);

    ggm->updateCameraMatrix(
        glm::lookAt(cache, inputRotationY + cache, glm::vec3(0, 0, -1)));
  }

  void mouseEvent(const tge::io::MouseEvent& event) override {
    using namespace tge::io;
    if (event.pressMode == PressMode::CLICKED) {
      if (!pressedMiddle) vec = glm::vec2(event.x, event.y);
      if (event.pressed == 1) {
        pressedLeft = true;
      } else if (event.pressed == 3) {
        pressedMiddle = true;
      }
      if (event.additional & 8) {
        pressedShift = true;
      }
    } else if (event.pressMode == PressMode::RELEASED) {
      if (event.pressed == 3) {
        pressedMiddle = false;
      }
    }
    constexpr auto MODIFER = 0.001f;
    if (pressedMiddle) {
      switch (event.pressMode) {
        case PressMode::HOLD:
          glm::vec2 newPos(event.x, event.y);
          auto currentDelta = (newPos - vec);
          deltaMouse += currentDelta * MODIFER;
          vec = newPos;
          glm::quat xRotation(glm::vec3(0, 0, deltaMouse.x));
          glm::quat yRotation(glm::vec3(-deltaMouse.y, 0, 0));
          inputRotationX = glm::rotate(
              yRotation, glm::rotate(xRotation, glm::vec3(1, 0, 0)));
          inputRotationY = glm::rotate(
              yRotation, glm::rotate(xRotation, glm::vec3(0, 1, 0)));
          break;
      }
    }
  }

  void keyboardEvent(const tge::io::KeyboardEvent& event) override {
    if (event.signal < 255) {
      stack[event.signal] = true;
    }
  }

  void recreate() override {
    const auto extent = ggm->getAPILayer()->getRenderExtent();
    ggm->updateViewMatrix(glm::perspective(
        glm::radians(45.0f), extent.x / extent.y, 0.01f, 10000.0f));
    std::array array = {dataHolder};
    if (!(!dataHolder)) ggm->getAPILayer()->removeData(array, true);
    dataHolder = tge::graphics::TDataHolder();
    getImageIDFromBackend();
    // TODO Remove buffer;
  }
};
