#pragma once

#include <IO/IOModule.hpp>
#include <glm/gtx/transform.hpp>
#include <graphics/GameGraphicsModule.hpp>

class TGAppIO : public tge::io::IOModule
{
public:
	tge::graphics::GameGraphicsModule* ggm;
	size_t nodeID;
	tge::graphics::NodeTransform transform;
	glm::vec2 vec;
	glm::vec3 cache{};
	bool pressedMiddle = false;
	float scale = 1;

	void tick(double deltatime) override
	{
	}

	void mouseEvent(const tge::io::MouseEvent event) override
	{
		if (event.pressed == tge::io::SCROLL) {
			scale += event.x / 10.0f;
		}

		if (event.pressed == tge::io::MIDDLE_MOUSE) {
			const glm::vec2 current(event.x, event.y);
			if (pressedMiddle) {
				glm::vec2 delta = (vec - current) * scale;
				cache.x += delta.x;
				cache.y += delta.y;
			}
			else {
				pressedMiddle = true;
			}
			vec = current;
		}
		else {
			pressedMiddle = false;
		}

		ggm->updateCameraMatrix(glm::lookAt(cache, glm::vec3(1, 1, 0) + cache, glm::vec3(0, 0, -1)));
	}

	void keyboardEvent(const tge::io::KeyboardEvent event) override
	{
	}

	void recreate() override {
		const auto extent = ggm->getAPILayer()->getRenderExtent();
		ggm->updateViewMatrix(glm::perspective(glm::radians(45.0f), extent.x / extent.y, 0.01f, 10000.0f));
	}
};
