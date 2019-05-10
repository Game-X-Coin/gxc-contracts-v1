add_contract(gxc.htlc gxc.htlc ${CMAKE_CURRENT_SOURCE_DIR}/src/gxc.htlc.cpp)

target_include_directories(gxc.htlc
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/../libraries/include
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(gxc.htlc
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
