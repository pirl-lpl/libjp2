project(KDU)

cmake_minimum_required(VERSION 3.12)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CXX_STANDARD 17)

# sources and flags
file(GLOB SOURCES *.cc)

# what to build
add_library(${PROJECT_NAME} SHARED ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC ${PIRL++_INCLUDE} ${KAKADU_INCLUDE} "..")
target_link_libraries(${PROJECT_NAME} JP2_Reader ${KAKADU} ${KAKADU_AUX})
