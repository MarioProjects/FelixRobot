configure_file(${CMAKE_SOURCE_DIR}/config.h.cmake ${CMAKE_BINARY_DIR}/config.h)
configure_file(${CMAKE_SOURCE_DIR}/version.h.cmake ${CMAKE_BINARY_DIR}/version.h)


set(iatros_htr_online_SRCS)

list(APPEND iatros_htr_online_SRCS features.cc online.cc read.cc)

add_static_and_dynamic_library(iatros-htr-online ${iatros_htr_online_SRCS})
static_and_dynamic_link_libraries(iatros-htr-online m fftw3) 
export_static_and_dynamic_libraries(iatros-htr-online-config iatros-htr-online)

add_executable(iatros-htr-online-extract extract.cc)
target_link_libraries(iatros-htr-online-extract iatros-htr-online)
install(TARGETS iatros-htr-online-extract DESTINATION bin)

add_executable(iatros-htr-online-segment segment.cc)
target_link_libraries(iatros-htr-online-segment iatros-htr-online)
install(TARGETS iatros-htr-online-segment DESTINATION bin)
