set(KAKADU_VERSION 7A)

set(kdu_lib_path "${Kakadu_Dir}/lib/Linux-x86-64-gcc/" "${Kakadu_Dir}/lib/Mac-x86-64-gcc/" "${Kakadu_Dir}/../lib_x64/" "${Kakadu_Dir}/../lib_x86/")

set(kdu_inc_path "${Kakadu_Dir}/managed/all_includes/")

#message("Looking for Kakadu includes in ${kdu_inc_path}")

find_path(kdu_inc jp2.h PATHS ${kdu_inc_path})

#message("Looking for Kakadu libraries in ${kdu_lib_path}")

if (MSVC)
	set(kdu_lib_name "kdu_v${KAKADU_VERSION}R")
	set(kdu_aux_name "kdu_a${KAKADU_VERSION}R")
else()
	set(kdu_lib_name "kdu")
	set(kdu_aux_name "kdu_aux")
endif()

find_library(kdu_lib NAMES ${kdu_lib_name} PATHS ${kdu_lib_path} NO_DEFAULT_PATH)
find_library(kdu_aux NAMES ${kdu_aux_name} PATHS ${kdu_lib_path} NO_DEFAULT_PATH)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(Kakadu DEFAULT_MSG kdu_lib kdu_aux kdu_inc)

set(KAKADU_LIBRARIES ${kdu_lib} ${kdu_aux})
set(KAKADU_INCLUDE_DIRS ${kdu_inc})

message("Found KDU libs ${KAKADU_LIBRARIES}")