cmake_minimum_required(VERSION 3.30)
project(mira VERSION 0.1.0 LANGUAGES CXX)

option(BUILD_TEST "Build tests" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(YamlCpp REQUIRED IMPORTED_TARGET yaml-cpp>=0.8)
pkg_check_modules(ArgParse REQUIRED IMPORTED_TARGET argparse>=3.0)
pkg_check_modules(Lua REQUIRED IMPORTED_TARGET lua>=5.4)
pkg_check_modules(Rime REQUIRED IMPORTED_TARGET rime>=1.10)

add_subdirectory(include)
add_subdirectory(src)

if("${BUILD_TEST}")
  enable_testing()
  add_subdirectory(tests)
endif()
