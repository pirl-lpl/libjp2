cmake_minimum_required(VERSION 3.13)

project(libJP2 VERSION 2.0.0 DESCRIPTION "Provides C++ JP2 interface with Kakadu implementation")

# windows example from build dir
# cmake --fresh -S ..\..\libjp2 -B . -DCMAKE_PREFIX_PATH=C:\Users\deser -DKAKADU_DIR=C:\Users\deser\hiview3\v8_4-00307L
# cmake --build . --config release
set(CMAKE_CXX_STANDARD 17)

# or set CMAKE_COMPILE_WARNING_AS_ERROR true
if(MSVC)
    add_compile_options(/W0) # /WX for warnings as errors
else()
    add_compile_options(-Wall) # -Werror for warnings as errors
endif()

# TODO this should come from the pirl++ import, i.e. ${CMAKE_INSTALL_INCLUDEDIR}
if(APPLE)
    include_directories(/usr/local/include ${PROJECT_SOURCE_DIR})
endif()

# for FindKakadu.cmake
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR})

if(WIN32)
    set(CMAKE_FIND_USE_SYSTEM_PACKAGE_REGISTRY FALSE)
    set(CMAKE_FIND_USE_PACKAGE_REGISTRY FALSE)
    set(CMAKE_FIND_USE_CMAKE_SYSTEM_PATH FALSE)
endif()

# TODO should be able to use <PackageName>_ROOT variables automatically
find_package(PIRL 3.0.0 REQUIRED)
find_package(idaeim 2.3.4 REQUIRED)
find_package(Kakadu REQUIRED)

add_library(KDU STATIC IMPORTED)
add_library(KDU_AUX STATIC IMPORTED)

# TODO it should be possible w/ use of the 'SYSTEM' property to not see warnings from these includes
set_target_properties(KDU PROPERTIES IMPORTED_LOCATION ${kdu_lib} INTERFACE_INCLUDE_DIRECTORIES ${KAKADU_INCLUDE_DIRS})
set_target_properties(KDU_AUX PROPERTIES IMPORTED_LOCATION ${kdu_aux} INTERFACE_INCLUDE_DIRECTORIES ${KAKADU_INCLUDE_DIRS})

add_library(JP2 STATIC JP2.cc JP2_Utilities.cc JP2_Exception.cc) #
add_library(JP2_Reader STATIC JP2_Metadata.cc JP2_Reader.cc JP2_Exception.cc)

set_target_properties(JP2 PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(JP2_Reader PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(JP2 JP2_Reader KDU PIRL::PIRL++)
target_link_libraries(JP2_Reader KDU KakaduReaders idaeim::PVL_static idaeim::Strings_static idaeim::Utility_static PIRL::PIRL++)

#if (WIN32)
#    set_target_properties(JP2 PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
#    set_target_properties(JP2_Reader PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)

#    target_link_options(JP2 PRIVATE "/MANIFEST:NO")
#    target_link_options(JP2_Reader PRIVATE "/MANIFEST:NO")
#endif()

add_subdirectory(Kakadu)
#add_subdirectory(tests)

add_executable(test_kdu_system tests/test_kdu_system.cc)
#add_executable(test_JPIP_Connect tests/test_JPIP_Connect.cc)
add_executable(test_jp2_system Kakadu/test_jp2_system.cc)

set_target_properties(test_kdu_system test_jp2_system PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

#set_target_properties(test_JPIP_Connect PROPERTIES COMPILE_DEFINITIONS BUILDER_STATIC_DEFINE)
#target_compile_definitions(test_jp2_system PRIVATE CORESYS_IMPORTS KDU_AUX_IMPORTS)
#target_compile_definitions(test_JPIP_Connect PRIVATE CORESYS_IMPORTS KDU_AUX_IMPORTS)

target_include_directories(test_kdu_system PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
#arget_include_directories(test_JPIP_Connect PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(test_jp2_system PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(test_kdu_system KDU)
#target_link_libraries(test_JPIP_Connect KDU JP2 JP2_Reader KakaduReaders PIRL::PIRL++)
target_link_libraries(test_jp2_system KakaduReaders KDU KDU_AUX)

set(CMAKE_EXPORT_PACKAGE_REGISTRY ON)
export(TARGETS JP2 JP2_Reader KakaduReaders NAMESPACE ${PROJECT_NAME}:: FILE "${PROJECT_NAME}-exports.cmake")
export(PACKAGE ${PROJECT_NAME})

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

write_basic_package_version_file("${PROJECT_NAME}-config-version.cmake" VERSION ${PROJECT_VERSION}
   COMPATIBILITY SameMajorVersion)

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
  INSTALL_DESTINATION "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/cmake")

install(TARGETS JP2 JP2_Reader KakaduReaders EXPORT ${PROJECT_NAME}-export
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT ${PROJECT_NAME}-export FILE "${PROJECT_NAME}-exports.cmake"
        NAMESPACE ${PROJECT_NAME}:: DESTINATION "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/cmake")

# install config files
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
              "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/cmake")

#include(GenerateExportHeader)
#generate_export_header(JP2 EXPORT_FILE_NAME "JP2_Export.hh")
#target_include_directories(JP2 PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
#target_include_directories(JP2_Reader PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
#

# TODO build and install tests

#set(CMAKE_EXPORT_PACKAGE_REGISTRY ON)
#export(TARGETS JP2 JP2_Reader JP2_static JP2_Reader_static KakaduReaders KakaduReaders_static
# NAMESPACE UA::HiRISE:: FILE JP2-config.cmake) #test_JP2_Reader test_JPIP_Connect
#export(PACKAGE JP2)
