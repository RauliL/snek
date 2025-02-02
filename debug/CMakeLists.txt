project(SnekDebugTools)

include(../cmake/utils.cmake)

add_executable(
  snek-lexer-debugger
  ./src/lexer.cpp
  ./src/utils.cpp
)
add_executable(
  snek-parser-debugger
  ./src/parser.cpp
  ./src/utils.cpp
)

foreach(Executable snek-lexer-debugger snek-parser-debugger)
  target_link_libraries(
    ${Executable}
    SnekParser
  )
  target_compile_features(
    ${Executable}
    PRIVATE
      cxx_std_17
  )
  enable_all_warnings(${Executable})
endforeach()

