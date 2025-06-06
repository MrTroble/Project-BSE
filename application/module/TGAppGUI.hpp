#pragma once

#include <imgui.h>

#include <algorithm>
#include <graphics/APILayer.hpp>
#include <graphics/GUIModule.hpp>
#include <graphics/GameGraphicsModule.hpp>
#include <graphics/PerformanceTestAPI.hpp>

class TGAppGUI : public tge::gui::DebugGUIModule {
 public:
  tge::graphics::Light light;
  tge::graphics::NodeTransform transformData;
  size_t lightID;
  bool focused = false;

  TGAppGUI(tge::io::IOModule* io) : tge::gui::DebugGUIModule(io) {}

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
    focused = ImGui::IsWindowFocused();
    ImGui::End();
  }
};
