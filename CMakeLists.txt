cmake_minimum_required (VERSION 3.12)

project ("BSE" LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")

add_compile_definitions(APPLICATION_NAME="BSE")

if(WIN32)  
add_compile_options("/D_ITERATOR_DEBUG_LEVEL=0" "/MD" "/EHa")
add_link_options("/ignore:4099")
else()
add_link_options("-fPIC")
endif()

add_subdirectory(nifly)
add_subdirectory(TGEngine)

add_library(TGInterOp SHARED "interop/Interop.cpp" "interop/Interop.hpp" "application/TGApp.cpp" "application/TGAppDef.cpp"  "application/module/NifLoader.cpp") 
target_include_directories(TGInterOp PUBLIC "TGEngine/TGEngine/public" ${NIFLY_INCLUDE_DIR} "$ENV{VULKAN_DIR}")
target_link_libraries(TGInterOp PUBLIC tc::TGEngine nifly)

add_executable(TGApp "application/TGAppLauncher.cpp")
target_link_libraries(TGApp PUBLIC TGInterOp)

install(DIRECTORY "application/assets" DESTINATION $<TARGET_FILE_DIR:TGApp>)