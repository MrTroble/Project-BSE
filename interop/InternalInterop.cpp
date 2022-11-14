#include "InternalInterop.hpp"
#include "../application/module/NifLoader.hpp"
#include <unordered_map>
#include <TGEngine.hpp>

namespace tge::interop {

	std::unordered_map<std::string, size_t> REFERENCE_MAP;

	inline glm::vec3 vectors(const vec3& vec3) {
		return glm::vec3(vec3.x, vec3.y, vec3.z);
	}

	inline glm::quat quats(const vec3& vec3) {
		return glm::quat(vectors(vec3));
	}

	inline tge::graphics::NodeTransform transformFromInput(const ReferenceTransform& transform) {
		return { vectors(transform.translation), vectors(transform.scale), quats(transform.rotations) };
	}

	bool load(uint count, ReferenceLoad* loads)
	{
		const auto ggm = tge::main::getGameGraphicsModule();
		for (ReferenceLoad* cLoad = loads; cLoad < loads + count; cLoad++)
		{
			const auto& load = *cLoad;

			const auto newTranform = transformFromInput(load.transform);
			const auto nodeID = tge::nif::nifModule->load(load.path, newTranform);
			if (nodeID == SIZE_MAX)
				return false;
			REFERENCE_MAP[load.formKey] = nodeID;
		}
		return true;
	}

	bool update(uint count, ReferenceUpdate* keys)
	{
		return false;
	}

	bool hide(uint count, FormKey* keys, bool hide)
	{
		return false;
	}

	bool remove(uint count, FormKey* keys)
	{
		return false;
	}
}