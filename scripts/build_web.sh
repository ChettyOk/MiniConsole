#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build-web"

if ! command -v emcmake >/dev/null 2>&1; then
  echo "error: emcmake not found. Install/activate Emscripten SDK first."
  echo "       https://emscripten.org/docs/getting_started/downloads.html"
  exit 1
fi

if [[ -z "${SFML_DIR:-}" ]]; then
  echo "error: SFML_DIR is not set."
  echo "       Build VRSFML for Emscripten and export SFML_DIR to its cmake package path."
  echo "       Example: export SFML_DIR=\$PWD/sfml-install/lib/cmake/SFML"
  exit 1
fi

echo "Configuring web build..."
emcmake cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" \
  -DCMAKE_BUILD_TYPE=Release \
  -DSFML_STATIC_LIBRARIES=ON \
  -DSFML_DIR="${SFML_DIR}"

echo "Building web artifacts..."
cmake --build "${BUILD_DIR}" -j 4

echo "Done. Open ${BUILD_DIR}/miniconsole.html in a local web server."
