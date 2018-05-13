# Install include files
install_headers(iatros-speech 
  ${CMAKE_SOURCE_DIR}/config.h.cmake
  ${CMAKE_SOURCE_DIR}/version.h.cmake
  acquisitor.h
  derivative/derivative.h
  libAudio.h
)


# Install include files
install(FILES 
  DESTINATION include/iatros-speech)

set(iatros_speech_SRCS)

list(APPEND iatros_speech_SRCS circularBuffer/circularBuffer.c)
list(APPEND iatros_speech_SRCS derivative/derivative.c)
list(APPEND iatros_speech_SRCS lda/lda.c)
list(APPEND iatros_speech_SRCS libAudio.c acquisitor.c)

add_static_and_dynamic_library(iatros-speech ${iatros_speech_SRCS})
static_and_dynamic_link_libraries(iatros-speech m fftw3 pthread asound iatros) 
export_static_and_dynamic_libraries(iatros-speech-config iatros-speech)

add_executable(iatros-speech-derivative derivative/get_deriv.c)
target_link_libraries(iatros-speech-derivative iatros-speech)
install(TARGETS iatros-speech-derivative DESTINATION bin)

add_executable(iatros-speech-cepstral extractCC/extractCC.c)
target_link_libraries(iatros-speech-cepstral iatros-speech)
install(TARGETS iatros-speech-cepstral DESTINATION bin)

add_executable(iatros-speech-record record/record.c)
target_link_libraries(iatros-speech-record iatros-speech)
install(TARGETS iatros-speech-record DESTINATION bin)

add_executable(iatros-speech-lda lda/get_lda.c)
target_link_libraries(iatros-speech-lda iatros-speech)
install(TARGETS iatros-speech-lda DESTINATION bin)

add_executable(iatros-speech-emisison-probs emission/get_emission_probs.c)
target_link_libraries(iatros-speech-emisison-probs iatros-speech)
install(TARGETS iatros-speech-emisison-probs DESTINATION bin)

add_executable(iatros-speech-preprocess preprocess.c)
target_link_libraries(iatros-speech-preprocess iatros-speech)
install(TARGETS iatros-speech-preprocess DESTINATION bin)
