cmake_minimum_required(VERSION 3.16)
project(
	GDEM_Downloader
	VERSION 1.0.0
	LANGUAGES CXX
)

set(CMAKE_DEBUG_POSTFIX _d)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

find_package(CURL REQUIRED)

file(GLOB CPP_FILES
    "./src/*.cpp"
	"./src/client/*.cc"
)

add_executable(${PROJECT_NAME} ${CPP_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE ${CURL_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${CURL_LIBRARY})