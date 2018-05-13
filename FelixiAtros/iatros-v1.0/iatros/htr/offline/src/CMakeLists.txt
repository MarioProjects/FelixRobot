configure_file(${CMAKE_SOURCE_DIR}/config.h.cmake ${CMAKE_BINARY_DIR}/config.h)
configure_file(${CMAKE_SOURCE_DIR}/version.h.cmake ${CMAKE_BINARY_DIR}/version.h)


set(iatros_htr_offline_SRCS)

list(APPEND iatros_htr_offline_SRCS 
      data_sets/pgmClass.cc
      data_sets/libpbm.c
      data_sets/libpgm.c
      data_sets/libprep.c
      data_sets/mfSetClass.cc
)

if(HAVE_FFTW3_H)
  list(APPEND iatros_htr_offline_SRCS 
        LineSegmentation/mainSegLines.cc
        LineSegmentation/segLinesClass.cc
  )
endif(HAVE_FFTW3_H)

add_static_and_dynamic_library(iatros-htr-offline ${iatros_htr_offline_SRCS})
static_and_dynamic_link_libraries(iatros-htr-offline m) 
if(HAVE_FFTW3_H)
  static_and_dynamic_link_libraries(iatros-htr-offline fftw3) 
endif(HAVE_FFTW3_H)
export_static_and_dynamic_libraries(iatros-htr-offline-config iatros-htr-offline)

add_executable(iatros-htr-offline-pbmclean pbmclean.cc)
target_link_libraries(iatros-htr-offline-pbmclean iatros-htr-offline)
install(TARGETS iatros-htr-offline-pbmclean DESTINATION bin)

add_executable(iatros-htr-offline-pgmborde pgmborde.c)
target_link_libraries(iatros-htr-offline-pgmborde iatros-htr-offline)
install(TARGETS iatros-htr-offline-pgmborde DESTINATION bin)

add_executable(iatros-htr-offline-pgmmedian pgmmedian.c)
target_link_libraries(iatros-htr-offline-pgmmedian iatros-htr-offline)
install(TARGETS iatros-htr-offline-pgmmedian DESTINATION bin)

add_executable(iatros-htr-offline-pgmnormsize pgmnormsize.c)
target_link_libraries(iatros-htr-offline-pgmnormsize iatros-htr-offline)
install(TARGETS iatros-htr-offline-pgmnormsize DESTINATION bin)

add_executable(iatros-htr-offline-pgmresta pgmresta.cc)
target_link_libraries(iatros-htr-offline-pgmresta iatros-htr-offline)
install(TARGETS iatros-htr-offline-pgmresta DESTINATION bin)

add_executable(iatros-htr-offline-pgmRLSA pgmRLSA.cc)
target_link_libraries(iatros-htr-offline-pgmRLSA iatros-htr-offline)
install(TARGETS iatros-htr-offline-pgmRLSA DESTINATION bin)

add_executable(iatros-htr-offline-pgmskew pgmskew.c)
target_link_libraries(iatros-htr-offline-pgmskew iatros-htr-offline)
install(TARGETS iatros-htr-offline-pgmskew DESTINATION bin)

add_executable(iatros-htr-offline-pgmslant pgmslant.c)
target_link_libraries(iatros-htr-offline-pgmslant iatros-htr-offline)
install(TARGETS iatros-htr-offline-pgmslant DESTINATION bin)

add_executable(iatros-htr-offline-pgmslope pgmslope.c)
target_link_libraries(iatros-htr-offline-pgmslope iatros-htr-offline)
install(TARGETS iatros-htr-offline-pgmslope DESTINATION bin)

add_executable(iatros-htr-offline-pgmsubstract pgmsubstract.cc)
target_link_libraries(iatros-htr-offline-pgmsubstract iatros-htr-offline)
install(TARGETS iatros-htr-offline-pgmsubstract DESTINATION bin)

add_executable(iatros-htr-offline-pgmtextfea pgmtextfea.c)
target_link_libraries(iatros-htr-offline-pgmtextfea iatros-htr-offline)
install(TARGETS iatros-htr-offline-pgmtextfea DESTINATION bin)

