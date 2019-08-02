
add_library(KakaduImplementation STATIC JP2_File_Reader.cc JP2_JPIP_Reader.cc JP2_Box.cc)
#target_include_directories(KakaduImplementation PUBLIC ${PROJECT_SOURCE_DIR})

target_link_libraries(KakaduImplementation JP2_Reader KDU KDU_AUX PIRL::PIRL++ idaeim::PVL)
