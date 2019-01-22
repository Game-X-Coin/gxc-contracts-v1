add_contract(gxc.systoken gxc.systoken ${CMAKE_CURRENT_SOURCE_DIR}/src/gxc.systoken.cpp)

target_include_directories(gxc.systoken
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/../libraries/include
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(gxc.systoken
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
