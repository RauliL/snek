project(SnekInterpreter)

option(
  SNEK_ENABLE_BOOLEAN_CACHE
  "Whether to enable boolean caching or not."
  ON
)
option(
  SNEK_ENABLE_INT_CACHE
  "Whether to enable integer caching or not."
  ON
)
option(
  SNEK_ENABLE_PROPERTY_CACHE
  "Whether value properties should be cached or not."
  ON
)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/include/snek/interpreter/config.hpp.in
  ${CMAKE_CURRENT_SOURCE_DIR}/include/snek/interpreter/config.hpp
)

include(../cmake/utils.cmake)

add_library(
  SnekInterpreter
  ./src/api.cpp
  ./src/evaluate.cpp
  ./src/execute.cpp
  ./src/module.cpp
  ./src/parameter.cpp
  ./src/prototype/boolean.cpp
  ./src/prototype/float.cpp
  ./src/prototype/function.cpp
  ./src/prototype/int.cpp
  ./src/prototype/list.cpp
  ./src/prototype/object.cpp
  ./src/prototype/number.cpp
  ./src/prototype/record.cpp
  ./src/prototype/string.cpp
  ./src/resolve/field.cpp
  ./src/resolve/parameter.cpp
  ./src/resolve/expression.cpp
  ./src/resolve/statement.cpp
  ./src/resolve/type.cpp
  ./src/runtime.cpp
  ./src/scope.cpp
  ./src/type.cpp
  ./src/value.cpp
  ./src/value/boolean.cpp
  ./src/value/float.cpp
  ./src/value/function.cpp
  ./src/value/int.cpp
  ./src/value/list.cpp
  ./src/value/record.cpp
  ./src/value/string.cpp
)

target_include_directories(
  SnekInterpreter
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_link_libraries(
  SnekInterpreter
  SnekParser
)
target_compile_features(
  SnekInterpreter
  PRIVATE
    cxx_std_17
)
enable_all_warnings(SnekInterpreter)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  install(
    TARGETS
      SnekInterpreter
    LIBRARY
      DESTINATION lib
  )
  install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
  )
endif()
