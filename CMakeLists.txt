cmake_minimum_required(VERSION 3.0)

project(cool LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(CMAKE_CXX_STANDARD 11)

if (WIN32)
  cmake_policy(SET CMP0074 NEW)

  set(BISON_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/tools/bison)
  set(RE2C_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/tools/re2c)
endif()

# Generate lexer
find_package(RE2C REQUIRED)
RE2C_TARGET(
  COOL_SCANNER
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cool-lex.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cool-lex.gen.cc
)

# Generate parser
find_package(BISON REQUIRED)
BISON_TARGET(
  COOL_PARSER
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cool-parse.yy
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cool-parse.gen.cc
  DEFINES_FILE ${CMAKE_CURRENT_SOURCE_DIR}/src/cool-parse.gen.h
  VERBOSE ${CMAKE_CURRENT_SOURCE_DIR}/cool-parse.output
)

add_executable(
  coolc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cool-lex.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cool-tree.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cool-tree.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strtab.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strtab.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/symtab.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/utilities.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/utilities.h
  ${RE2C_COOL_SCANNER_OUTPUT_SOURCE}
  ${BISON_COOL_PARSER_OUTPUT_HEADER}
  ${BISON_COOL_PARSER_OUTPUT_SOURCE}
)

# /Zc:__cplusplus is required to make __cplusplus accurate
# /Zc:__cplusplus is available starting with Visual Studio 2017 version 15.7
# (according to https://learn.microsoft.com/en-us/cpp/build/reference/zc-cplusplus)
# That version is equivalent to _MSC_VER==1914
# (according to https://learn.microsoft.com/en-us/cpp/preprocessor/predefined-macros?view=vs-2019)
# CMake's ${MSVC_VERSION} is equivalent to _MSC_VER
# (according to https://cmake.org/cmake/help/latest/variable/MSVC_VERSION.html#variable:MSVC_VERSION)
if ((MSVC) AND (MSVC_VERSION GREATER_EQUAL 1914))
  target_compile_options(coolc PUBLIC "/Zc:__cplusplus")
endif()
