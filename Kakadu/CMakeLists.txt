
add_library(KakaduReaders JP2_File_Reader.cc JP2_JPIP_Reader.cc JP2_Box.cc)
set_target_properties(KakaduReaders PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON POSITION_INDEPENDENT_CODE ON)

target_link_libraries(KakaduReaders JP2_Reader KDU KDU_AUX PIRL::PIRL++ idaeim::PVL)
