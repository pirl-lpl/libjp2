
add_executable(test_JP2_Reader test_JP2_Reader.cc)
add_executable(test_JPIP_Connect test_JPIP_Connect.cc)
include_directories(${PIRL++_INCLUDE} ${KAKADU_INCLUDE} ${MADMIND_INCLUDE} ".." "../Kakadu")
target_link_libraries(test_JP2_Reader JP2 KDU ${KAKADU} ${KAKADU_AUX} "-L${PIRL_ROOT}/lib" PIRL++ "-L${MADMIND_ROOT}/lib" PVL)
target_link_libraries(test_JPIP_Connect JP2 KDU ${KAKADU} ${KAKADU_AUX} "-L${PIRL_ROOT}/lib" PIRL++ "-L${MADMIND_ROOT}/lib" PVL)
