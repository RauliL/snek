project(SnekCli)

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
)
target_link_libraries(
  SnekCli
  SnekInterpreter
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
