#pragma once

#include <algorithm>
#include <graphics/APILayer.hpp>
#include <graphics/GUIModule.hpp>
#include <imgui.h>

class TGAppGUI : public tge::gui::GUIModule {
public:
	tge::graphics::Light light;
	tge::graphics::APILayer* api;
	size_t lightID;
	bool focused = false;

	void renderGUI() override {
		if (ImGui::Begin("test")) {
			
			if (
				ImGui::SliderFloat3("Light", (float*) & light.pos, -10.0f, 10.0f) || //
				ImGui::SliderFloat("Intesity", (float*)&light.intensity, -10.0f, 10.0f)  //
				) {
				lightID = api->pushLights(1, &light);
			}
		}
		focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow);
		ImGui::End();
	}

	void recreate() override {
		tge::gui::GUIModule::recreate();
	}

};
