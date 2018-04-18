cmake_minimum_required(VERSION 3.0.2)
project(confluence_update)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads)
find_package(OpenSSL)
find_package(Boost COMPONENTS system filesystem)

find_path(BEAST_INC "boost/beast/core.hpp")

add_library(beast_http basichttps.cpp confluencewebdav.cpp)

target_include_directories(beast_http PUBLIC 
	${BEAST_INC}
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>
)

target_link_libraries(beast_http Boost::system Boost::filesystem Threads::Threads OpenSSL::SSL)

export(TARGETS beast_http FILE BeastHttpConfig.cmake)
