add_executable(test_JP2_Reader test_JP2_Reader.cc)
add_executable(test_JPIP_Connect test_JPIP_Connect.cc)

#target_include_directories(test_JP2_Reader PRIVATE "..")
#target_include_directories(test_JPIP_Connect PRIVATE "..")

target_link_libraries(test_JP2_Reader JP2 KDU KDU_AUX PIRL::PIRL++ idaeim::PVL)
target_link_libraries(test_JPIP_Connect JP2 KDU KDU_AUX PIRL::PIRL++ idaeim::PVL)