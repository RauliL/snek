CMAKE_MINIMUM_REQUIRED(VERSION 3.12)

PROJECT(
  Snek
  VERSION 0.1.0
  DESCRIPTION "Interpreter for Python like language."
  HOMEPAGE_URL "https://github.com/RauliL/snek"
  LANGUAGES C CXX
)

OPTION(
  SNEK_ENABLE_TESTS
  "Whether to compile test cases or not."
  ON
)

INCLUDE(CheckIncludeFile)
INCLUDE(CheckFunctionExists)

CHECK_INCLUDE_FILE(unistd.h SNEK_HAS_UNISTD_H)
CHECK_INCLUDE_FILE(sysexits.h SNEK_HAS_SYSEXITS_H)
CHECK_INCLUDE_FILE(sys/types.h SNEK_HAS_SYS_TYPES_H)
CHECK_INCLUDE_FILE(sys/stat.h SNEK_HAS_SYS_STAT_H)
CHECK_FUNCTION_EXISTS(isatty SNEK_HAS_ISATTY)

CONFIGURE_FILE(
  ${CMAKE_CURRENT_SOURCE_DIR}/config.hpp.in
  ${CMAKE_CURRENT_SOURCE_DIR}/include/snek/config.hpp
)

ADD_LIBRARY(
  snek
  STATIC
  src/api.cpp
  src/api/bin.cpp
  src/api/debug.cpp
  src/api/io.cpp
  src/api/list.cpp
  src/api/str.cpp
  src/ast/base.cpp
  src/ast/expr/base.cpp
  src/ast/expr/binary.cpp
  src/ast/expr/bool.cpp
  src/ast/expr/call.cpp
  src/ast/expr/field.cpp
  src/ast/expr/float.cpp
  src/ast/expr/func.cpp
  src/ast/expr/id.cpp
  src/ast/expr/int.cpp
  src/ast/expr/list.cpp
  src/ast/expr/null.cpp
  src/ast/expr/record.cpp
  src/ast/expr/str.cpp
  src/ast/expr/subscript.cpp
  src/ast/expr/unary.cpp
  src/ast/import.cpp
  src/ast/parameter.cpp
  src/ast/record.cpp
  src/ast/stmt/assign.cpp
  src/ast/stmt/base.cpp
  src/ast/stmt/block.cpp
  src/ast/stmt/break.cpp
  src/ast/stmt/continue.cpp
  src/ast/stmt/export.cpp
  src/ast/stmt/expr.cpp
  src/ast/stmt/if.cpp
  src/ast/stmt/import.cpp
  src/ast/stmt/pass.cpp
  src/ast/stmt/return.cpp
  src/ast/stmt/type.cpp
  src/ast/stmt/while.cpp
  src/ast/type/base.cpp
  src/ast/type/builtin.cpp
  src/ast/type/func.cpp
  src/ast/type/list.cpp
  src/ast/type/multiple.cpp
  src/ast/type/named.cpp
  src/ast/type/record.cpp
  src/ast/type/str.cpp
  src/cst.cpp
  src/interpreter.cpp
  src/lexer.cpp
  src/message.cpp
  src/parameter.cpp
  src/parser.cpp
  src/parser/expr.cpp
  src/parser/import.cpp
  src/parser/record.cpp
  src/parser/stmt.cpp
  src/parser/type.cpp
  src/scope.cpp
  src/type/any.cpp
  src/type/base.cpp
  src/type/func.cpp
  src/type/intersection.cpp
  src/type/list.cpp
  src/type/primitive.cpp
  src/type/record.cpp
  src/type/str.cpp
  src/type/tuple.cpp
  src/type/union.cpp
  src/utils.cpp
  src/value/base.cpp
  src/value/bin.cpp
  src/value/bool.cpp
  src/value/float.cpp
  src/value/func.cpp
  src/value/int.cpp
  src/value/list.cpp
  src/value/null.cpp
  src/value/record.cpp
  src/value/str.cpp
)

TARGET_INCLUDE_DIRECTORIES(
  snek
  PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/cget/include>
)

TARGET_COMPILE_FEATURES(
  snek
  PRIVATE
    cxx_std_17
)

ADD_EXECUTABLE(
  snek-cli
  src/main.cpp
  src/repl.cpp
)

TARGET_INCLUDE_DIRECTORIES(
  snek-cli
  PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/cget/include>
)

TARGET_COMPILE_FEATURES(
  snek-cli
  PRIVATE
    cxx_std_17
)

TARGET_LINK_LIBRARIES(
  snek-cli
  PRIVATE
    snek
)

SET_TARGET_PROPERTIES(
  snek-cli
  PROPERTIES
    OUTPUT_NAME snek
)

INSTALL(
  TARGETS
    snek-cli
  RUNTIME DESTINATION
    ${CMAKE_INSTALL_BINDIR}
)

IF(${SNEK_ENABLE_TESTS})
  ENABLE_TESTING()
  ADD_SUBDIRECTORY(tests)
ENDIF()
