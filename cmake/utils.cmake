function(enable_all_warnings target)
  if(MSVC)
    target_compile_options(
      ${target}
      PRIVATE
        /W4 /WX
    )
  else()
    target_compile_options(
      ${target}
      PRIVATE
        -Wall -Werror
    )
  endif()
endfunction()
