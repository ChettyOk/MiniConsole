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
  if [[ -f "${ROOT_DIR}/sfml-install/lib/cmake/SFML/SFMLConfig.cmake" ]]; then
    export SFML_DIR="${ROOT_DIR}/sfml-install/lib/cmake/SFML"
    echo "SFML_DIR not set; using ${SFML_DIR}"
  else
    echo "error: SFML_DIR is not set and local sfml-install was not found."
    echo "       Run ./scripts/build_vrsfml_web.sh first, then re-run this script."
    exit 1
  fi
fi

echo "Configuring web build..."
emcmake cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" \
  -DCMAKE_BUILD_TYPE=Release \
  -DSFML_STATIC_LIBRARIES=ON \
  -DSFML_DIR="${SFML_DIR}"

echo "Building web artifacts..."
cmake --build "${BUILD_DIR}" -j 4

echo "Done. Open ${BUILD_DIR}/miniconsole.html in a local web server."
