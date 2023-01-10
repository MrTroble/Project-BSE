#define TGE_IMPORT_INTEROP 1
#include <TGEngine.hpp>
#include <thread>

#include "../TGApp.hpp"

using namespace tge::main;
using namespace tge::graphics;

void test() {
  waitFinishedInit();
  std::vector<ReferenceLoad> loads;
  std::vector<std::string> names(100);
  std::fill(begin(names), end(names), "wrhouse02.nif");
  vec3 translate = {0,0,0};
  constexpr auto toMeter = 1.42875f * 100;
  for (const auto& name : names) {
    ReferenceLoad load;
    load.formKey = (FormKey) "TEST";
    load.path = name.c_str();
    load.transform = TGE_DEFAULT_TRANSFORM;
    load.transform.translation = translate;
    translate.x += 8 * toMeter;
    if (translate.x >= 8 * toMeter * 10) {
      translate.y += 8 * toMeter;
      translate.x = 0;
    }
    loads.push_back(load);
  }
  const auto ref = loadReferences(loads.size(), loads.data());
  printf("Loaded, %d\n", ref);

  for (auto& load : loads) {
    load.transform.translation.z += 6 * toMeter;
  }
  const auto ref2 = loadReferences(loads.size(), loads.data());
  printf("Loaded, %d\n", ref2);
}

int main(int argv, const char** in) {
  std::thread thread(&test);
  thread.detach();
  const InitConfig config{1, "assets"};
  return initTGEditor(&config);
}
