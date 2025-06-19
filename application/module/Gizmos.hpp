#pragma once
#include <graphics/APILayer.hpp>
#include <graphics/GameGraphicsModule.hpp>

struct GizmoLibrary {
	tge::graphics::TNodeHolder node;
	glm::vec3 position{};

	inline void addPosition(glm::vec3 toAdd, tge::graphics::GameGraphicsModule* ggm) {
		tge::graphics::NodeTransform transform;
		transform.translation = (position += toAdd);
		ggm->updateTransform(node, transform);
	}
};
GizmoLibrary loadLibrary(tge::graphics::APILayer* api);

