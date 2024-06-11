#define TGE_IMPORT_INTEROP 1
#include <TGEngine.hpp>
#include <thread>

#include "../TGApp.hpp"

using namespace tge::main;
using namespace tge::graphics;

std::vector<std::string> fromKeys;
std::unordered_set<std::string> toDeleteMemTest = {
    "testBSAFormMemTest", "testBSACompressedWithTexturesFormMemTest"};

class TestMod : public tge::main::Module {
public:
    bool start = false;
    double test = 0;
    bool lastStatus = false;

    void tick(double delta) override {
        if (start) {
            if (test >= 10) {
                test = 0;
                FormKey key = "testBSAForm";
                lastStatus != lastStatus;
                hideReferences(1, &key, lastStatus);
            }
            test += delta;
        }
    }
};
TestMod mod;

bool deleteHalf(const uint count, const ReferenceLoad* load) {
  std::vector<FormKey> keys;
  for (auto& key : std::span(load, load + count)) {
    std::string value(key.formKey);
    if (value == "testBSAForm") {
        mod.start = true;
    }
    if (toDeleteMemTest.contains(value)) {
      keys.push_back(key.formKey);
      continue;
    }
    if (value.starts_with("testKey") && rand() % 2 == 0) {
      keys.push_back(key.formKey);
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
    load.transform.rotations.z = translate.x;
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
  loadBSARef[0].transform.scale = {0.3f, 0.3f, 0.3f};

  loadBSARef[1].formKey = "testBSACompressedWithTexturesForm";
  loadBSARef[1].path = "meshes\\architecture\\whiterun\\wrhouse02.nif";
  loadBSARef[1].transform = TGE_DEFAULT_TRANSFORM;
  loadBSARef[1].transform.translation.x = -500;
  loadBSARef[1].transform.scale = {0.1f, 0.1f, 0.1f};

  loadBSARef[2].formKey = "testProblemNifForm";
  loadBSARef[2].path = "dwerubblecolumn02.nif";
  loadBSARef[2].transform = TGE_DEFAULT_TRANSFORM;
  loadBSARef[2].transform.translation.x = -400;
  loadBSARef[2].transform.scale = {0.1f, 0.1f, 0.1f};
  loadReferences(3, loadBSARef);

  ReferenceLoad loadBSARefMemTest[2];
  loadBSARefMemTest[0].formKey = "testBSAFormMemTest";
  loadBSARefMemTest[0].path =
      "meshes\\survival\\maginvhungerpenaltyspellart.nif";
  loadBSARefMemTest[0].transform = TGE_DEFAULT_TRANSFORM;
  loadBSARefMemTest[0].transform.translation.x = -300;
  loadBSARefMemTest[0].transform.scale = {0.3f, 0.3f, 0.3f};

  loadBSARefMemTest[1].formKey = "testBSACompressedWithTexturesFormMemTest";
  loadBSARefMemTest[1].path = "meshes\\architecture\\whiterun\\wrhouse02.nif";
  loadBSARefMemTest[1].transform = TGE_DEFAULT_TRANSFORM;
  loadBSARefMemTest[1].transform.translation.x = -500;
  loadBSARefMemTest[1].transform.scale = {0.1f, 0.1f, 0.1f};
  loadReferences(2, loadBSARefMemTest);

  std::vector<float> buffer;
  buffer.resize(33 * 33 * 7);
  auto position = buffer.begin();
  auto normal = position + 33 * 33;
  auto color = position + 33 * 33 * 4;
  for (size_t x = 0; x < 33; x++) {
    for (size_t y = 0; y < 33; y++) {
      const auto value = x / 66.0f + y / 66.0f;
      *position = value * 20;
      color[0] = value;
      color[1] = 0;
      color[2] = 1 - value;
      normal[0] = 0;
      normal[1] = 1;
      normal[2] = 0;
      normal += 3;
      color += 3;
      position++;
    }
  }
  TerrainInfo info;
  info.x = -5000;
  info.positionBegin = 0;
  info.colorBegin = 33 * 33 * 4;
  info.normalBegin = 33 * 33;
  info.point_size = 33;
  info.cornerSets.TopLeft.BaseLayer.Diffuse =
      "assets\\textures\\bsdevorange.dds";
  info.cornerSets.TopRight.BaseLayer.Diffuse =
      "assets\\textures\\bsdevorange.dds";
  info.cornerSets.BottomRight.BaseLayer.Diffuse =
      "assets\\textures\\bsdevorange.dds";
  info.cornerSets.BottomLeft.BaseLayer.Diffuse =
      "assets\\textures\\Leftlower.png";
  loadTerrain(1, &info, buffer.data());
}

int main(int argv, const char** in) {
  std::thread thread(&test);
  thread.detach();
  auto directory = "assets";
  std::vector<char*> bsaHandles{(char*)"ccQDRSSE001-SurvivalMode.bsa",
                                (char*)"Whiterun - Textures.bsa",
                                (char*)"Whiterun.bsa"};
  std::vector<Module*> additional{ &mod };
  InitConfig config{CURRENT_INIT_VERSION, (char*)directory};
  config.featureSet.mipMapLevels = INVALID_UINT32;
  config.additionalEditor = additional.data();
  config.additionalCount = additional.size();
  return initTGEditor(&config, (const char**)bsaHandles.data(),
                      bsaHandles.size());
}
