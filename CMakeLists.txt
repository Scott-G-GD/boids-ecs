cmake_minimum_required(VERSION 3.17)

project(engine)
project(sim)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMakeModules")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG")

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)

include_directories(SDL2 PRIVATE "${CMAKE_SOURCE_DIR}/src/engine" "${CMAKE_SOURCE_DIR}/include")

if(APPLE)
	include_directories("/opt/homebrew/include/")
	link_directories("/opt/homebrew/lib/" "./lib/")
else()
	link_directories("./lib/")
endif()

file(
	GLOB_RECURSE
	SIM_SRC
	"${CMAKE_SOURCE_DIR}/src/sim/*.c"
)

add_library(sim STATIC ${SIM_SRC})
set_target_properties(sim PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

target_link_libraries(sim SDL2 SDL2_image SDL2_ttf ecs c m)

file(
	GLOB_RECURSE
	ENGINE_SRC
	"${CMAKE_SOURCE_DIR}/src/engine/*.c"
)

add_executable(engine ${ENGINE_SRC})
target_link_libraries(engine sim SDL2 SDL2_image SDL2_ttf ecs c m)
