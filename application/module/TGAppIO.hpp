#pragma once

#include <IO/IOModule.hpp>
#include <array>
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
  glm::vec3 cache{};
  bool pressedMiddle = false;
  float scale = 1;
  std::array<bool, 255> stack = {false};
  bool pressedLeft = false;
  bool pressedShift = false;

  void getImageIDFromBackend();

  tge::main::Error init() override  {
    getImageIDFromBackend();
    return tge::io::IOModule::init();
  }

  void selectInternal();

  void tick(double deltatime) override {
    const float actualOffset = (float)(offset * deltatime);
    if (stack['W']) {
      cache.y += actualOffset;
    }
    if (stack['S']) {
      cache.y -= actualOffset;
    }
    if (stack['A']) {
      cache.x += actualOffset;
    }
    if (stack['D']) {
      cache.x -= actualOffset;
    }
    if (stack['Q']) {
      cache.z += actualOffset;
    }
    if (stack['E']) {
      cache.z -= actualOffset;
    }

    if (stack['R']) {
      cache = glm::vec3(0);
    }

    if (pressedLeft) {
      pressedLeft = false;
      const auto [imageData, internalDataHolder] = ggm->getAPILayer()->getImageData(imageID, dataHolder);
      dataHolder = internalDataHolder;
      const auto bounds = ggm->getAPILayer()->getRenderExtent();
      const auto fbuffer = (float*)imageData.data();
      const auto offset = (size_t)(bounds.x * vec.y) + (size_t)vec.x;
      if (imageData.size() > offset * sizeof(float)) {
        const size_t idSelected = static_cast<size_t>(fbuffer[offset]);
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
        glm::lookAt(cache, glm::vec3(0, 1, 0) + cache, glm::vec3(0, 0, -1)));
  }

  void mouseEvent(const tge::io::MouseEvent &event) override {
    if (event.pressed == 1) {
      if (event.pressMode == tge::io::PressMode::CLICKED) {
        pressedLeft = true;
        vec = glm::vec2(event.x, event.y);
        if (event.additional & 8) {
          pressedShift = true;
        }
      }
    }
  }

  void keyboardEvent(const tge::io::KeyboardEvent &event) override {
    if (event.signal < 255) {
      stack[event.signal] = true;
    }
  }

  void recreate() override {
    const auto extent = ggm->getAPILayer()->getRenderExtent();
    ggm->updateViewMatrix(glm::perspective(
        glm::radians(45.0f), extent.x / extent.y, 0.01f, 10000.0f));
    dataHolder = tge::graphics::TDataHolder();
    getImageIDFromBackend();
    // TODO Remove buffer;
  }
};
