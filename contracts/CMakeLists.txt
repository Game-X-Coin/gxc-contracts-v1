cmake_minimum_required( VERSION 3.5 )

project(contracts)

set(EOSIO_WASM_OLD_BEHAVIOR "Off")
find_package(eosio.cdt)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D${TARGET_NETWORK}")

add_subdirectory(libraries)
link_libraries(eoslib)

add_subdirectory(gxc.bios)
add_subdirectory(eosio.msig)
add_subdirectory(eosio.wrap)
add_subdirectory(gxc.system)
add_subdirectory(gxc.token)
add_subdirectory(gxc.reserve)
add_subdirectory(gxc.user)
add_subdirectory(gxc.game)
add_subdirectory(gxc.htlc)
add_subdirectory(gxc.bancor)
