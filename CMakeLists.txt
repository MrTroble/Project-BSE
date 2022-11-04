cmake_minimum_required (VERSION 3.12)

project ("BSE" LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")

add_compile_definitions(APPLICATION_NAME="BSE")

if(WIN32)  
add_compile_options("/D_ITERATOR_DEBUG_LEVEL=0" "/MD" "/EHa")
add_link_options("/ignore:4099")
endif()

add_subdirectory(nifly)
add_subdirectory(TGEngine)

add_library(TGInterOp SHARED "interop/Interop.cpp" "interop/Interop.hpp") 

add_executable(TGApp "application/TGApp.cpp" "application/TGAppDef.cpp"  "application/NifLoader.cpp")
target_link_libraries(TGApp PUBLIC tc::TGEngine nifly TGInterOp)
target_include_directories(TGApp PUBLIC "TGEngine/TGEngine/public" ${NIFLY_INCLUDE_DIR} "$ENV{VULKAN_DIR}")

add_executable(TGAppTest "application/test/TGTest.cpp"  "application/TGAppDef.cpp"  "application/NifLoader.cpp")
target_link_libraries(TGAppTest PUBLIC tc::TGEngine gtest_main nifly)
target_include_directories(TGAppTest PUBLIC "TGEngine/TGEngine/public" ${NIFLY_INCLUDE_DIR})
install(DIRECTORY "application/assets" DESTINATION $<TARGET_FILE_DIR:TGApp>)