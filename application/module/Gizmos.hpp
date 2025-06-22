#pragma once
#include <graphics/APILayer.hpp>
#include <graphics/GameGraphicsModule.hpp>

struct GizmoLibrary {
	tge::graphics::TNodeHolder node;
	glm::vec3 basePosition{};
	glm::vec3 position{};

	inline void addPosition(glm::vec3 toAdd, tge::graphics::GameGraphicsModule* ggm) {
		tge::graphics::NodeTransform transform;
		transform.translation = (position += toAdd) + basePosition;
		ggm->updateTransform(node, transform);
	
	}

	inline void resetTo(glm::vec3 basePosition, tge::graphics::GameGraphicsModule* ggm) {
		this->basePosition = basePosition;
		position = glm::vec3{0.0f};
		addPosition(position, ggm);
	}
};
GizmoLibrary loadLibrary(tge::graphics::APILayer* api);

