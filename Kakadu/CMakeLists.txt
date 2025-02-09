
add_library(KakaduReaders STATIC JP2_File_Reader.cc JP2_JPIP_Reader.cc JP2_Box.cc)
set_target_properties(KakaduReaders PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(KakaduReaders PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

#add_library(KakaduReaders SHARED $<TARGET_OBJECTS:objKakaduReaders>)
#add_library(KakaduReaders_static STATIC $<TARGET_OBJECTS:objKakaduReaders>)

#target_include_directories(KakaduReaders_static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

#target_link_libraries(objKakaduReaders JP2_Reader KDU KDU_AUX PIRL::PIRL++ idaeim::PVL)
target_link_libraries(KakaduReaders JP2_Reader KDU PIRL::PIRL++ idaeim::PVL_static)

