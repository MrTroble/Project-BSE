cmake_minimum_required (VERSION 3.12)

project ("BSE" LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_COMPILER_FORCED TRUE)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")

add_compile_definitions(APPLICATION_NAME="BSE")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
add_compile_definitions(DEBUG=1)
endif()

if(false)
add_compile_options("-fsanitize=address")
add_link_options("-fsanitize=address")
endif()

if(WIN32)  
add_compile_options("/D_ITERATOR_DEBUG_LEVEL=0" "/MD" "/EHa")
add_link_options("/ignore:4099")
else()
add_compile_options("-fPIC")
endif()

add_subdirectory(nifly)
add_subdirectory(TGEngine)

include(FetchContent)
FETCHCONTENT_DECLARE(
  binary_io
  GIT_REPOSITORY https://github.com/Ryan-rsm-McKenzie/binary_io
  GIT_TAG 2.0.5
  FIND_PACKAGE_ARGS NAMES binary_io
)
FETCHCONTENT_DECLARE(
  DirectXTex
  GIT_REPOSITORY https://github.com/microsoft/DirectXTex
  GIT_TAG apr2023
  FIND_PACKAGE_ARGS NAMES DirectXTex directxtex
)
FETCHCONTENT_DECLARE(
  lz4
  GIT_REPOSITORY https://github.com/lz4/lz4/
  GIT_TAG v1.9.4
  SOURCE_SUBDIR "build/cmake"
  FIND_PACKAGE_ARGS NAMES LZ4
)

FETCHCONTENT_DECLARE(
  mmio
  GIT_REPOSITORY https://github.com/Ryan-rsm-McKenzie/mmio
  GIT_TAG 1.1.0
  FIND_PACKAGE_ARGS NAMES mmio
)
FETCHCONTENT_DECLARE(
  zlib
  GIT_REPOSITORY https://github.com/madler/zlib
  GIT_TAG v1.2.13
  FIND_PACKAGE_ARGS NAMES ZLIB
)
FETCHCONTENT_DECLARE(
  bsa
  GIT_REPOSITORY https://github.com/Ryan-rsm-McKenzie/bsa
)
FETCHCONTENT_DECLARE(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.0.1
)
FETCHCONTENT_MAKEAVAILABLE(Catch2 zlib lz4 DirectXTex mmio binary_io)
  install(
    TARGETS lz4_static 
    EXPORT lz4_static
  )
  install(
    TARGETS zlibstatic 
    EXPORT zlibstatic
  )
add_library(LZ4::LZ4 ALIAS lz4_static)
add_library(ZLIB::ZLIB ALIAS zlibstatic)
add_library(Microsoft::DirectXTex ALIAS DirectXTex)
FETCHCONTENT_MAKEAVAILABLE(bsa)

add_library(TGInterOp SHARED "interop/Interop.cpp" "interop/Interop.hpp" "application/TGApp.cpp" "application/TGAppDef.cpp"  "application/module/NifLoader.cpp" "interop/InternalInterop.cpp" "application/module/TGAppIO.cpp") 
target_include_directories(TGInterOp PUBLIC "TGEngine/TGEngine/public" ${NIFLY_INCLUDE_DIR} "$ENV{VULKAN_DIR}")
target_link_libraries(TGInterOp PUBLIC tc::TGEngine nifly)

add_executable(TGApp "application/TGAppLauncher.cpp")
target_link_libraries(TGApp PUBLIC TGInterOp)

add_executable(TGTest "application/test/TGTest.cpp")
target_link_libraries(TGTest PUBLIC TGInterOp)

install(DIRECTORY "application/assets" DESTINATION $<TARGET_FILE_DIR:TGApp>)