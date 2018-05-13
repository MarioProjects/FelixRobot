########### test #########
add_executable(iatros-speech-online online.c)
target_link_libraries(iatros-speech-online iatros-speech)

install(TARGETS iatros-speech-online RUNTIME DESTINATION bin)

