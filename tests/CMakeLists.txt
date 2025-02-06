add_compile_definitions(CORESYS_EXPORTS KDU_AUX_EXPORTS)

add_executable(test_JP2_Reader test_JP2_Reader.cc)
#add_executable(test_JPIP_Connect test_JPIP_Connect.cc)

target_include_directories(test_JP2_Reader PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
#target_include_directories(test_JPIP_Connect PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

set_target_properties(test_JP2_Reader PROPERTIES COMPILE_DEFINITIONS BUILDER_STATIC_DEFINE)
#set_target_properties(test_JPIP_Connect PROPERTIES COMPILE_DEFINITIONS BUILDER_STATIC_DEFINE)

target_compile_definitions(test_JP2_Reader PRIVATE CORESYS_EXPORTS KDU_AUX_EXPORTS)
#target_compile_definitions(test_JPIP_Connect PRIVATE CORESYS_EXPORTS KDU_AUX_EXPORTS)

target_link_libraries(test_JP2_Reader KDU KDU_AUX JP2 PIRL::PIRL++_static)
#target_link_libraries(test_JPIP_Connect KDU KDU_AUX JP2)
