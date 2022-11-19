#define TGE_IMPORT_INTEROP 1
#include <TGEngine.hpp>
#include "../TGApp.hpp"
#include <thread>

using namespace tge::main;
using namespace tge::graphics;

void test() {
	waitFinishedInit();
	ReferenceLoad load;
	load.formKey = (FormKey)"TEST";
	load.path = (char*)"wrhouse02.nif";
	load.transform = TGE_DEFAULT_TRANSFORM;
	const auto ref = loadReferences(1, &load);
	printf("Loaded, %d\n", ref);
}

int main(int argv, const char** in) {
	std::thread thread(&test);
	thread.detach();
	const InitConfig config{ 1, "assets" };
	return initTGEditor(&config);
}
