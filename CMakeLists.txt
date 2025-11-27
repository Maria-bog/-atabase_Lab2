cmake_minimum_required(VERSION 3.16)
project(FileDatabase)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

set(SOURCES
    main.cpp
    Database.cpp
    FileManager.cpp
    GUI.cpp
)

set(HEADERS
    Database.h
    FileManager.h
    GUI.h
)

qt6_wrap_cpp(HEADERS_MOC ${HEADERS})

add_executable(filedb ${SOURCES} ${HEADERS_MOC})

target_link_libraries(filedb Qt6::Core Qt6::Widgets)

install(TARGETS filedb DESTINATION bin)