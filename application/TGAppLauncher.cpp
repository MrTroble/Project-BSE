#define TGE_IMPORT_INTEROP 1
#include "TGApp.hpp"

int main(const int count, const char** strings) {
	const InitConfig config{ 5, (char*)"assets\\" };
	return initTGEditor(&config, nullptr, 0);
}