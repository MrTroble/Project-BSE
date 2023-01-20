#include "TGAppIO.hpp"
#include <TGEngine.hpp>
#include <graphics/vulkan/VulkanModuleDef.hpp>

tge::main::Error TGAppIO::init() { 
	auto api = (tge::graphics::VulkanGraphicsModule*)tge::main::getAPILayer();
	this->imageID = api->roughnessMetallicImage;
    return tge::io::IOModule::init();
}