cmake_minimum_required(VERSION 3.28)

project(libtde
    VERSION 0.1.0
    DESCRIPTION "What TDE applications share: configuration, theme, window frame and dialogs"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(TDE_BUILD_TESTS "Build the unit tests" ${PROJECT_IS_TOP_LEVEL})

find_package(Qt6 6.8 REQUIRED COMPONENTS Core Gui Widgets)
find_package(PkgConfig REQUIRED)
pkg_search_module(LUA REQUIRED IMPORTED_TARGET lua lua5.5 lua-5.5 lua5.4 lua-5.4 lua54)

qt_standard_project_setup()
include(GNUInstallDirs)
# Installed among the system libraries, it needs no RPATH.
set(CMAKE_SKIP_INSTALL_RPATH ON)

set(public_headers
    include/tde/ConfigWatcher.hpp
    include/tde/DesktopConfig.hpp
    include/tde/Dialog.hpp
    include/tde/FramelessHelper.hpp
    include/tde/HeaderBar.hpp
    include/tde/LuaConfig.hpp
    include/tde/Theme.hpp
    include/tde/Toast.hpp
    include/tde/WindowButtons.hpp
)

add_library(tde SHARED
    ${public_headers}
    src/ConfigWatcher.cpp
    src/DesktopConfig.cpp
    src/Dialog.cpp
    src/FramelessHelper.cpp
    src/HeaderBar.cpp
    src/IconRepair.cpp
    src/IconRepair.hpp
    src/LuaConfig.cpp
    src/Theme.cpp
    src/Toast.cpp
    src/WindowButtons.cpp
)
add_library(Tde::Tde ALIAS tde)
# The documented defaults, written out as the user's desktop config when there is none.
qt_add_resources(tde defaults PREFIX /tde BASE data FILES data/config.lua)
target_compile_options(tde PRIVATE -Wall -Wextra -Wpedantic)
target_include_directories(tde
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE src)
target_link_libraries(tde
    PUBLIC Qt6::Core Qt6::Gui Qt6::Widgets
    PRIVATE PkgConfig::LUA)
# Until 1.0, every minor version may change the ABI.
set_target_properties(tde PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    EXPORT_NAME Tde)

install(TARGETS tde EXPORT TdeTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES ${public_headers} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tde)
install(EXPORT TdeTargets NAMESPACE Tde:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Tde)

include(CMakePackageConfigHelpers)
configure_package_config_file(cmake/TdeConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/TdeConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Tde)
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/TdeConfigVersion.cmake
    COMPATIBILITY SameMinorVersion)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/TdeConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/TdeConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Tde)
# The desktop-wide config, as an example to copy to ~/.config/tde.
install(FILES data/config.lua DESTINATION ${CMAKE_INSTALL_DOCDIR}/examples)
install(FILES README.md DESTINATION ${CMAKE_INSTALL_DOCDIR})

if(TDE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
