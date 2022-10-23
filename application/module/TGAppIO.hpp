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
	glm::vec3 cache{1, 0, 1};
	bool pressedMiddle = false;

	void tick(double deltatime) override
	{
	}

	void mouseEvent(const tge::io::MouseEvent event) override
	{
		if (event.pressed == 16) {
			const glm::vec2 current(event.x, event.y);
			if (pressedMiddle) {
				glm::vec2 delta = vec - current;
				cache.x += delta.x;
				cache.z += delta.y;
				ggm->updateCameraMatrix(glm::lookAt(cache, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)));
			}
			else {
				pressedMiddle = true;
			}
			vec = current;
		}
		else {
			pressedMiddle = false;
		}
	}

	void keyboardEvent(const tge::io::KeyboardEvent event) override
	{
	}

};
