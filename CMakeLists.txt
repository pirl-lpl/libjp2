project(libJP2)

cmake_minimum_required(VERSION 3.11)

set(CMAKE_CXX_STANDARD 17)

if(MSVC)
   add_compile_options(/W4) # /WX for warnings as errors
else()
   add_compile_options(-Wall)
endif()

# for FindKakadu.cmake
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR})

find_package(PIRL 3.0.0 REQUIRED)
find_package(idaeim 2.3.4 REQUIRED)
find_package(Kakadu REQUIRED)

add_library(KDU STATIC IMPORTED)
add_library(KDU_AUX STATIC IMPORTED)

# TODO it should be possible w/ use of the 'SYSTEM' property to not see warnings from these includes
set_target_properties(KDU PROPERTIES IMPORTED_LOCATION ${kdu_lib} INTERFACE_INCLUDE_DIRECTORIES ${KAKADU_INCLUDE_DIRS})
set_target_properties(KDU_AUX PROPERTIES IMPORTED_LOCATION ${kdu_aux} INTERFACE_INCLUDE_DIRECTORIES ${KAKADU_INCLUDE_DIRS})

add_library(objJP2 OBJECT JP2.cc JP2_Utilities.cc JP2_Exception.cc) #
add_library(objJP2_Reader OBJECT JP2_Metadata.cc JP2_Reader.cc JP2_Exception.cc)

set_target_properties(objJP2 PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(objJP2_Reader PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(objJP2 JP2_Reader KDU KDU_AUX PIRL::PIRL++)
target_link_libraries(objJP2_Reader KDU KDU_AUX idaeim::PVL idaeim::Strings PIRL::PIRL++)

# TODO FIXME needed for MacOS??
include_directories(${PROJECT_SOURCE_DIR})

add_library(JP2 SHARED $<TARGET_OBJECTS:objJP2>)
add_library(JP2-static STATIC $<TARGET_OBJECTS:objJP2>)
add_library(JP2_Reader SHARED $<TARGET_OBJECTS:objJP2_Reader>)
add_library(JP2_Reader-static STATIC $<TARGET_OBJECTS:objJP2_Reader>)

if (WIN32)
   set_target_properties(JP2-static PROPERTIES OUTPUT_NAME libJP2)
   set_target_properties(JP2_Reader-static PROPERTIES OUTPUT_NAME libJP2_Reader)

   set_target_properties(JP2 PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
   set_target_properties(JP2_Reader PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)

   if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.13")
      target_link_options(JP2 PRIVATE "/MANIFEST:NO")
      target_link_options(JP2_Reader PRIVATE "/MANIFEST:NO")
   else()
      set(CMAKE_SHARED_LINKER_FLAGS "/MANIFEST:NO")
   endif()
else()
   set_target_properties(JP2-static PROPERTIES OUTPUT_NAME JP2)
   set_target_properties(JP2_Reader-static PROPERTIES OUTPUT_NAME JP2_Reader)
endif()

target_link_libraries(JP2_Reader idaeim::PVL idaeim::Strings idaeim::Utility PIRL::PIRL++)
target_link_libraries(JP2 JP2_Reader KakaduReaders)

#
# TODO currently we need to link using the static libraries, not the import libraries
if(WIN32)
   	add_custom_command(TARGET JP2 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:JP2-static> JP2.lib)
   	add_custom_command(TARGET JP2_Reader POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:JP2_Reader-static> JP2_Reader.lib)
endif()

#include(GenerateExportHeader)
#generate_export_header(JP2 EXPORT_FILE_NAME "JP2_Export.hh")
#target_include_directories(JP2 PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
#target_include_directories(JP2_Reader PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
add_subdirectory(Kakadu)

# TODO currently unable to build these on windows, because the dependency includes "KDU-NOTFOUND" (???)
#add_subdirectory(tests)

set(CMAKE_EXPORT_PACKAGE_REGISTRY ON)
export(TARGETS JP2 JP2_Reader JP2-static JP2_Reader-static KakaduReaders 
	NAMESPACE UA::HiRISE:: FILE JP2-config.cmake) #test_JP2_Reader test_JPIP_Connect
export(PACKAGE JP2)
