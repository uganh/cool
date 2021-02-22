cmake_minimum_required(VERSION 3.0)

project(COOL LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Generate lexer
find_package(RE2C REQUIRED)
RE2C_TARGET(
  COOL_SCANNER
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cool-lex.re
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cool-lex.cc
  COMPILE_FLAGS "-c"
  )

# Generate parser
find_package(BISON REQUIRED)
BISON_TARGET(
  COOL_PARSER
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cool-parse.yy
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cool-parse.cc
  DEFINES_FILE ${CMAKE_CURRENT_SOURCE_DIR}/src/cool-parse.h
  VERBOSE ${CMAKE_CURRENT_SOURCE_DIR}/src/cool-parse.output
  )

# Executable
add_executable(
  COOLC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cool-lex.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cool-tree.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cool-type.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cool-type.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/logger.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/semant.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strtab.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/symtab.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/utilities.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/utilities.h
  ${RE2C_COOL_SCANNER_OUTPUT_SOURCE}
  ${BISON_COOL_PARSER_OUTPUT_HEADER}
  ${BISON_COOL_PARSER_OUTPUT_SOURCE}
  )
set_target_properties(
  COOLC
  PROPERTIES
  OUTPUT_NAME coolc
  CXX_STANDARD 14
  )