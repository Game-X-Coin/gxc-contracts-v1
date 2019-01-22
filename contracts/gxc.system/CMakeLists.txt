add_contract(gxc.system gxc.system ${CMAKE_CURRENT_SOURCE_DIR}/src/gxc.system.cpp)

target_include_directories(gxc.system
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/../libraries/include
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(gxc.system
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
