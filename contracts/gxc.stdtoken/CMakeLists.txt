add_contract(gxc.stdtoken gxc.stdtoken ${CMAKE_CURRENT_SOURCE_DIR}/src/gxc.stdtoken.cpp)

target_include_directories(gxc.stdtoken
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/../libraries/include
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(gxc.stdtoken
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
