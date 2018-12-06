project(libJP2)

cmake_minimum_required(VERSION 3.12)

function(assert_found)
foreach(_id IN LISTS ARGN)
   if(NOT ${_id})
      message(FATAL_ERROR "${_id} was not found")
   endif()
endforeach()
endfunction()

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_RULE_MESSAGES OFF)
set(CXX_STANDARD 17)

if(MSVC)
   add_compile_options(/W4 /WX)
else()
   add_compile_options(-Wall)
endif()

if(APPLE)
   set(DEPS "/HiRISE/Users/guym/Deps/MacOS")
elseif(UNIX)
   set(DEPS "/HiRISE/Users/guym/Deps/Linux")
elseif(MSVC)
   set(DEPS "C:\Users\guyma")
else()
   message("Unconfigured OS setup ${CMAKE_SYSTEM_NAME}")
endif()

set(PIRL++-VERSION 2.3.3)
find_library(PIRL++ NAMES "PIRL++" PATHS "${DEPS}/pirl++-${PIRL++-VERSION}" NO_DEFAULT_PATH)
find_path(PIRL++_INCLUDE NAMES "Dimensions.hh" PATHS "${DEPS}/pirl++-${PIRL++-VERSION}" NO_DEFAULT_PATH)

set(KAKADU_VERSION 7.A.5)
set(KAKADU_ROOT "${DEPS}/kakadu-${KAKADU_VERSION}")

find_path(KAKADU_INCLUDE NAMES "jp2.h" PATHS ${KAKADU_ROOT}/managed/all_includes NO_DEFAULT_PATH)

find_library(KAKADU NAMES ${CMAKE_STATIC_LIBRARY_PREFIX}kdu${CMAKE_STATIC_LIBRARY_SUFFIX} PATHS ${KAKADU_ROOT}/lib NO_DEFAULT_PATH)
find_library(KAKADU_AUX NAMES ${CMAKE_STATIC_LIBRARY_PREFIX}kdu_aux${CMAKE_STATIC_LIBRARY_SUFFIX} PATHS ${KAKADU_ROOT}/lib NO_DEFAULT_PATH)

assert_found(KAKADU KAKADU_AUX PIRL++ PIRL++_INCLUDE KAKADU_INCLUDE)

add_subdirectory(Kakadu)

add_library(JP2 JP2.cc JP2_Utilities.cc)
add_library(JP2_Reader JP2_Metadata.cc JP2_Reader.cc JP2_Exception.cc)
#set_property(TARGET JP2 PROPERTY CXX_STANDARD 17)

include_directories(${CMAKE_SOURCE_DIR} ${PIRL++_INCLUDE} ${KAKADU_INCLUDE})
#link_directories(Kakadu)
#target_include_directories(JP2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(JP2 KDU PIRL++ ${KAKADU} ${KAKADU_AUX})
target_link_libraries(JP2_Reader PIRL++ ${KAKADU} ${KAKADU_AUX})
