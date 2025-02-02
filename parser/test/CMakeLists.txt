include(FetchContent)
include(../../cmake/utils.cmake)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY
    https://github.com/catchorg/Catch2.git
  GIT_TAG
    v3.7.1
)
FetchContent_MakeAvailable(Catch2)

file(GLOB TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

foreach(TEST_FILENAME ${TEST_SOURCES})
  get_filename_component(TEST_NAME ${TEST_FILENAME} NAME_WE)
  add_executable(${TEST_NAME} ${TEST_FILENAME})

  target_include_directories(
    ${TEST_NAME}
    PUBLIC
      ${CMAKE_CURRENT_SOURCE_DIR}/../include
  )
  target_compile_features(
    ${TEST_NAME}
    PUBLIC
      cxx_std_17
  )
  enable_all_warnings(${TEST_NAME})
  target_link_libraries(
    ${TEST_NAME}
    Catch2::Catch2WithMain
    SnekParser
  )
  add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()
