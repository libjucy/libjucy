cmake_minimum_required(VERSION 3.15)
project(jucy VERSION 0.1.0)
if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif()

find_package(Qt5 ${QT_MIN_VERSION} REQUIRED CONFIG COMPONENTS Core)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(QT_MIN_VERSION "5.11.0")
set(ENV{PKG_CONFIG_PATH} "/usr/lib/pkgconfig:/usr/local/lib/pkgconfig")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${CMAKE_CURRENT_LIST_DIR}/cmake)

include(FindPkgConfig)
include(PySide2Helper)
include(Shiboken2Helper)

pkg_check_modules(Jack REQUIRED jack)

add_subdirectory(juce)
add_subdirectory(src)
add_subdirectory(pyside_bindings)
