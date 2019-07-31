find_package(PIRL REQUIRED)
find_package(idaeim REQUIRED)
find_package(Kakadu REQUIRED)

add_library(KDU STATIC JP2_File_Reader.cc JP2_JPIP_Reader.cc JP2_Box.cc)
target_include_directories(KDU PUBLIC "..")
target_include_directories(KDU PUBLIC ${KAKADU_INCLUDE_DIRS})
target_link_libraries(KDU JP2_Reader Kakadu PIRL::PIRL++ idaeim::PVL idaeim::Strings)
