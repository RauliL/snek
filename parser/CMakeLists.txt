project(SnekParser)

include(FetchContent)

FetchContent_Declare(
  PeeloUnicode
  GIT_REPOSITORY
    https://github.com/peelonet/peelo-unicode.git
  GIT_TAG
    v2.1.0
)
FetchContent_MakeAvailable(PeeloUnicode)

add_library(
  SnekParser
  ./src/element.cpp
  ./src/error.cpp
  ./src/expression.cpp
  ./src/field.cpp
  ./src/lexer.cpp
  ./src/parameter.cpp
  ./src/position.cpp
  ./src/statement.cpp
  ./src/token.cpp
  ./src/type.cpp
  ./src/utils.cpp
)

target_include_directories(
  SnekParser
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_link_libraries(
  SnekParser
  PeeloUnicode
)
target_compile_features(
  SnekParser
  PRIVATE
    cxx_std_17
)
if(MSVC)
  target_compile_options(
    SnekParser
    PRIVATE
      /W4 /WX
  )
else()
  target_compile_options(
    SnekParser
    PRIVATE
      -Wall -Werror
  )
endif()

add_subdirectory(test)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  install(
    TARGETS
      SnekParser
    LIBRARY
      DESTINATION lib
  )
  install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
  )
endif()
