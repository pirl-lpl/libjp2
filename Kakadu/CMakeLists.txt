
add_library(objKakaduReaders OBJECT JP2_File_Reader.cc JP2_JPIP_Reader.cc JP2_Box.cc)
set_target_properties(objKakaduReaders PROPERTIES POSITION_INDEPENDENT_CODE ON)


add_library(KakaduReaders SHARED $<TARGET_OBJECTS:objKakaduReaders>)
add_library(KakaduReaders-static STATIC $<TARGET_OBJECTS:objKakaduReaders>)

target_link_libraries(objKakaduReaders JP2_Reader KDU KDU_AUX PIRL::PIRL++ idaeim::PVL)
target_link_libraries(KakaduReaders JP2_Reader KDU KDU_AUX PIRL::PIRL++ idaeim::PVL)

if (WIN32)
   set_target_properties(objKakaduReaders PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
   set_target_properties(KakaduReaders-static PROPERTIES OUTPUT_NAME libKakaduReaders)

   if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.13")
      target_link_options(KakaduReaders PRIVATE "/MANIFEST:NO")
   else()
      set(CMAKE_SHARED_LINKER_FLAGS "/MANIFEST:NO")
   endif()

   	add_custom_command(TARGET KakaduReaders POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:KakaduReaders-static> KakaduReaders.lib)

else()
   set_target_properties(KakaduReaders-static PROPERTIES OUTPUT_NAME KakaduReaders)
endif()