cmake_minimum_required(VERSION 3.16)
project(chord-dht CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()

find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

add_executable(node
	ChordNode/node.cpp
	ChordNode/nodeclass.cpp
	ChordNode/nodeserver.cpp
	ChordNode/nodesync.cpp
	ChordNode/util.cpp
	ChordNode/log.cpp
)
target_link_libraries(node PRIVATE OpenSSL::SSL OpenSSL::Crypto Threads::Threads)
set_target_properties(node PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/ChordNode")

add_executable(client
	ChordClient/client.cpp
	ChordClient/clientutil.cpp
)
set_target_properties(client PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/ChordClient")

add_custom_target(script
	COMMAND python3 create_nodes.py 4
	COMMAND python3 create_clients.py 2
	WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
	DEPENDS node client
)
