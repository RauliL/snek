cmake_minimum_required(VERSION 3.11)
project(snek)

option(
  SNEK_ENABLE_CLI
  "Whether CLI interpreter should be built or not."
  ON
)
option(
  SNEK_ENABLE_DEBUG_TOOLS
  "Compile tools used to debug the Snek parser."
  OFF
)

enable_testing()

add_subdirectory(parser)
add_subdirectory(interpreter)
if(SNEK_ENABLE_CLI)
  add_subdirectory(cli)
endif()
if(SNEK_ENABLE_DEBUG_TOOLS)
  add_subdirectory(debug)
endif()
