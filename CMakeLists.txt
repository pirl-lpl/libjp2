project(libJP2)

cmake_minimum_required(VERSION 3.11)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_DEBUG_TARGET_PROPERTIES INCLUDE_DIRECTORIES)
set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)

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

add_library(KDU SHARED IMPORTED)
add_library(KDU_AUX SHARED IMPORTED)

set_target_properties(KDU PROPERTIES IMPORTED_LOCATION ${kdu_lib})
set_target_properties(KDU_AUX PROPERTIES IMPORTED_LOCATION ${kdu_aux})

# TODO it should be possible w/ use of the 'SYSTEM' property to not see warnings from these includes
set_target_properties(KDU PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${KAKADU_INCLUDE_DIRS})
set_target_properties(KDU_AUX PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${KAKADU_INCLUDE_DIRS})


add_library(JP2 STATIC JP2.cc JP2_Utilities.cc)
add_library(JP2_Reader STATIC JP2_Metadata.cc JP2_Reader.cc JP2_Exception.cc)

# TODO FIXME slightly less gross hack needed for MacOS
include_directories(${PROJECT_SOURCE_DIR})

target_link_libraries(JP2 KDU KDU_AUX PIRL::PIRL++)
target_link_libraries(JP2_Reader KDU KDU_AUX idaeim::PVL PIRL::PIRL++)

add_subdirectory(Kakadu)
#add_subdirectory(tests)

set(CMAKE_EXPORT_PACKAGE_REGISTRY ON)
#export(TARGETS JP2 JP2_Reader KakaduImplementation test_JP2_Reader test_JPIP_Connect NAMESPACE UA::HiRISE:: FILE JP2-config.cmake)
#export(PACKAGE JP2)
