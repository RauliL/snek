project(SnekCli)

include(FetchContent)

FetchContent_Declare(
  Linenoise
  GIT_REPOSITORY
    https://github.com/yhirose/cpp-linenoise.git
  GIT_TAG
    5d2183e931eff05bc0d419a5eb84040a1d0af2c1
)
FetchContent_MakeAvailable(Linenoise)

add_executable(
  SnekCli
  ./src/main.cpp
  ./src/repl.cpp
  ./src/utils.cpp
)

target_include_directories(
  SnekCli
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(
  SnekCli
  PUBLIC
    SnekInterpreter
  PRIVATE
    linenoise
)
target_compile_features(
  SnekCli
  PRIVATE
    cxx_std_17
)
if(NOT MSVC)
  target_compile_options(
    SnekCli
    PRIVATE
      -Wall -Werror
  )
endif()

set_target_properties(
  SnekCli
  PROPERTIES
    OUTPUT_NAME snek
)

install(
  TARGETS
    SnekCli
  RUNTIME DESTINATION bin
)
