#!/bin/bash
# build.sh - Cross-platform build script for dittoffi library

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}======================================${NC}"
echo -e "${GREEN}Building Ditto FFI Library${NC}"
echo -e "${GREEN}======================================${NC}"

# Get the directory where this script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

# Detect platform
PLATFORM=$(uname -s)
echo -e "\n${YELLOW}Platform detected: $PLATFORM${NC}"

# Create build directory
BUILD_DIR="build"
if [ -d "$BUILD_DIR" ]; then
    echo -e "${YELLOW}Cleaning existing build directory...${NC}"
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure and build
echo -e "\n${YELLOW}Configuring build...${NC}"
cmake -DCMAKE_BUILD_TYPE=Release ..

echo -e "\n${YELLOW}Building library...${NC}"
cmake --build . --config Release

echo -e "\n${YELLOW}Installing library to platform directory...${NC}"
cmake --install . --config Release

cd ..

# Verify the build
echo -e "\n${GREEN}======================================${NC}"
echo -e "${GREEN}Build Complete!${NC}"
echo -e "${GREEN}======================================${NC}"

case "$PLATFORM" in
    Darwin*)
        LIB_PATH="macos/libdittoffi.dylib"
        ;;
    Linux*)
        LIB_PATH="linux/libdittoffi.so"
        ;;
    MINGW*|MSYS*|CYGWIN*)
        LIB_PATH="windows/dittoffi.dll"
        ;;
    *)
        echo -e "${RED}Unknown platform: $PLATFORM${NC}"
        exit 1
        ;;
esac

if [ -f "$LIB_PATH" ]; then
    echo -e "${GREEN}✓ Library created: $LIB_PATH${NC}"

    # Show file info
    ls -lh "$LIB_PATH"

    # On macOS, show library dependencies
    if [[ "$PLATFORM" == "Darwin"* ]]; then
        echo -e "\n${YELLOW}Library dependencies:${NC}"
        otool -L "$LIB_PATH"
    fi

    # On Linux, show library dependencies
    if [[ "$PLATFORM" == "Linux"* ]]; then
        echo -e "\n${YELLOW}Library dependencies:${NC}"
        ldd "$LIB_PATH" || true
    fi
else
    echo -e "${RED}✗ Library not found at expected location: $LIB_PATH${NC}"
    exit 1
fi

echo -e "\n${GREEN}Library is ready for use with Flutter!${NC}"
echo -e "${YELLOW}Next steps:${NC}"
echo "  cd ../app/flutter"
echo "  flutter pub get"
echo "  flutter run -d macos  # or linux/windows"
