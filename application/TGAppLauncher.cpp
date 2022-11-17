#define TGE_IMPORT_INTEROP 1
#include "TGApp.hpp"

int main(const int count, const char** strings) {
	const InitConfig config{ 1, "assets\\" };
	return initTGEditor(&config);
}