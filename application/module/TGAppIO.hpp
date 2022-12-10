#pragma once

#include <IO/IOModule.hpp>
#include <glm/gtx/transform.hpp>
#include <graphics/GameGraphicsModule.hpp>
#include <array>

constexpr float offset = 2.0f;

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
	std::array<bool, 255> stack = { false };
	
	void tick(double deltatime) override
	{
		const auto actualOffset = offset * deltatime;
		if(stack['W']) {
			cache.y += actualOffset;
		}
		if(stack['S']) {
			cache.y -= actualOffset;
		}
		if(stack['A']) {
			cache.x += actualOffset;
		}
		if(stack['D']) {
			cache.x -= actualOffset;
		}
		if(stack['Q']) {
			cache.z += actualOffset;
		}
		if(stack['E']) {
			cache.z -= actualOffset;
		}

		if(stack['R']) {
			cache = glm::vec3(0);
		}

		std::fill(begin(stack), end(stack), false);
		ggm->updateCameraMatrix(glm::lookAt(cache, glm::vec3(0, 1, 0) + cache, glm::vec3(0, 0, -1)));
	}

	void mouseEvent(const tge::io::MouseEvent event) override
	{

	}

	void keyboardEvent(const tge::io::KeyboardEvent event) override
	{
		if(event.signal < 255) {
			stack[event.signal] = true;
		}
	}

	void recreate() override {
		const auto extent = ggm->getAPILayer()->getRenderExtent();
		ggm->updateViewMatrix(glm::perspective(glm::radians(45.0f), extent.x / extent.y, 0.01f, 10000.0f));
	}
};
