#pragma once

#include <stddef.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>    // glm::extractEulerAngleYXZ
#include <glm/glm.hpp>
#include <graphics/GameGraphicsModule.hpp>

#include "Interop.hpp"

namespace tge::interop {


	inline glm::vec3 vectors(const vec3& vec3) {
		return glm::vec3(vec3.x, vec3.y, vec3.z);
	}

	inline glm::quat quats(const vec3& vec3) { return glm::quat(vectors(vec3)); }

	inline tge::graphics::NodeTransform transformFromInput(
		const ReferenceTransform& transform) {
		return { vectors(transform.translation), vectors(transform.scale),
				quats(transform.rotations) };
	}

	inline ReferenceTransform transformToOutput(
		const tge::graphics::NodeTransform& transform) {
		ReferenceTransform out;
		out.translation = { transform.translation.x, transform.translation.y, transform.translation.z };
		out.scale = { transform.scale.x, transform.scale.y, transform.scale.z };
		vec3 euler{ 0 };
		glm::extractEulerAngleXYZ(glm::mat4_cast(transform.rotation), euler.x, euler.y, euler.z);
		out.rotations = euler;
		return out;
	}


	bool load(const uint count, const ReferenceLoad* load);

	bool update(const uint count, const ReferenceUpdate* keys);

	bool hide(const uint count, const FormKey* keys, bool hide);

	bool remove(const uint count, const FormKey* keys);

	bool select(const uint count, const FormKey* keys);

	bool terrain(const uint count, const TerrainInfo* info, float* buffer);

	bool internalSelect(const size_t count, const size_t* ids);

	bool internalUpdateTransform(std::vector<ReferenceUpdate>&& update, std::span<size_t> ids);

	void* getMainWindowHandle();

}  // namespace tge::interop