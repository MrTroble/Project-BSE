#pragma once

#include <IO/IOModule.hpp>
#include <array>
#define GLM_ENABLE_EXPERIMENTAL 1
#include <glm/gtx/transform.hpp>
#include <graphics/GameGraphicsModule.hpp>
#include <vector>
#include <headerlibs/enum.h>

constexpr float offset = 2.0f;

BETTER_ENUM(CameraModel, char, Rotating, Free_Cam);
BETTER_ENUM(IOFunctionBindingType, uint32_t, Keyboard, Mouse, Scroll, None);
BETTER_ENUM(IOFunction, uint32_t, Rotating_Forward, Rotating_Backwards, //
	Rotating_Speed_Add, Rotating_Speed_Reduce, Rotating_Up, Rotating_Down, Rotating_Reset,//
	Free_Forward, Free_Backwards, Free_Left, Free_Right, Free_Up, Free_Down, Free_Reset,//
	Select, Multi_Select_Modifier, Move_Camera);

struct IOFunctionBinding {
	IOFunctionBindingType type = IOFunctionBindingType::None;
	int32_t key = -1;
};

extern std::array<IOFunctionBinding, IOFunction::_size()> functionBindings;
constexpr float SPEED_MULTIPLIER = 10;

BETTER_ENUM(SpecialKeys, uint32_t, Shift=126);

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
	float speed = 1;
	std::array<tge::io::PressMode, 255> stack = { tge::io::PressMode::UNKNOWN };
	glm::mat4 oldView = glm::identity<glm::mat4>();
	glm::vec4 oldRotX{};
	glm::vec4 oldRotY{};
	glm::vec4 lookAt{ 0, 1.0, 0, 0 };
	CameraModel cameraModel = CameraModel::Rotating;
	double scrollStack = 0;
	std::array<tge::io::PressMode, 16> mouseStack = { tge::io::PressMode::UNKNOWN };

	void getImageIDFromBackend();

	inline bool checkForBinding(const IOFunctionBinding binding) {
		const auto realKey = std::abs(binding.key);
		switch (binding.type)
		{
		case IOFunctionBindingType::Keyboard:
			if (stack[realKey] == tge::io::PressMode::CLICKED) return true;
			return binding.key > 0 &&
				stack[realKey] == tge::io::PressMode::HOLD;
		case IOFunctionBindingType::Mouse:
			if (mouseStack[realKey] == tge::io::PressMode::CLICKED) return true;
			return binding.key > 0 &&
				mouseStack[realKey] == tge::io::PressMode::HOLD;
		case IOFunctionBindingType::Scroll:
			return scrollStack * binding.key > 0.0;
		default:
			return false;
		}
	}

	inline bool checkForBinding(const IOFunction function) {
		return checkForBinding(functionBindings[function._to_index()]);
	}

	tge::main::Error init() override {
		std::fill(begin(mouseStack), end(mouseStack), tge::io::PressMode::RELEASED);
		std::fill(begin(stack), end(stack), tge::io::PressMode::RELEASED);
		getImageIDFromBackend();
		return tge::io::IOModule::init();
	}

	void selectInternal();

	void changeCameraModel(CameraModel newModel) {
		if (cameraModel == newModel) return;
		cache = glm::vec3{ 0 };
		lookAt = glm::vec4{ 0, 1.0, 0, 0 };
		cameraModel = newModel;
	}

	void tick(double deltatime) override {
		tge::io::IOModule::tick(deltatime);
		const auto currentVP = glm::inverse(ggm->getVPMatrix());
		const float actualOffset = (float)(offset * deltatime * speed);

		glm::vec3 eye;
		glm::vec3 center;
		switch (cameraModel)
		{
		case CameraModel::Rotating:
			if (checkForBinding(IOFunction::Rotating_Forward)) {
				scale -= actualOffset * 2.0f;
			}
			if (checkForBinding(IOFunction::Rotating_Backwards)) {
				scale += actualOffset * 2.0f;
			}
			if (checkForBinding(IOFunction::Rotating_Speed_Add)) {
				speed += SPEED_MULTIPLIER * deltatime;
			}
			if (checkForBinding(IOFunction::Rotating_Speed_Reduce)) {
				speed -= SPEED_MULTIPLIER * deltatime;
			}
			if (checkForBinding(IOFunction::Rotating_Up)) {
				cache.z += actualOffset;
			}
			if (checkForBinding(IOFunction::Rotating_Down)) {
				cache.z -= actualOffset;
			}
			if (checkForBinding(IOFunction::Rotating_Reset)) {
				cache = glm::vec3(0);
			}
			eye = cache + glm::vec3(lookAt * scale);
			center = cache;
			break;
		case CameraModel::Free_Cam:
			glm::vec3 yDir(glm::normalize(currentVP * glm::vec4(1.0f, 0.0, 0.0, 0.0)) * actualOffset);
			glm::vec3 xDir(-yDir.y, yDir.x, 0.0f);
			scale = 1;
			if (checkForBinding(IOFunction::Free_Forward)) {
				cache -= xDir;
			}
			if (checkForBinding(IOFunction::Free_Backwards)) {
				cache += xDir;
			}
			if (checkForBinding(IOFunction::Free_Left)) {
				cache -= yDir;
			}
			if (checkForBinding(IOFunction::Free_Right)) {
				cache += yDir;
			}
			if (checkForBinding(IOFunction::Free_Up)) {
				cache.z += actualOffset;
			}
			if (checkForBinding(IOFunction::Free_Down)) {
				cache.z -= actualOffset;
			}
			if (checkForBinding(IOFunction::Free_Reset)) {
				cache = glm::vec3(0);
			}
			eye = cache;
			center = cache + glm::vec3(lookAt * scale);
			break;
		default:
			break;
		}
		speed = glm::clamp(speed, 0.001f, 1000.0f);
		scale = glm::clamp(scale, 0.001f, 1000.0f);
		oldView = glm::lookAt(eye, center, glm::vec3{ 0.0f, 0.0f, -1.0f });
		ggm->updateCameraMatrix(oldView);

		if (checkForBinding(IOFunction::Select)) {
			auto api = ggm->getAPILayer();
			const auto [imageData, internalDataHolder] =
				api->getImageData(imageID, dataHolder);
			dataHolder = internalDataHolder;
			const auto bounds = api->getRenderExtent();
			const auto dataBuffer = (float*)imageData.data();
			const auto offset = (size_t)(bounds.x * vec.y) + (size_t)vec.x;
			if (imageData.size() > offset * sizeof(float)) {
				const size_t idSelected = static_cast<size_t>(dataBuffer[offset]);
				if (!checkForBinding(IOFunction::Multi_Select_Modifier)) {
					selectedIDs.clear();
				}
				const auto end = std::end(selectedIDs);
				const auto foundIter =
					std::find(std::begin(selectedIDs), end, idSelected);
				if (foundIter == end) selectedIDs.push_back(idSelected);
				selectInternal();
			}
		}

		scrollStack = 0;
	}

	void mouseEvent(const tge::io::MouseEvent& event) override {
		using namespace tge::io;
		if (event.pressMode == PressMode::SCROLL) {
			scrollStack += event.y;
		}
		else if (event.pressMode == tge::io::PressMode::RELEASED) {
			mouseStack[event.pressed] = tge::io::PressMode::RELEASED;
		}
		else if (stack[event.pressed] == tge::io::PressMode::CLICKED) {
			mouseStack[event.pressed] = tge::io::PressMode::HOLD;
		}
		else if (event.pressMode == tge::io::PressMode::CLICKED) {
			mouseStack[event.pressed] = tge::io::PressMode::CLICKED;
		}

		constexpr auto MODIFER = 0.001f;
		if (checkForBinding(IOFunction::Move_Camera)) {
			switch (event.pressMode) {
			case PressMode::CLICKED:
				vec = glm::vec2(event.x, event.y);
				break;
			case PressMode::HOLD:
				const auto currentVP = glm::inverse(ggm->getVPMatrix());
				glm::vec2 newPos(event.x, event.y);
				auto currentDelta = (newPos - vec) * MODIFER;
				deltaMouse += currentDelta;
				vec = newPos;
				const auto newRotX = glm::rotate(-currentDelta.x, glm::vec3(currentVP * glm::vec4(0.0, 1.0f, 0.0, 0.0)));
				const auto newRotY = glm::rotate(currentDelta.y, glm::vec3(currentVP * glm::vec4(1.0f, 0.0, 0.0, 0.0)));
				lookAt = glm::normalize(newRotX * lookAt);
				lookAt = glm::normalize(newRotY * lookAt);
				break;
			}
		}
	}

	void keyboardEvent(const tge::io::KeyboardEvent& event) override {
		if (event.signal < 126) {
			if (event.mode == tge::io::PressMode::RELEASED) {
				stack[event.signal] = tge::io::PressMode::RELEASED;
			}
			else if (stack[event.signal] == tge::io::PressMode::CLICKED) {
				stack[event.signal] = tge::io::PressMode::HOLD;
			}
			else if (event.mode == tge::io::PressMode::CLICKED) {
				stack[event.signal] = tge::io::PressMode::CLICKED;
			}
		}
	}

	void recreate() override {
		const auto extent = ggm->getAPILayer()->getRenderExtent();
		ggm->updateViewMatrix(glm::perspective(
			glm::radians(45.0f), extent.x / extent.y, 0.01f, 10000.0f));
		std::array array = { dataHolder };
		if (!(!dataHolder)) ggm->getAPILayer()->removeData(array, true);
		dataHolder = tge::graphics::TDataHolder();
		getImageIDFromBackend();
		// TODO Remove buffer;
	}
};
