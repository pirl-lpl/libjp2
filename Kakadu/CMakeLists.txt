project(KDU)

cmake_minimum_required(VERSION 3.12)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CXX_STANDARD 17)

# sources and flags
file(GLOB SOURCES *.cc)

# what to build
add_library(${PROJECT_NAME} SHARED ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC ${PIRL++_INCLUDE} ${KAKADU_INCLUDE} ${MADMIND_INCLUDE} "..")
#link_directories(${PIRL_ROOT}/lib ${MADMIND_ROOT}/lib)
target_link_libraries(${PROJECT_NAME} JP2_Reader ${KAKADU} ${KAKADU_AUX} "-L${PIRL_ROOT}/lib" PIRL++ "-L${MADMIND_ROOT}/lib" PVL String)
