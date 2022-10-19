#include "../TGApp.hpp"
#include <gtest/gtest.h>

int main(int argv, char** in) {
	testing::InitGoogleTest(&argv, in);
	return RUN_ALL_TESTS();
}
