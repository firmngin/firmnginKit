#!/bin/bash

# Path dari tools/ ke root dan library folder
ROOT_DIR=".."
LIB_DIR="$ROOT_DIR/lib/firmnginKit"
LIBRARY_PROPS="$ROOT_DIR/library.properties"

# Periksa apakah file library.properties ada
if [ ! -f "$LIBRARY_PROPS" ]; then
    echo "Error: File library.properties tidak ditemukan di $LIBRARY_PROPS"
    echo "Menggunakan library.properties dari folder library sebagai fallback"
    LIBRARY_PROPS="$LIB_DIR/library.properties"
fi

# Tampilkan isi library.properties untuk debugging
echo "Menggunakan library.properties dari: $LIBRARY_PROPS"
echo "Isi library.properties:"
cat "$LIBRARY_PROPS"
echo "-------------------"

# Baca versi dari library.properties
VERSION=$(grep "version=" "$LIBRARY_PROPS" | cut -d'=' -f2)

# Periksa apakah versi berhasil diambil
if [ -z "$VERSION" ]; then
    echo "Error: Tidak dapat membaca versi dari library.properties"
    echo "Menggunakan versi default 1.0.0"
    VERSION="1.0.0"
fi

LIBRARY_NAME="firmnginKit"
RELEASE_DIR="$ROOT_DIR/releases/${LIBRARY_NAME}-${VERSION}"

echo "Membuat release untuk ${LIBRARY_NAME} versi ${VERSION}"

# Buat direktori release
rm -rf "$RELEASE_DIR"
mkdir -p "$RELEASE_DIR/src"
mkdir -p "$RELEASE_DIR/examples/BasicExample"

# Copy file-file utama
cp "$LIBRARY_PROPS" "$RELEASE_DIR/library.properties"
cp "$LIB_DIR/README.md" "$RELEASE_DIR/" 2>/dev/null || echo "README.md tidak ditemukan"
cp "$LIB_DIR/keywords.txt" "$RELEASE_DIR/" 2>/dev/null || echo "keywords.txt tidak ditemukan"
cp "$LIB_DIR/CHANGELOG.md" "$RELEASE_DIR/" 2>/dev/null || echo "CHANGELOG.md tidak ditemukan"

# Copy source files
cp "$LIB_DIR/firmnginKit.h" "$LIB_DIR/firmnginKit.cpp" "$RELEASE_DIR/src/"

# Copy examples
mkdir -p "$RELEASE_DIR/examples/BasicExample"
cp "$LIB_DIR/examples/BasicExample/BasicExample.ino" "$RELEASE_DIR/examples/BasicExample/" 2>/dev/null || echo "Example tidak ditemukan"

# Buat folder releases jika belum ada
mkdir -p "$ROOT_DIR/releases"

# Buat ZIP
ZIP_FILE="$ROOT_DIR/releases/${LIBRARY_NAME}-${VERSION}.zip"
rm -f "$ZIP_FILE"
(cd "$ROOT_DIR/releases" && zip -r "${LIBRARY_NAME}-${VERSION}.zip" "${LIBRARY_NAME}-${VERSION}")

echo "Release package dibuat: $ZIP_FILE"

# Cleanup
rm -rf "$RELEASE_DIR"

# Sinkronkan library.properties ke folder lib/firmnginKit
if [ -f "$LIBRARY_PROPS" ]; then
    echo "Menyinkronkan library.properties ke folder lib/firmnginKit"
    cp "$LIBRARY_PROPS" "$LIB_DIR/library.properties"
    echo "File library.properties telah disinkronkan"
else
    echo "File library.properties tidak ditemukan di root, tidak ada yang disinkronkan"
fi

echo "Selesai!"