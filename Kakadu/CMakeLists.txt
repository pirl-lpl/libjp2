
add_library(objKakaduReaders OBJECT JP2_File_Reader.cc JP2_JPIP_Reader.cc JP2_Box.cc)
set_target_properties(objKakaduReaders PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_library(KakaduReaders SHARED $<TARGET_OBJECTS:objKakaduReaders>)
add_library(KakaduReaders_static STATIC $<TARGET_OBJECTS:objKakaduReaders>)

#target_include_directories(KakaduReaders PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR})

target_link_libraries(objKakaduReaders JP2_Reader KDU KDU_AUX PIRL++::PIRL++ idaeim::PVL)
target_link_libraries(KakaduReaders JP2_Reader KDU KDU_AUX PIRL++::PIRL++ idaeim::PVL)

if (WIN32)
   set_target_properties(objKakaduReaders PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)

      target_link_options(KakaduReaders PRIVATE "/MANIFEST:NO")

   	#add_custom_command(TARGET KakaduReaders POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:KakaduReaders-static> KakaduReaders.lib)
endif()
