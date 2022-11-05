#define TGE_IMPORT_INTEROP 1
#include "TGApp.hpp"

int main(const int count, const char** strings) {
	return initTGEditor(count, strings);
}