#include "TGAppIO.hpp"

#include <TGEngine.hpp>
#include <graphics/vulkan/VulkanModuleDef.hpp>

#include "../../interop/InternalInterop.hpp"

tge::main::Error TGAppIO::init() {
  auto api = (tge::graphics::VulkanGraphicsModule*)tge::main::getAPILayer();
  this->imageID = api->roughnessMetallicImage;
  return tge::io::IOModule::init();
}

void TGAppIO::selectInternal(const size_t size) {
  tge::interop::internalSelect(1, &size);
}