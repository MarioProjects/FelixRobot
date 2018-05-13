########### lex library ###############
add_library(lda lda.c)

add_executable(get_lda get_lda.c)
target_link_libraries(get_lda lda utils)
