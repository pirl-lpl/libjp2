cmake_minimum_required(VERSION 3.11)

project(jp2)

set(LibSrc
        "JP2.cc"
        "JP2_Utilities.cc"
)

#set(StaticLib ${PROJECT_NAME}s)
#set(SharedLib ${PROJECT_NAME})

# this is the "object library" target: compiles the sources only once
add_library(ObjectLib OBJECT ${LibSrc})

# shared libraries need PIC
set_property(TARGET ObjectLib PROPERTY POSITION_INDEPENDENT_CODE 1)