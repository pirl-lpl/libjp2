
add_library(KDU STATIC JP2_File_Reader.cc JP2_JPIP_Reader.cc JP2_Box.cc)
target_include_directories(KDU PUBLIC ${PIRL++_INCLUDE} ${KAKADU_INCLUDE} ${MADMIND_INCLUDE} "..")
target_link_libraries(KDU JP2_Reader ${KAKADU} ${KAKADU_AUX} "-L${PIRL_ROOT}/lib" PIRL++ "-L${MADMIND_ROOT}/lib" PVL String)
