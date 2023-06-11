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

  ReferenceLoad loadBSARef[3];
  loadBSARef[0].formKey = "testBSAForm";
  loadBSARef[0].path = "meshes\\survival\\maginvhungerpenaltyspellart.nif";
  loadBSARef[0].transform = TGE_DEFAULT_TRANSFORM;
  loadBSARef[0].transform.translation.x = -300;
  loadBSARef[0].transform.scale = { 0.3f, 0.3f, 0.3f };

  loadBSARef[1].formKey = "testBSACompressedWithTexturesForm";
  loadBSARef[1].path = "meshes\\architecture\\whiterun\\wrhouse02.nif";
  loadBSARef[1].transform = TGE_DEFAULT_TRANSFORM;
  loadBSARef[1].transform.translation.x = -500;
  loadBSARef[1].transform.scale = { 0.1f, 0.1f, 0.1f };

  loadBSARef[2].formKey = "testProblemNifForm";
  loadBSARef[2].path = "dwerubblecolumn02.nif";
  loadBSARef[2].transform = TGE_DEFAULT_TRANSFORM;
  loadBSARef[2].transform.translation.x = -400;
  loadBSARef[2].transform.scale = { 0.1f, 0.1f, 0.1f };
  loadReferences(3, loadBSARef);
}

int main(int argv, const char** in) {
  std::thread thread(&test);
  thread.detach();
  auto directory = "assets";
  std::vector<char*> bsaHandles{(char*)"ccQDRSSE001-SurvivalMode.bsa",
                                (char*)"Whiterun - Textures.bsa",
                                (char*)"Whiterun.bsa"};
  InitConfig config{1, (char*)directory, bsaHandles.size(), bsaHandles.data()};
  return initTGEditor(&config);
}
