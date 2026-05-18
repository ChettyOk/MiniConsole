#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SRC_DIR="${ROOT_DIR}/sfml-src"
BUILD_DIR="${ROOT_DIR}/sfml-build"
INSTALL_DIR="${ROOT_DIR}/sfml-install"
VRSFML_REF="3.0.2"

if ! command -v emcmake >/dev/null 2>&1; then
  echo "error: emcmake not found. Install/activate Emscripten SDK first."
  echo "       https://emscripten.org/docs/getting_started/downloads.html"
  exit 1
fi

if [[ ! -d "${SRC_DIR}" ]]; then
  echo "Cloning VRSFML..."
  git clone --depth 1 --branch "${VRSFML_REF}" --recurse-submodules https://github.com/vittorioromeo/VRSFML.git "${SRC_DIR}"
else
  echo "Using existing ${SRC_DIR}"
fi

echo "Pinning VRSFML revision for web build..."
git -C "${SRC_DIR}" fetch --depth 1 origin "${VRSFML_REF}"
git -C "${SRC_DIR}" checkout "${VRSFML_REF}"

echo "Patching VRSFML CMake for Emscripten platform detection..."
python3 - "${SRC_DIR}" <<'PY'
import sys
from pathlib import Path

cfg = Path(sys.argv[1]) / "cmake" / "Config.cmake"
text = cfg.read_text()

if "elseif(${EMSCRIPTEN})" not in text:
    marker = "elseif(${CYGWIN})"
    if marker not in text:
        raise SystemExit("Could not find CYGWIN marker in VRSFML Config.cmake")
    injected = """elseif(${EMSCRIPTEN})
    message(STATUS "Detected Emscripten")
    set(SFML_OS_EMSCRIPTEN 1)

    # use the OpenGL ES implementation on Emscripten
    set(OPENGL_ES 1)
"""
    text = text.replace(marker, injected + "\n" + marker, 1)
    cfg.write_text(text)
    print("Injected EMSCRIPTEN platform block.")
else:
    print("EMSCRIPTEN platform block already present.")
PY

echo "Patching VRSFML CMake for OpenGL target shims..."
python3 - "${SRC_DIR}" <<'PY'
import sys
from pathlib import Path

root_cmake = Path(sys.argv[1]) / "CMakeLists.txt"
text = root_cmake.read_text()

if "MiniConsole OpenGL shim patch" not in text:
    marker = "project("
    pos = text.find(marker)
    if pos == -1:
        raise SystemExit("Could not find project() in VRSFML CMakeLists.txt")

    line_end = text.find("\n", pos)
    if line_end == -1:
        line_end = len(text)

    shim = """

# MiniConsole OpenGL shim patch (Emscripten)
if(EMSCRIPTEN)
    if(NOT TARGET OpenGL::GL)
        add_library(OpenGL::GL INTERFACE IMPORTED)
    endif()
    if(NOT TARGET OpenGL::OpenGL)
        add_library(OpenGL::OpenGL INTERFACE IMPORTED)
    endif()
endif()
"""
    text = text[:line_end + 1] + shim + text[line_end + 1:]
    root_cmake.write_text(text)
    print("Injected OpenGL imported target shim block.")
else:
    print("OpenGL shim block already present.")
PY

echo "Configuring VRSFML for Emscripten..."
rm -rf "${BUILD_DIR}"
emcmake cmake -S "${SRC_DIR}" -B "${BUILD_DIR}" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
  -DCMAKE_MODULE_PATH="${ROOT_DIR}/cmake/web" \
  -DBUILD_SHARED_LIBS=OFF \
  -DSFML_USE_SYSTEM_DEPS=OFF \
  -DSFML_BUILD_AUDIO=OFF \
  -DSFML_BUILD_NETWORK=OFF \
  -DSFML_BUILD_EXAMPLES=OFF \
  -DSFML_BUILD_TEST_SUITE=OFF \
  -DSFML_BUILD_GLUTILS=OFF

echo "Building and installing VRSFML..."
cmake --build "${BUILD_DIR}" --target install -j 4

echo
echo "Done. Export this before running scripts/build_web.sh:"
echo "  export SFML_DIR=\"${INSTALL_DIR}/lib/cmake/SFML\""
