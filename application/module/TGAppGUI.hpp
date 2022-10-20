#pragma once

#include <algorithm>
#include <graphics/APILayer.hpp>
#include <graphics/GUIModule.hpp>
#include <imgui.h>

class TGAppGUI : public tge::gui::GUIModule {
public:
	tge::graphics::Light light;
	bool focused = false;

	void renderGUI() override {
		if (ImGui::Begin("test")) {
		}
		focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow);
		ImGui::End();
	}

	void recreate() override {
		tge::gui::GUIModule::recreate();
	}

};
