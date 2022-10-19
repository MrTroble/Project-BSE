#pragma once

#include "../util/Calculations.hpp"
#include <IO/IOModule.hpp>
#include <glm/gtx/transform.hpp>
#include <graphics/GameGraphicsModule.hpp>

bool isFocused();

class TGAppIO : public tge::io::IOModule
{
public:
	glm::vec3 translation = glm::vec3(0, 0, 0);
	glm::vec3 implTrans = glm::vec3(0, 0, 0);
	glm::vec3 scale = glm::vec3(1, 1, 1);
	glm::mat4 rotation = glm::mat4(1);
	glm::mat4 mvpMatrix = glm::mat4(1);
	glm::mat4 mvp2Matrix = glm::mat4(1);
	glm::mat4 view = glm::mat4(1);
	glm::vec2 total;
	float aspectRatio = 1;
	glm::mat4 projectionMatrix;
	uint32_t binding = UINT32_MAX;
	tge::graphics::APILayer *api;
	glm::vec2 last = glm::vec2(-1, -1);

	bool wState;
	bool sState;
	bool aState;
	bool dState;
	bool qState;
	bool eState;
	int changeY = 0;

	TGAppIO()
	{
		calculateMatrix();
	}

	void calculateMatrix()
	{
		projectionMatrix = glm::perspective(glm::radians(45.0f), aspectRatio, 0.00001f, 10000.0f);
		projectionMatrix[1][1] *= -1;
		view =
			glm::lookAt(glm::vec3(0, 0, 1), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		mvpMatrix = projectionMatrix * view *
					(glm::translate(translation) * rotation * glm::scale(scale) * glm::translate(implTrans));
		mvp2Matrix = projectionMatrix * view *
					 (glm::translate(translation + glm::vec3(0, 0, 0.001)) * rotation * glm::scale(scale) * glm::translate(implTrans));
	}

	void sendChanges()
	{
		this->api->changeData(this->binding, &this->mvpMatrix,
							  sizeof(this->mvpMatrix));
		this->api->changeData(this->binding + 1, &this->mvp2Matrix,
							  sizeof(this->mvp2Matrix));
	}

	void tick(double deltatime) override
	{
		const auto step = deltatime * 10;
		if (sState || wState)
			translation.x += sState ? -step : step;
		if (aState || dState)
			translation.y += aState ? -step : step;
		if (qState || eState)
			translation.z += eState ? -step : step;
		wState = false;
		aState = false;
		sState = false;
		dState = false;
		eState = false;
		qState = false;
		calculateMatrix();
		sendChanges();
		tryUpdateY(deltatime);
	}

	void mouseEvent(const tge::io::MouseEvent event) override
	{
		if (!isFocused())
		{
			if ((event.pressed & 1) == 1)
			{
				total += (glm::vec2(event.x, event.y) - last) * 0.001f;
				total = glm::clamp(total, glm::vec2(-10, -10), glm::vec2(10, 10));
				rotation = glm::mat4(1) * glm::rotate(total.x, glm::vec3(0, 1, 0)) *
						   glm::rotate(total.y, glm::vec3(1, 0, 0));
			}
			else if (event.pressed == tge::io::SCROLL)
			{
				constexpr float WEIGHT = 0.0002f;
				scale += event.x * WEIGHT;
				scale = glm::clamp(scale, glm::vec3(0.01f, 0.01f, 0.01f), glm::vec3(10000, 10000, 10000));
			}
		}
		last = glm::vec2(event.x, event.y);
	}

	void keyboardEvent(const tge::io::KeyboardEvent event) override
	{
		if (event.signal == 'W')
		{
			wState = true;
		}
		if (event.signal == 'A')
		{
			aState = true;
		}
		if (event.signal == 'S')
		{
			sState = true;
		}
		if (event.signal == 'D')
		{
			dState = true;
		}
		if (event.signal == 'Q')
		{
			eState = true;
		}
		if (event.signal == 'E')
		{
			qState = true;
		}
		if (event.signal == 'R')
		{
			changeY = 1;
		}
		if (event.signal == 'F')
		{
			changeY = -1;
		}
	}

	void tryUpdateY(const double delta);
};
