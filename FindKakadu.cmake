set(KAKADU_VERSION 7A)

set(kdu_lib_path "${KAKADU_ROOT}/lib/Linux-x86-64-gcc/" "${KAKADU_ROOT}/lib/Mac-x86-64-gcc/" "${KAKADU_ROOT}/../lib_x64/" "${KAKADU_ROOT}/../lib_x86/")

set(kdu_inc_path "${KAKADU_ROOT}/managed/all_includes/")

#message("Looking for Kakadu includes in ${kdu_inc_path}")

find_path(kdu_inc jp2.h PATHS ${kdu_inc_path})

#message("Looking for Kakadu libraries in ${kdu_lib_path}")

find_library(kdu_lib NAMES "kdu_v${KAKADU_VERSION}R" PATHS ${kdu_lib_path} NO_DEFAULT_PATH)
find_library(kdu_aux NAMES "kdu_a${KAKADU_VERSION}R" PATHS ${kdu_lib_path} NO_DEFAULT_PATH)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(Kakadu DEFAULT_MSG kdu_lib kdu_inc)

set(KAKADU_LIBRARIES ${kdu_lib} ${kdu_aux})
set(KAKADU_INCLUDE_DIRS ${kdu_inc})