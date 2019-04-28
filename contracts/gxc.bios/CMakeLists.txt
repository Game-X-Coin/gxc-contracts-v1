add_contract(gxc.bios gxc.bios ${CMAKE_CURRENT_SOURCE_DIR}/src/gxc.bios.cpp)

target_include_directories(gxc.bios
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/../libraries/include
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(gxc.bios
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
