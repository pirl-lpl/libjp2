
add_library(KakaduReaders STATIC JP2_File_Reader.cc JP2_JPIP_Reader.cc JP2_Box.cc)

target_link_libraries(KakaduReaders JP2_Reader KDU KDU_AUX PIRL::PIRL++ idaeim::PVL)
