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
	Free_Forward, Free_Backwards, Free_Left, Free_Right, Free_Up, Free_Down, Free_Speed_Add, Free_Speed_Reduce, Free_Reset,//
	Select, Multi_Select_Modifier, Move_Camera);

struct IOFunctionBinding {
	IOFunctionBindingType type = IOFunctionBindingType::None;
	int32_t key = -1;
};

extern std::array<IOFunctionBinding, IOFunction::_size()> functionBindings;
constexpr float SPEED_MULTIPLIER = 10;

BETTER_ENUM(SpecialKeys, uint32_t, Shift = 126);
BETTER_ENUM(RepressChecks, uint32_t, Select);

class TGAppIO : public tge::io::IOModule {
public:
	std::vector<size_t> selectedIDs;
	tge::graphics::GameGraphicsModule* ggm;
	size_t nodeID{};
	tge::graphics::TTextureHolder imageID;
	tge::graphics::TDataHolder dataHolder;
	tge::graphics::NodeTransform transform;
	glm::vec2 oldInputPosition{};
	glm::vec3 positionVector{};
	glm::vec4 directionVector{ 0, 1.0, 0, 0 };
	float scale = 1;
	float speed = 1;
	std::array<tge::io::PressMode, 255> keyboardPressesCache{};
	std::array<tge::io::PressMode, 16> mouseButtonsCache{};
	double scrollCache = 0;
	std::array<bool, RepressChecks::_size()> repressChecks{ false };
	CameraModel cameraModel = CameraModel::Rotating;

	void getImageIDFromBackend();

	inline bool checkForBinding(const IOFunctionBinding binding, bool* repressCheck = nullptr) const {
		// TODO Check binding
		const auto realKey = std::abs(binding.key);
		auto iterator = keyboardPressesCache.data();
		switch (binding.type)
		{
		case IOFunctionBindingType::Keyboard: break;
		case IOFunctionBindingType::Mouse:
			iterator = mouseButtonsCache.data();
			break;
		case IOFunctionBindingType::Scroll:
			return scrollCache * binding.key > 0.0;
		default:
			return false;
		}
		if (repressCheck == nullptr) {
			return iterator[realKey] == tge::io::PressMode::HOLD || iterator[realKey] == tge::io::PressMode::CLICKED;
		}
		if (iterator[realKey] == tge::io::PressMode::RELEASED) {
			*repressCheck = false;
			return false;
		}
		if (*repressCheck) return false;
		*repressCheck = true;
		return true;
	}

	inline bool checkForBinding(const IOFunction function, bool* repressCheck = nullptr) const {
		return checkForBinding(functionBindings[function._to_index()], repressCheck);
	}

	inline bool checkForBinding(const IOFunction function, const RepressChecks check) {
		return checkForBinding(functionBindings[function._to_index()], &repressChecks[check._to_index()]);
	}

	tge::main::Error init() override {
		std::fill(begin(mouseButtonsCache), end(mouseButtonsCache), tge::io::PressMode::RELEASED);
		std::fill(begin(keyboardPressesCache), end(keyboardPressesCache), tge::io::PressMode::RELEASED);
		getImageIDFromBackend();
		return tge::io::IOModule::init();
	}

	void selectInternal();

