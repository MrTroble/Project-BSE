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
	std::array<bool, 255> stack = { false };
	bool pressedLeft = false;
	bool pressedMiddle = false;
	bool pressedShift = false;
	glm::mat4 oldView = glm::identity<glm::mat4>();
	glm::vec4 oldRotX{};
	glm::vec4 oldRotY{};
	glm::vec4 lookAt{ 0, 1.0, 0, 0 };
	CameraModel cameraModel = CameraModel::Rotating;

	void getImageIDFromBackend();

	tge::main::Error init() override {
		getImageIDFromBackend();
		return tge::io::IOModule::init();
	}

	void selectInternal();

	void changeCameraModel(CameraModel newModel) {
		if (cameraModel == newModel) return;
		if (newModel._to_integral() == CameraModel::Rotating) {
			cache = glm::vec3{ 0 };
			lookAt = glm::vec4{ 0, 1.0, 0, 0 };
		}
		else {
			cache = glm::vec3{ 0 };
			lookAt = glm::vec4{ 0, 1.0, 0, 0 };
		}
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
			if (stack['W']) {
				scale -= actualOffset;
			}
			if (stack['S']) {
				scale += actualOffset;
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
			eye = cache + glm::vec3(lookAt * scale);
			center = cache;
			break;
		case CameraModel::Free_Cam:
			glm::vec3 yDir(glm::normalize(currentVP * glm::vec4(1.0f, 0.0, 0.0, 0.0)) * actualOffset);
			glm::vec3 xDir(-yDir.y, yDir.x, 0.0f);
			scale = 1;
			if (stack['W']) {
				cache -= xDir;
			}
			if (stack['S']) {
				cache += xDir;
			}
			if (stack['A']) {
				cache -= yDir;
			}
			if (stack['D']) {
				cache += yDir;
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
			eye = cache;
			center = cache + glm::vec3(lookAt * scale);
			break;
		default:
			break;
		}
		scale = glm::clamp(scale, 0.001f, 1000.0f);
		oldView = glm::lookAt(eye, center, glm::vec3{ 0.0f, 0.0f, -1.0f });
		ggm->updateCameraMatrix(oldView);

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
	}

	void mouseEvent(const tge::io::MouseEvent& event) override {
		using namespace tge::io;
		if (event.pressMode == PressMode::SCROLL) {
			speed = glm::max(0.05f, glm::min(speed + event.y * 0.2f, 100.0f));
		}
		if (event.pressMode == PressMode::CLICKED) {
			if (!pressedMiddle) vec = glm::vec2(event.x, event.y);
			if (event.pressed == 1) {
				pressedLeft = true;
			}
			else if (event.pressed == 3) {
				pressedMiddle = true;
			}
			if (event.additional & 8) {
				pressedShift = true;
			}
		}
		else if (event.pressMode == PressMode::RELEASED) {
			if (event.pressed == 3) {
				pressedMiddle = false;
			}
		}
		constexpr auto MODIFER = 0.001f;
		if (pressedMiddle) {
			switch (event.pressMode) {
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
		if (event.signal < 255) {
			stack[event.signal] = true;
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
