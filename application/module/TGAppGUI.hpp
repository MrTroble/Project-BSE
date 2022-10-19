#pragma once

#include <algorithm>
#include <graphics/GUIModule.hpp>
#include <imgui.h>

constexpr auto minY = -10.0f;
constexpr auto maxY = 10.0f;

constexpr auto minInterpolate = 1;
constexpr auto maxInterpolate = 10;

constexpr auto minIntensity = 0.1f;
constexpr auto maxIntensity = 10.0f;

class TGAppGUI : public tge::gui::GUIModule {
public:
	float currentY = 0.5;
	int interpolation = 4;
	bool focused = false;
	bool doubleSided = false;
	bool wireFrame = false;

	void renderGUI() override {
		if (ImGui::Begin("test")) {
			ImGui::SliderFloat("Y", &currentY, minY, maxY);
			ImGui::SliderInt("Interpolation", &interpolation, minInterpolate,
				maxInterpolate);
			
			if (ImGui::CollapsingHeader("Light")) {
				ImGui::SliderFloat3("Light Position", (float*)&light.pos, minLightpos.x, maxLightpos.x);
				ImGui::SliderFloat("Light Intensity", (float*)&light.intensity, minIntensity, maxIntensity);
				ImGui::ColorPicker3("Light Color", (float*)&light.color);
			}

			if (ImGui::CollapsingHeader("Render")) {
				if(ImGui::Checkbox("Double sided", &this->doubleSided))
					CellEntry::updatePipelines = true;
				if(ImGui::Checkbox("Wire frame", &this->wireFrame))
					CellEntry::updatePipelines = true;
			}

			if (ImGui::Button("Apply")) {
				currentY = std::clamp(currentY, minY, maxY);
				interpolation = std::clamp(interpolation, minInterpolate, maxInterpolate);
				light.pos = glm::clamp(light.pos, minLightpos, maxLightpos);
				light.color = glm::clamp(light.color, minColor, maxColor);
				light.intensity = std::clamp(light.intensity, minIntensity, maxIntensity);
				makeVulkan();
			}
		}
		focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow);
		ImGui::End();
	}

	void recreate() override {
		tge::gui::GUIModule::recreate();
		makeVulkan();
	}

};