	void changeCameraModel(CameraModel newModel) {
		if (cameraModel == newModel) return;
		positionVector = glm::vec3{ 0 };
		directionVector = glm::vec4{ 0, 1.0, 0, 0 };
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
				positionVector.z += actualOffset;
			}
			if (checkForBinding(IOFunction::Rotating_Down)) {
				positionVector.z -= actualOffset;
			}
			if (checkForBinding(IOFunction::Rotating_Reset)) {
				positionVector = glm::vec3(0);
			}
			eye = positionVector + glm::vec3(directionVector * scale);
			center = positionVector;
			break;
		case CameraModel::Free_Cam:
			glm::vec3 yDir(glm::normalize(currentVP * glm::vec4(1.0f, 0.0, 0.0, 0.0)) * actualOffset);
			glm::vec3 xDir(-yDir.y, yDir.x, 0.0f);
			scale = 1;
			if (checkForBinding(IOFunction::Free_Forward)) {
				positionVector -= xDir;
			}
			if (checkForBinding(IOFunction::Free_Backwards)) {
				positionVector += xDir;
			}
			if (checkForBinding(IOFunction::Free_Left)) {
				positionVector -= yDir;
			}
			if (checkForBinding(IOFunction::Free_Right)) {
				positionVector += yDir;
			}
			if (checkForBinding(IOFunction::Free_Up)) {
				positionVector.z += actualOffset;
			}
			if (checkForBinding(IOFunction::Free_Down)) {
				positionVector.z -= actualOffset;
			}
			if (checkForBinding(IOFunction::Free_Reset)) {
				positionVector = glm::vec3(0);
			}
			if (checkForBinding(IOFunction::Free_Speed_Add)) {
				speed += SPEED_MULTIPLIER * deltatime;
			}
			if (checkForBinding(IOFunction::Free_Speed_Reduce)) {
				speed -= SPEED_MULTIPLIER * deltatime;
			}
			eye = positionVector;
			center = positionVector + glm::vec3(directionVector * scale);
			break;
		default:
			break;
		}
		speed = glm::clamp(speed, 0.001f, 1000.0f);
		scale = glm::clamp(scale, 0.001f, 1000.0f);
		const auto oldView = glm::lookAt(eye, center, glm::vec3{ 0.0f, 0.0f, -1.0f });
		ggm->updateCameraMatrix(oldView);

		if (checkForBinding(IOFunction::Select, RepressChecks::Select)) {
			auto api = ggm->getAPILayer();
			const auto [imageData, internalDataHolder] =
				api->getImageData(imageID, dataHolder);
			dataHolder = internalDataHolder;
			const auto bounds = api->getRenderExtent();
			const auto dataBuffer = (float*)imageData.data();
			const auto offset = (size_t)(bounds.x * oldInputPosition.y) + (size_t)oldInputPosition.x;
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

		scrollCache = 0;
	}

	void mouseEvent(const tge::io::MouseEvent& event) override {
		using namespace tge::io;
		if (event.pressMode == PressMode::SCROLL) {
			scrollCache += event.y;
		}
		else {
			switch (event.pressMode)
			{
			case tge::io::PressMode::HOLD:
			case tge::io::PressMode::CLICKED:
			case tge::io::PressMode::RELEASED:
				mouseButtonsCache[event.pressed] = event.pressMode;
				break;
			default:
				break;
			}
		}

		constexpr auto MODIFER = 0.001f;
		if (checkForBinding(IOFunction::Move_Camera)) {
			switch (event.pressMode) {
			case PressMode::CLICKED:
				oldInputPosition = glm::vec2(event.x, event.y);
				break;
			case PressMode::HOLD:
				const auto currentVP = glm::inverse(ggm->getVPMatrix());
				glm::vec2 newPos(event.x, event.y);
				auto currentDelta = (newPos - oldInputPosition) * MODIFER;
				oldInputPosition = newPos;
				const auto newRotX = glm::rotate(-currentDelta.x, glm::vec3(currentVP * glm::vec4(0.0, 1.0f, 0.0, 0.0)));
				const auto newRotY = glm::rotate(currentDelta.y, glm::vec3(currentVP * glm::vec4(1.0f, 0.0, 0.0, 0.0)));
				directionVector = glm::normalize(newRotX * directionVector);
				directionVector = glm::normalize(newRotY * directionVector);
				break;
			}
		}
	}

	void keyboardEvent(const tge::io::KeyboardEvent& event) override {
		if (event.signal < 126) {
			if (event.mode == tge::io::PressMode::RELEASED) {
				keyboardPressesCache[event.signal] = tge::io::PressMode::RELEASED;
			}
			else if (keyboardPressesCache[event.signal] == tge::io::PressMode::CLICKED) {
				keyboardPressesCache[event.signal] = tge::io::PressMode::HOLD;
			}
			else if (event.mode == tge::io::PressMode::CLICKED) {
				keyboardPressesCache[event.signal] = tge::io::PressMode::CLICKED;
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
