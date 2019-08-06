
add_executable(test_JP2_Reader test_JP2_Reader.cc)
add_executable(test_JPIP_Connect test_JPIP_Connect.cc)

target_link_libraries(test_JP2_Reader KakaduReaders JP2 PIRL::PIRL++ idaeim::PVL)
target_link_libraries(test_JPIP_Connect KakaduReaders JP2 PIRL::PIRL++ idaeim::PVL)
