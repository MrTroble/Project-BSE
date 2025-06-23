#pragma once
#include <graphics/APILayer.hpp>
#include <graphics/GameGraphicsModule.hpp>

struct GizmoLibrary {
	tge::graphics::TNodeHolder node;
	glm::vec3 basePosition{};
	glm::vec3 position{};
	glm::vec3 baseScale{};

	inline void addPosition(glm::vec3 toAdd) {
		position += toAdd;
	}

	inline void setBasescale(float scale) {
		baseScale = glm::vec3(scale);
	}

	inline void update(tge::graphics::GameGraphicsModule* ggm) {
		tge::graphics::NodeTransform transform;
		transform.translation = position + basePosition;
		transform.scale = baseScale;
		ggm->updateTransform(node, transform);
	}

	inline void resetTo(glm::vec3 basePosition) {
		this->basePosition = basePosition;
		position = glm::vec3{0.0f};
	}
};
GizmoLibrary loadLibrary(tge::graphics::APILayer* api);

