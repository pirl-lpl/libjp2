project(libJP2)

cmake_minimum_required(VERSION 3.12)

set(CXX_STANDARD 17)

if(MSVC)
   add_compile_options(/W4 /WX)
else()
   add_compile_options(-Wall)
endif()

#
# PIRL++ dependency
#
set(PIRL_ROOT $ENV{PIRL_ROOT})
find_library(PIRL++ NAMES "PIRL++" PATHS ${PIRL_ROOT}/lib NO_DEFAULT_PATH)
find_path(PIRL++_INCLUDE NAMES "Dimensions.hh" PATHS ${PIRL_ROOT}/include NO_DEFAULT_PATH)

#
# Kakadu dependency
#
set(KAKADU_ROOT $ENV{KAKADU_ROOT})
find_path(KAKADU_INCLUDE NAMES "jp2.h" PATHS ${KAKADU_ROOT}/managed/all_includes NO_DEFAULT_PATH)
find_library(KAKADU NAMES ${CMAKE_STATIC_LIBRARY_PREFIX}kdu${CMAKE_STATIC_LIBRARY_SUFFIX} PATHS ${KAKADU_ROOT}/lib NO_DEFAULT_PATH)
find_library(KAKADU_AUX NAMES ${CMAKE_STATIC_LIBRARY_PREFIX}kdu_aux${CMAKE_STATIC_LIBRARY_SUFFIX} PATHS ${KAKADU_ROOT}/lib NO_DEFAULT_PATH)

#
# madmind dependency
#
set(MADMIND_ROOT $ENV{IDAEIM_ROOT})
find_library(MADMIND NAMES "PVL" PATHS ${MADMIND_ROOT}/lib NO_DEFAULT_PATH)
find_path(MADMIND_INCLUDE NAMES "PVL.hh" PATHS ${MADMIND_ROOT}/include NO_DEFAULT_PATH)

add_subdirectory(Kakadu)
add_subdirectory(tests)

add_library(JP2 SHARED JP2.cc JP2_Utilities.cc)
add_library(JP2_Reader SHARED JP2_Metadata.cc JP2_Reader.cc JP2_Exception.cc)

include_directories(${CMAKE_SOURCE_DIR} ${PIRL++_INCLUDE} ${KAKADU_INCLUDE} ${MADMIND_INCLUDE})
target_link_libraries(JP2 KDU ${KAKADU} ${KAKADU_AUX})
target_link_libraries(JP2_Reader ${KAKADU} ${KAKADU_AUX})
target_link_libraries(JP2 "-L${PIRL_ROOT}/lib" PIRL++ "-L${MADMIND_ROOT}/lib" PVL)
target_link_libraries(JP2_Reader "-L${PIRL_ROOT}/lib" PIRL++ "-L${MADMIND_ROOT}/lib" PVL String)
