
add_library(KakaduReaders SHARED JP2_File_Reader.cc JP2_JPIP_Reader.cc JP2_Box.cc)
set_target_properties(KakaduReaders PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(KakaduReaders JP2_Reader KDU KDU_AUX PIRL::PIRL++ idaeim::PVL)

if (WIN32)
   set_target_properties(KakaduReaders PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)

   if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.13")
      target_link_options(KakaduReaders PRIVATE "/MANIFEST:NO")
   else()
      set(CMAKE_SHARED_LINKER_FLAGS "/MANIFEST:NO")
   endif()

endif()