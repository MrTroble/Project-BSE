#pragma once

#include <imgui.h>

#include <numbers>
#include <algorithm>
#include <graphics/APILayer.hpp>
#include <graphics/GUIModule.hpp>
#include <graphics/GameGraphicsModule.hpp>
#include <graphics/PerformanceTestAPI.hpp>

class TGAppGUI : public tge::gui::GUIModule {
public:
	tge::graphics::Light light;
	tge::graphics::APILayer* api = nullptr;
	tge::graphics::GameGraphicsModule* ggm = nullptr;
	tge::graphics::NodeTransform transformData;
	size_t lightID;
	bool focused = false;
	char search[101] = { 0 };
	std::unordered_set<size_t> extended;

	void renderGUI() override {
		if (ImGui::Begin("test")) {
			if (ImGui::SliderFloat3("Light", (float*)&light.pos, -10.0f, 10.0f) ||  //
				ImGui::SliderFloat("Intesity", (float*)&light.intensity, -10.0f,
					10.0f)  //
				) {
				lightID = api->pushLights(1, &light);
			}
			if (api != nullptr) {
				const auto debug =
					((tge::graphics::PerformanceMessuringAPILayer*)api)->getDebug();
				ImGui::Text("%s", debug.c_str());
			}
#ifdef DEBUG
			if (ggm != nullptr && ImGui::BeginChild("Scene Graph")) {
				std::lock_guard guard(ggm->nodeHolder.mutex);
				auto& marker = std::get<4>(ggm->nodeHolder.internalValues);
				const auto& values = std::get<7>(ggm->nodeHolder.internalValues);
				const auto& children = std::get<6>(ggm->nodeHolder.internalValues);
				const auto& parents = std::get<2>(ggm->nodeHolder.internalValues);
				auto& transform = std::get<1>(ggm->nodeHolder.internalValues);
				if (ImGui::InputText("Search", search, 100)) {
					extended.clear();
					if (search[1] != 0) {
						for (int i = values.size() - 1; i >= 0; i--)
						{
							const auto index = ggm->nodeHolder.translationTable[i];
							const auto parent = parents[i];
							if (extended.contains(index)) {
								if (parent != INVALID_SIZE_T)
									extended.insert(ggm->nodeHolder.translationTable[parent]);
								continue;
							}
							const auto& nameChild = values[index] == nullptr ? "Unknown #" + std::to_string(index) : values[index]->name;
							const auto value = nameChild.find(search);
							const bool found = value < nameChild.size();
							if (found) {
								extended.insert(index);
								extended.insert(ggm->nodeHolder.translationTable[parent]);
							}
						}
					}
				}
				for (size_t i = 0; i < values.size(); i++)
				{
					if (parents[i] != INVALID_SIZE_T) continue;
					const auto index1 = ggm->nodeHolder.translationTable[i];
					const auto& name = values[index1] == nullptr ? "Unknown #" + std::to_string(index1) : values[index1]->name;
					if (extended.contains(index1))
						ImGui::SetNextItemOpen(true);
					if (ImGui::TreeNode(name.c_str())) {
						std::vector<std::vector<size_t>> nextProcess;
						nextProcess.push_back(children[i]);
						while (!nextProcess.empty()) {
							auto& next = nextProcess.back();
							if (next.empty()) {
								nextProcess.pop_back();
								ImGui::TreePop();
								continue;
							}
							const auto child = next.back();
							next.pop_back();
							if (!ggm->nodeHolder.translationTable.contains(child)) continue;
							const auto index = ggm->nodeHolder.translationTable[child];
							const auto& nameChild = values[index] == nullptr ? "Unknown #" + std::to_string(index) : values[index]->name;
							const auto passCheck = extended.contains(index);
							if (!passCheck && search[1] != 0) {
								continue;
							}
							if (ImGui::TreeNode(nameChild.c_str())) {
								nextProcess.push_back(children[index]);
								auto& nodeTrans = transform[index];
								if(ImGui::InputFloat3("Translation", &nodeTrans.translation[0])) marker[index] = 1;
								if(ImGui::InputFloat3("Scale", &nodeTrans.scale[0])) marker[index] = 1;
								auto euler = glm::eulerAngles(nodeTrans.rotation);
								if (ImGui::SliderFloat3("Rotation", &euler[0], 0, 2*std::numbers::pi)) {
									nodeTrans.rotation = glm::quat(euler);
									marker[index] = 1;
								}
							}
						}
					}
				}
				ImGui::EndChild();
			}
#endif // DEBUG
		}
		focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow);
		ImGui::End();
	}

	void recreate() override { tge::gui::GUIModule::recreate(); }
};
