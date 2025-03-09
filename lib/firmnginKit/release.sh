#!/bin/bash

# Baca versi dari library.properties
VERSION=$(grep "version=" library.properties | cut -d'=' -f2)
LIBRARY_NAME="firmnginKit"
RELEASE_DIR="${LIBRARY_NAME}-${VERSION}"

echo "Membuat release untuk ${LIBRARY_NAME} versi ${VERSION}"

# Buat direktori release
rm -rf $RELEASE_DIR
mkdir -p $RELEASE_DIR/src
mkdir -p $RELEASE_DIR/examples/BasicExample

# Copy file-file utama
cp library.properties $RELEASE_DIR/
cp README.md $RELEASE_DIR/
cp keywords.txt $RELEASE_DIR/

# Copy source files
cp firmnginKit.h firmnginKit.cpp $RELEASE_DIR/src/

# Copy examples
cp examples/BasicExample/BasicExample.ino $RELEASE_DIR/examples/BasicExample/

# Buat ZIP
rm -f "${RELEASE_DIR}.zip"
zip -r "${RELEASE_DIR}.zip" $RELEASE_DIR

echo "Release package dibuat: ${RELEASE_DIR}.zip"

# Cleanup
rm -rf $RELEASE_DIR

echo "Selesai!" 