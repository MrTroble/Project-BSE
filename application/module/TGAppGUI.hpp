#pragma once

#include <algorithm>
#include <graphics/APILayer.hpp>
#include <graphics/GUIModule.hpp>
#include <graphics/GameGraphicsModule.hpp>
#include <imgui.h>

class TGAppGUI : public tge::gui::GUIModule {
public:
	tge::graphics::Light light;
	tge::graphics::APILayer* api;
	tge::graphics::GameGraphicsModule* ggm;
	tge::graphics::NodeTransform transformData;
	size_t nodeID;
	size_t lightID;
	bool focused = false;

	void renderGUI() override {
		if (ImGui::Begin("test")) {
			
			if (
				ImGui::SliderFloat3("Light", (float*) &light.pos, -10.0f, 10.0f) || //
				ImGui::SliderFloat("Intesity", (float*)&light.intensity, -10.0f, 10.0f)  //
				) {
				lightID = api->pushLights(1, &light);
			}
			if (ImGui::SliderFloat3("Transform node 2", (float*)&transformData.translation, -1000.0f, 1000.0f) || //
				ImGui::SliderFloat3("Scale node 2", (float*)&transformData.scale, -10.0f, 10.0f) //
				) {
				ggm->updateTransform(nodeID, transformData);
			}
		}
		focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow);
		ImGui::End();
	}

	void recreate() override {
		tge::gui::GUIModule::recreate();
	}

};
