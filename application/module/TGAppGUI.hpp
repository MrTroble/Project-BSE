#pragma once

#include <imgui.h>

#include <algorithm>
#include <graphics/APILayer.hpp>
#include <graphics/GUIModule.hpp>
#include <graphics/GameGraphicsModule.hpp>
#include <graphics/PerformanceTestAPI.hpp>

#include "TGAppIO.hpp"

class TGAppGUI : public tge::gui::DebugGUIModule {
public:
	tge::graphics::Light light;
	tge::graphics::NodeTransform transformData;
	size_t lightID;
	bool focused = false;

	TGAppGUI(tge::io::IOModule* io) : tge::gui::DebugGUIModule(io) {}

	template<class EnumClass>
	inline void selectorFor(EnumClass* model, const char* name) {
		const auto currentPreset = model->_to_string();
		if (ImGui::BeginCombo(name, currentPreset)) {
			for (const auto currentType : EnumClass::_values())
			{
				const auto name = currentType._to_string();
				const bool isSelected = (currentType == *model);
				if (ImGui::Selectable(name, isSelected)) {
					*model = currentType;
					break;
				}
				if (isSelected) ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
	}

	void renderGUI() override {
		if (ImGui::Begin("test")) {
			if (ImGui::SliderFloat3("Light", (float*)&light.pos, -10.0f, 10.0f) ||  //
				ImGui::SliderFloat("Intesity", (float*)&light.intensity, -10.0f,
					10.0f)  //
				) {
				lightID = api->pushLights(1, &light);
			}
			auto debugAPI =
				dynamic_cast<tge::graphics::PerformanceMessuringAPILayer*>(api);
			if (debugAPI != nullptr) {
				const auto debug = debugAPI->getDebug();
				ImGui::Text("%s", debug.c_str());
			}
		}
		auto appio =
			static_cast<TGAppIO*>(io);

		selectorFor(&appio->cameraModel, "Camera Mode");

		focused = ImGui::IsWindowFocused();
		ImGui::End();
	}
};
