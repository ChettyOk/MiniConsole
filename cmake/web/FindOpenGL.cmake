# Emscripten compatibility shim for projects consuming SFML package exports
# that call find_dependency(OpenGL). Web builds use GLES/WebGL through the
# toolchain, so we provide imported interface targets to satisfy CMake deps.

if(NOT TARGET OpenGL::GL)
    add_library(OpenGL::GL INTERFACE IMPORTED)
endif()

if(NOT TARGET OpenGL::OpenGL)
    add_library(OpenGL::OpenGL INTERFACE IMPORTED)
endif()

set(OpenGL_FOUND TRUE)
