#pragma once

#include <IO/IOModule.hpp>
#include <array>
#include <glm/gtx/transform.hpp>
#include <graphics/GameGraphicsModule.hpp>

constexpr float offset = 2.0f;

class TGAppIO : public tge::io::IOModule {
 public:
  tge::graphics::GameGraphicsModule* ggm;
  size_t nodeID;
  size_t imageID;
  tge::graphics::NodeTransform transform;
  glm::vec2 vec;
  glm::vec3 cache{};
  tge::graphics::CacheIndex index;
  bool pressedMiddle = false;
  float scale = 1;
  std::array<bool, 255> stack = {false};
  bool pressedLeft = false;

  tge::main::Error init() override;

  void selectInternal(const size_t size);

  void tick(double deltatime) override {
    const auto actualOffset = offset * deltatime;
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
      const auto imageData = ggm->getAPILayer()->getImageData(imageID, &index);
      const auto bounds = ggm->getAPILayer()->getRenderExtent();
      const auto fbuffer = (float*)imageData.data();
      const auto offset = (size_t)(bounds.x * vec.y) + (size_t)vec.x;
      if (imageData.size() > offset*sizeof(float)) {
        const float idSelected = fbuffer[offset];
        selectInternal(static_cast<size_t>(idSelected));
      }
    }

    std::fill(begin(stack), end(stack), false);
    ggm->updateCameraMatrix(
        glm::lookAt(cache, glm::vec3(0, 1, 0) + cache, glm::vec3(0, 0, -1)));
  }

  void mouseEvent(const tge::io::MouseEvent event) override {
    if (event.pressed == 1) {
      pressedLeft = true;
      vec = glm::vec2(event.x, event.y);
    }
  }

  void keyboardEvent(const tge::io::KeyboardEvent event) override {
    if (event.signal < 255) {
      stack[event.signal] = true;
    }
  }

  void recreate() override {
    const auto extent = ggm->getAPILayer()->getRenderExtent();
    ggm->updateViewMatrix(glm::perspective(
        glm::radians(45.0f), extent.x / extent.y, 0.01f, 10000.0f));
    index.buffer = SIZE_MAX;
    // TODO delete old buffer
  }
};
