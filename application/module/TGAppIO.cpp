#include "TGAppIO.hpp"
#include "TGAppIO.hpp"

#include <TGEngine.hpp>
#include <graphics/vulkan/VulkanModuleDef.hpp>

#include "../../interop/InternalInterop.hpp"

std::array<IOFunctionBinding, IOFunction::_size()> functionBindings = {
	// Rotating
	IOFunctionBinding{IOFunctionBindingType::Scroll, 1},
	IOFunctionBinding{IOFunctionBindingType::Scroll, -1},
	IOFunctionBinding{IOFunctionBindingType::Keyboard, 'W'},
	IOFunctionBinding{IOFunctionBindingType::Keyboard, 'S'},
	IOFunctionBinding{IOFunctionBindingType::Keyboard, 'Q'},
	IOFunctionBinding{IOFunctionBindingType::Keyboard, 'E'},
	IOFunctionBinding{IOFunctionBindingType::Keyboard, 'R'},
	// Free
	IOFunctionBinding{IOFunctionBindingType::Keyboard, 'W'},
	IOFunctionBinding{IOFunctionBindingType::Keyboard, 'S'},
	IOFunctionBinding{IOFunctionBindingType::Keyboard, 'A'},
	IOFunctionBinding{IOFunctionBindingType::Keyboard, 'D'},
	IOFunctionBinding{IOFunctionBindingType::Keyboard, 'Q'},
	IOFunctionBinding{IOFunctionBindingType::Keyboard, 'E'},
	IOFunctionBinding{IOFunctionBindingType::Scroll, 1},
	IOFunctionBinding{IOFunctionBindingType::Scroll, -1},
	IOFunctionBinding{IOFunctionBindingType::Keyboard, 'R'},
	// Other
	IOFunctionBinding{IOFunctionBindingType::Mouse, 1},
	IOFunctionBinding{IOFunctionBindingType::Keyboard, SpecialKeys::Shift},
	IOFunctionBinding{IOFunctionBindingType::Mouse, 3}
};

void TGAppIO::getImageIDFromBackend() {
	auto api =
		(tge::graphics::VulkanGraphicsModule*)tge::main::getAPILayer()->backend();
	this->imageID = api->internalImageData[3];
}

void TGAppIO::selectInternal() {
	tge::interop::internalSelect(selectedIDs.size(), selectedIDs.data());
}