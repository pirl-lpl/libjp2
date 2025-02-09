
add_library(KakaduReaders JP2_File_Reader.cc JP2_JPIP_Reader.cc JP2_Box.cc)
set_target_properties(KakaduReaders PROPERTIES POSITION_INDEPENDENT_CODE ${BUILD_SHARED_LIBS})

target_include_directories(KakaduReaders PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

if (WIN32)
    set_target_properties(KakaduReaders PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

target_link_libraries(KakaduReaders JP2_Reader KDU PIRL::PIRL++ idaeim::PVL_static)

