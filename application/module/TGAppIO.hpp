#pragma once

#include <IO/IOModule.hpp>
#include <glm/gtx/transform.hpp>
#include <graphics/GameGraphicsModule.hpp>

class TGAppIO : public tge::io::IOModule
{
public:
	tge::graphics::GameGraphicsModule* ggm;

	void tick(double deltatime) override
	{
	}

	void mouseEvent(const tge::io::MouseEvent event) override
	{
		ggm->updateCameraMatrix(glm::lookAt(glm::vec3(event.x / 10.0f, 1, 1), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)));
	}

	void keyboardEvent(const tge::io::KeyboardEvent event) override
	{
	}

};
