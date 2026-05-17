cmake_minimum_required(VERSION 3.16)

# macOS: set the SDK *before* `project()` so the compiler probe uses a sysroot that
# actually contains libc++ (<chrono>, etc.). Without this, CLT/Xcode mismatches or
# Rosetta-launched CMake often produce "fatal error: 'chrono' file not found".
if(APPLE)
    execute_process(
        COMMAND xcrun --sdk macosx --show-sdk-path
        OUTPUT_VARIABLE MC_MACOS_SDK
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
        RESULT_VARIABLE MC_MACOS_SDK_RES)
    if(MC_MACOS_SDK_RES EQUAL 0 AND MC_MACOS_SDK)
        set(CMAKE_OSX_SYSROOT "${MC_MACOS_SDK}" CACHE PATH "macOS SDK (libc++, system headers)" FORCE)
    endif()
endif()

project(MiniConsole VERSION 0.1.0 LANGUAGES CXX)

if(APPLE)
    # Apple Silicon host with an x86_64-only build (e.g. CMake run under Rosetta) breaks
    # libc++ header discovery and mismatches Homebrew arm64 SFML. Prefer native arm64.
    if(CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "arm64" AND CMAKE_OSX_ARCHITECTURES STREQUAL "x86_64")
        message(WARNING "MiniConsole: overriding CMAKE_OSX_ARCHITECTURES x86_64 -> arm64 (Apple Silicon host).")
        set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "macOS architectures" FORCE)
    endif()
    if(CMAKE_OSX_ARCHITECTURES STREQUAL "")
        set(CMAKE_OSX_ARCHITECTURES "${CMAKE_HOST_SYSTEM_PROCESSOR}" CACHE STRING "macOS architectures" FORCE)
    endif()
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(EMSCRIPTEN)
    set(SFML_STATIC_LIBRARIES ON)
    list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/web")
    set(SFML_OS_EMSCRIPTEN ON)
    set(EGL_LIBRARY EGL CACHE STRING "EGL library for Emscripten shim")
    if(CMAKE_SYSROOT)
        set(EGL_INCLUDE_DIR "${CMAKE_SYSROOT}/include" CACHE PATH "EGL include dir for Emscripten shim")
    endif()

    # VRSFML web package configs may reference SFML::GLUtils even when that
    # target is not exported. Predefine a compatibility placeholder so
    # find_package(SFML ...) can import successfully.
    if(NOT TARGET SFML::GLUtils)
        add_library(SFML::GLUtils INTERFACE IMPORTED GLOBAL)
    endif()

    # Some VRSFML package exports for Emscripten also reference a plain
    # `freetype` imported target. Predefine it as a compatibility placeholder.
    if(NOT TARGET freetype)
        add_library(freetype INTERFACE IMPORTED GLOBAL)
    endif()
endif()

find_package(SFML 3 COMPONENTS Graphics Window System REQUIRED)

add_executable(miniconsole
    src/main.cpp
    src/core/Application.cpp
    src/core/HighScoreBoard.cpp
    src/states/MenuState.cpp
    src/states/BreakoutState.cpp
    src/states/HighScoresState.cpp
    src/states/MinesweeperState.cpp
    src/states/PacmanState.cpp
    src/states/PlatformerState.cpp
    src/states/ShooterState.cpp
    src/states/StubGameState.cpp
    src/states/TowerDefenseState.cpp
    src/game/breakout/BreakoutWorld.cpp
    src/game/breakout/BreakoutView.cpp
    src/game/minesweeper/MinesweeperWorld.cpp
    src/game/minesweeper/MinesweeperView.cpp
    src/game/pacman/PacmanWorld.cpp
    src/game/pacman/PacmanView.cpp
    src/game/platformer/PlatformerWorld.cpp
    src/game/platformer/PlatformerView.cpp
    src/game/shooter/ShooterWorld.cpp
    src/game/shooter/ShooterView.cpp
    src/game/towerdefense/TowerDefenseWorld.cpp
    src/game/towerdefense/TowerDefenseView.cpp
)

target_include_directories(miniconsole PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

# Apple Clang often adds an incomplete libc++ path under CommandLineTools that lacks
# standard headers like <chrono>. Put the SDK's libc++ first so C++ mode works reliably.
if(APPLE AND CMAKE_OSX_SYSROOT)
    target_compile_options(miniconsole PRIVATE "-isystem${CMAKE_OSX_SYSROOT}/usr/include/c++/v1")
endif()

target_link_libraries(miniconsole PRIVATE SFML::Graphics SFML::Window SFML::System)

if(EMSCRIPTEN)
    set_target_properties(miniconsole PROPERTIES SUFFIX ".html")
    target_link_options(miniconsole PRIVATE
        "-sALLOW_MEMORY_GROWTH=1"
        "-sNO_EXIT_RUNTIME=1"
        "-sASSERTIONS=1"
        "-sUSE_WEBGL2=1"
        "-sFULL_ES3=1"
        "--preload-file" "${CMAKE_CURRENT_SOURCE_DIR}/levels@/levels"
        "--shell-file" "${CMAKE_CURRENT_SOURCE_DIR}/web/shell_miniconsole.html"
    )
endif()

if(MSVC)
    target_compile_options(miniconsole PRIVATE /W4 /permissive-)
else()
    target_compile_options(miniconsole PRIVATE -Wall -Wextra -Wpedantic)
endif()
