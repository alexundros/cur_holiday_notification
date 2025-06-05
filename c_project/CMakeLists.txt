cmake_minimum_required(VERSION 3.25)
project(c_project C)

set(CMAKE_C_STANDARD 11)

include_directories(src/utils)
include_directories(src/utils/ini)

find_package(LibXml2 REQUIRED)
include_directories(${LIBXML2_INCLUDE_DIR})

add_executable(c_project
        src/utils/ini/ini.c
        src/utils/ini/ini.h
        src/utils/ini_utils.c
        src/utils/ini_utils.h
        src/utils/xml_utils.c
        src/utils/xml_utils.h
        src/main.c)

target_link_libraries(c_project ${LIBXML2_LIBRARIES})
