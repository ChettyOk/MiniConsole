#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build-web"
SITE_DIR="${ROOT_DIR}/site"

if [[ ! -d "${BUILD_DIR}" ]]; then
  echo "error: ${BUILD_DIR} does not exist."
  echo "       Run ./scripts/build_web.sh first."
  exit 1
fi

rm -rf "${SITE_DIR}"
mkdir -p "${SITE_DIR}"
touch "${SITE_DIR}/.nojekyll"

shopt -s nullglob
HTML_FILES=("${BUILD_DIR}"/*.html)
JS_FILES=("${BUILD_DIR}"/*.js)
WASM_FILES=("${BUILD_DIR}"/*.wasm)
DATA_FILES=("${BUILD_DIR}"/*.data)
shopt -u nullglob

if [[ ${#HTML_FILES[@]} -eq 0 || ${#JS_FILES[@]} -eq 0 || ${#WASM_FILES[@]} -eq 0 ]]; then
  echo "error: expected .html, .js, and .wasm artifacts in ${BUILD_DIR}"
  exit 1
fi

cp "${HTML_FILES[0]}" "${SITE_DIR}/index.html"
cp "${JS_FILES[0]}" "${SITE_DIR}/"
cp "${WASM_FILES[0]}" "${SITE_DIR}/"

if [[ ${#DATA_FILES[@]} -gt 0 ]]; then
  cp "${DATA_FILES[0]}" "${SITE_DIR}/"
fi

echo "Packaged site assets in ${SITE_DIR}"
