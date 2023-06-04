cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

project(FramelessWindow LANGUAGES CXX)

set(Qt6_BIN "$ENV{QT_SDK}")
cmake_path(GET Qt6_BIN PARENT_PATH Qt6_DIR)
set(Qt6_CMAKE "${Qt6_DIR}/lib/cmake")
message("${Qt6_CMAKE}")
list(APPEND CMAKE_PREFIX_PATH "${Qt6_CMAKE}")

find_package(Qt6 REQUIRED COMPONENTS Widgets)

message("${CMAKE_CURRENT_SOURCE_DIR}/FramelessWindow/")

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/FramelessWindow/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/FramelessWindow/*.ui  
    ${CMAKE_CURRENT_SOURCE_DIR}/FramelessWindow/*.qrc
 	main.cpp
)

qt_add_executable(${PROJECT_NAME} MANUAL_FINALIZATION ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets)

set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE ON
)

qt_finalize_executable(${PROJECT_NAME})