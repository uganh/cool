cmake_minimum_required(VERSION 3.0)

project(cool LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(CMAKE_CXX_STANDARD 11)

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
