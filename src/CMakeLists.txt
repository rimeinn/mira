set(SRCS rime.cpp)

macro(add_mira_target TARGET)
  add_library(${TARGET} OBJECT ${SRCS})
  target_include_directories(${TARGET} PUBLIC
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_BINARY_DIR}/include")
  target_link_libraries(${TARGET} PUBLIC
    PkgConfig::ArgParse
    PkgConfig::YamlCpp
    PkgConfig::Lua
    PkgConfig::Rime)
endmacro()

add_mira_target(mira.o)

if ("${BUILD_TEST}")
  add_mira_target(mira_testing.o)
  target_compile_definitions(mira_testing.o PRIVATE -DTESTING=1)
endif()

add_executable(mira main.cpp)
target_link_libraries(mira PRIVATE mira.o)
