#pragma once

#include "NifLoader.hpp"

namespace tge::gizmo {


	constexpr auto TRANSLATE = "translategizmo.nif";

	constexpr std::array DEFAULT_LOADS = { TRANSLATE };

	class GizmoLibrary : public main::Module {
		nif::NifModule* nifModule;

	public:
		std::map<std::string, std::vector<tge::graphics::TNodeHolder>> translation;
		std::map<std::string, std::pair<tge::graphics::TRenderHolder, tge::graphics::TRenderHolder>> renderTranslation;

		GizmoLibrary(nif::NifModule* nifModule) : nifModule(nifModule) {}

		virtual main::Error init() {
			std::vector<nif::LoadNif> loads;
			for (const auto& load : DEFAULT_LOADS) {
				loads.emplace_back(load);
			}
			const auto nodes = nifModule->load(loads.size(), loads.data());
			auto nextLoad = loads.begin();
			for (const auto& node : nodes) {
				translation[nextLoad->file] = node;
				renderTranslation[nextLoad->file] = nifModule->nodeIdToRender[node[0]];
				nextLoad++;
			}
			return main::Error::NONE;
		}
	};

}
