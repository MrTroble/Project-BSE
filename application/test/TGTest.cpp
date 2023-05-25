#define TGE_IMPORT_INTEROP 1
#include <TGEngine.hpp>
#include <thread>

#include "../TGApp.hpp"

using namespace tge::main;
using namespace tge::graphics;

std::vector<std::string> fromKeys;

bool deleteHalf() {
  std::vector<FormKey> keys;
  for (auto& key : fromKeys) {
    if (rand() % 2 == 0) {
      keys.push_back(key.c_str());
    }
  }
  return deleteReferences(keys.size(), keys.data());
}

void test() {
  waitFinishedInit();
  std::vector<ReferenceLoad> loads;
  std::vector<std::string> names(100);
  std::fill(begin(names), end(names), "wrhouse02.nif");
  vec3 translate = {0, 0, 0};
  constexpr auto toMeter = 1.42875f * 100;

  fromKeys.reserve(200);
  size_t current = 0;
  for (const auto& name : names) {
    fromKeys.push_back("testKey" + std::to_string(current));
    ReferenceLoad load;
    load.formKey = fromKeys.back().c_str();
    load.path = name.c_str();
    load.transform = TGE_DEFAULT_TRANSFORM;
    load.transform.translation = translate;
    translate.x += 8 * toMeter;
    if (translate.x >= 8 * toMeter * 10) {
      translate.y += 8 * toMeter;
      translate.x = 0;
    }
    loads.push_back(load);
    current++;
  }

  addLoadFinishedCallback(&deleteHalf);
  const auto ref = loadReferences(loads.size(), loads.data());
  printf("Loaded, %d\n", ref);

  ReferenceLoad loadBSARef;
  loadBSARef.formKey = "testBSAForm";
  loadBSARef.path = "souphot.nif";
  loadBSARef.transform = TGE_DEFAULT_TRANSFORM;
  loadReferences(1, &loadBSARef);
}

int main(int argv, const char** in) {
  std::thread thread(&test);
  thread.detach();
  auto directory = "assets";
  std::vector<char*> bsaHandles{(char*)"ccQDRSSE001-SurvivalMode.bsa"};
  InitConfig config{1, (char*)directory, bsaHandles.size(), bsaHandles.data()};
  return initTGEditor(&config);
}
