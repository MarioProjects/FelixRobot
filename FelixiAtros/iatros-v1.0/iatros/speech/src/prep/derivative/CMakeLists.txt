########### lex library ###############
add_library(derivative derivative.c)

add_executable(get_deriv get_deriv.c)
target_link_libraries(get_deriv derivative utils)
