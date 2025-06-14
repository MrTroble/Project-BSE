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
	inline void selectorFor(const EnumClass model, const char* name, std::invocable<EnumClass> auto function) {
		const auto currentPreset = model._to_string();
		if (ImGui::BeginCombo(name, currentPreset)) {
			for (const auto currentType : EnumClass::_values())
			{
				const auto name = currentType._to_string();
				const bool isSelected = (currentType == model);
				if (ImGui::Selectable(name, isSelected)) {
					function(currentType);
					break;
				}
				if (isSelected) ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
	}

	void renderGUI() override {
		if (ImGui::Begin("Debug Menue")) {
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

			auto appio =
				static_cast<TGAppIO*>(io);
#if 0
			if (ImGui::Button("Print Metadata Buffer")) {
				const auto [imageData, internalDataHolder] =
					api->getImageData(appio->imageID, appio->dataHolder);
				appio->dataHolder = internalDataHolder;
				const auto bounds = api->getRenderExtent();
				const auto dataBuffer = (int*)imageData.data();
				for (size_t y = 0; y < (size_t)bounds.y; y++)
				{
					for (size_t x = 0; x < (size_t)bounds.x; x++)
					{
						int value = dataBuffer[y * (size_t)bounds.x + x];
						std::cout << (value < 1 ? 0:1);
					}
					std::cout << std::endl;
				}
			}
#endif // 0

			ImGui::Text("Speed: %f", appio->speed);
			selectorFor(appio->cameraModel, "Camera Mode", [=](auto x) { appio->changeCameraModel(x); });

			if (ImGui::CollapsingHeader("Keybindings")) {
				for (const auto function : IOFunction::_values())
				{
					if (appio->cameraModel._to_integral() == CameraModel::Rotating &&
						function._to_index() >= IOFunction::Free_Forward && function._to_index() <= IOFunction::Free_Reset)
						continue;
					if (appio->cameraModel._to_integral() == CameraModel::Free_Cam && function._to_index() < IOFunction::Free_Forward)
						continue;
					const auto binding = functionBindings[function._to_index()];
					std::string outputString = function._to_string();
					std::replace(outputString.begin(), outputString.end(), '_', ' ');
					const auto realValue = std::abs(binding.key);
					switch (binding.type)
					{
					case IOFunctionBindingType::Keyboard: {
						if (std::isalnum(realValue)) {
							ImGui::Text("%s: [%c]", outputString.c_str(), (char)realValue);
						}
						else {
							auto optional = SpecialKeys::_from_integral_nothrow(realValue);
							if (optional) {
								ImGui::Text("%s: [%s]", outputString.c_str(), optional->_to_string());
								break;
							}
							ImGui::Text("%s: [Error!]", outputString.c_str());
						}
						break;
					}
					case IOFunctionBindingType::Mouse: {
						ImGui::Text("%s: [Mouse %d]", outputString.c_str(), realValue);
						break;
					}
					case IOFunctionBindingType::Scroll: {
						const char* upOrDown = binding.key > 0 ? "Up" : "Down";
						ImGui::Text("%s: [Scroll %s]", outputString.c_str(), upOrDown);
						break;
					}
					default:
						ImGui::Text("%s: Unbound", outputString.c_str());
					}
				}
			}
		}

		focused = ImGui::IsWindowFocused();
		ImGui::End();
	}
};
