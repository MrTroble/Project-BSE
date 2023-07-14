#include "TGAppIO.hpp"
#include "TGAppIO.hpp"

#include <TGEngine.hpp>
#include <graphics/vulkan/VulkanModuleDef.hpp>

#include "../../interop/InternalInterop.hpp"

void TGAppIO::getImageIDFromBackend() {
  auto api =
      (tge::graphics::VulkanGraphicsModule*)tge::main::getAPILayer()->backend();
  this->imageID = api->internalImageData[3];
}

void TGAppIO::selectInternal() {
  tge::interop::internalSelect(selectedIDs.size(), selectedIDs.data());
}