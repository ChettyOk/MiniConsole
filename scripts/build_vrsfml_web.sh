#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SRC_DIR="${ROOT_DIR}/sfml-src"
BUILD_DIR="${ROOT_DIR}/sfml-build"
INSTALL_DIR="${ROOT_DIR}/sfml-install"

if ! command -v emcmake >/dev/null 2>&1; then
  echo "error: emcmake not found. Install/activate Emscripten SDK first."
  echo "       https://emscripten.org/docs/getting_started/downloads.html"
  exit 1
fi

if [[ ! -d "${SRC_DIR}" ]]; then
  echo "Cloning VRSFML..."
  git clone --depth 1 --recurse-submodules https://github.com/vittorioromeo/VRSFML.git "${SRC_DIR}"
  git -C "${SRC_DIR}" fetch --depth 1 origin b87231e8fc0bc3480c004232b3bec4dc083218ab
  git -C "${SRC_DIR}" checkout b87231e8fc0bc3480c004232b3bec4dc083218ab
else
  echo "Using existing ${SRC_DIR}"
fi

echo "Pinning VRSFML revision for web build..."
git -C "${SRC_DIR}" fetch --depth 1 origin b87231e8fc0bc3480c004232b3bec4dc083218ab
git -C "${SRC_DIR}" checkout b87231e8fc0bc3480c004232b3bec4dc083218ab

echo "Configuring VRSFML for Emscripten..."
emcmake cmake -S "${SRC_DIR}" -B "${BUILD_DIR}" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
  -DBUILD_SHARED_LIBS=OFF \
  -DSFML_STATIC_LIBRARIES=ON \
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
