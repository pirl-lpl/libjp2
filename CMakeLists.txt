project(libJP2)

cmake_minimum_required(VERSION 3.11)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_DEBUG_TARGET_PROPERTIES INCLUDE_DIRECTORIES)

if(MSVC)
   add_compile_options(/W4) # /WX for warnings as errors
else()
   add_compile_options(-Wall)
endif()

# for FindKakadu.cmake
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR})

find_package(PIRL REQUIRED)
find_package(idaeim REQUIRED)
find_package(Kakadu REQUIRED)

#add_library(JP2 STATIC JP2.cc JP2_Utilities.cc)
add_library(JP2_Reader STATIC JP2_Metadata.cc JP2_Reader.cc JP2_Exception.cc)

#target_link_libraries(JP2 Kakadu PIRL::PIRL++)
target_link_libraries(JP2_Reader idaeim::PVL PIRL::PIRL++)

#target_include_directories(JP2 PRIVATE ${KAKADU_INCLUDE_DIRS})
#target_include_directories(JP2_Reader PRIVATE ${KAKADU_INCLUDE_DIRS})

#include_directories(${CMAKE_SOURCE_DIR} ${PIRL++_INCLUDE} ${KAKADU_INCLUDE} ${MADMIND_INCLUDE})
#target_link_libraries(JP2 KDU ${KAKADU} ${KAKADU_AUX})
#target_link_libraries(JP2_Reader ${KAKADU} ${KAKADU_AUX})


#add_subdirectory(Kakadu)
#add_subdirectory(tests)
