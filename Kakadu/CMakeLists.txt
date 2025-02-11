
add_library(KakaduReaders JP2_File_Reader.cc JP2_JPIP_Reader.cc JP2_Box.cc)

set(headers JP2_File_Reader.hh JP2_JPIP_Reader.hh JP2_Box.hh)
set_target_properties(KakaduReaders PROPERTIES POSITION_INDEPENDENT_CODE ${BUILD_SHARED_LIBS})

set_target_properties(KakaduReaders PROPERTIES PUBLIC_HEADER "${headers}")
set_target_properties(KakaduReaders PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})

target_include_directories(KakaduReaders PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

if (WIN32)
    set_target_properties(KakaduReaders PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

target_link_libraries(KakaduReaders JP2_Reader KDU PIRL::PIRL++ idaeim::PVL_static)

export(TARGETS KakaduReaders NAMESPACE ${PROJECT_NAME}:: FILE "Kakadu-config.cmake")

include(GNUInstallDirs)
install(TARGETS KakaduReaders
    EXPORT ${PROJECT_NAME}-export
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
