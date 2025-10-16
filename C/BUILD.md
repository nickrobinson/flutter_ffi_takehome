# Building the Ditto FFI Library

This document explains how to build the native C library for the Flutter FFI interview module.

## 🎯 Overview

The Ditto library is a simple key-value store implemented in C with support for:
- CRUD operations (Create, Read, Update, Delete)
- Change notifications via callbacks
- Thread-safe operations using mutexes
- Cross-platform support (macOS, Linux, Windows)

## 📋 Prerequisites

### All Platforms
- CMake 3.15 or higher
- A C11-compatible compiler
- pthread support

### macOS
```bash
# Install Xcode Command Line Tools
xcode-select --install

# Or install via Homebrew
brew install cmake
```

### Linux (Ubuntu/Debian)
```bash
sudo apt-get update
sudo apt-get install build-essential cmake
```

### Windows
- Visual Studio 2019 or later with C++ build tools
- CMake (download from https://cmake.org/ or use `choco install cmake`)

## 🚀 Quick Build

### Option 1: Using the Build Script (macOS/Linux)

```bash
cd C
./build.sh
```

This will:
1. Create a `build` directory
2. Configure CMake for your platform
3. Build the library in Release mode
4. Install it to the appropriate platform directory

### Option 2: Manual CMake Build

```bash
cd C
mkdir build
cd build

# Configure
cmake -DCMAKE_BUILD_TYPE=Release ..

# Build
cmake --build . --config Release

# Install to platform directory
cmake --install . --config Release
```

### Windows-Specific

```cmd
cd C
mkdir build
cd build

# Configure for Visual Studio
cmake -G "Visual Studio 16 2019" -A x64 ..

# Build
cmake --build . --config Release

# Install
cmake --install . --config Release
```

## 📦 Output Locations

After building, the libraries will be placed in:

- **macOS**: `C/macos/libdittoffi.dylib`
- **Linux**: `C/linux/libdittoffi.so`
- **Windows**: `C/windows/dittoffi.dll`

## 🔍 Verifying the Build

### macOS
```bash
# Check if library exists
ls -lh macos/libdittoffi.dylib

# Check dependencies
otool -L macos/libdittoffi.dylib

# Check exported symbols
nm -g macos/libdittoffi.dylib | grep ditto
```

### Linux
```bash
# Check if library exists
ls -lh linux/libdittoffi.so

# Check dependencies
ldd linux/libdittoffi.so

# Check exported symbols
nm -D linux/libdittoffi.so | grep ditto
```

### Windows
```cmd
# Check if library exists
dir windows\dittoffi.dll

# Check dependencies (requires Dependency Walker or similar)
dumpbin /DEPENDENTS windows\dittoffi.dll

# Check exported symbols
dumpbin /EXPORTS windows\dittoffi.dll
```

## 🧪 Testing the Library

After building, you can test it with the Flutter application:

```bash
cd ../app/flutter
flutter pub get
flutter run -d macos  # or linux/windows
```

## 🛠️ Implementation Details

### Architecture

The library uses:
- **Hash table**: Simple chained hash table with 256 buckets
- **Thread safety**: Pthread mutexes for concurrent access
- **Memory management**: Dynamic allocation with proper cleanup
- **Callbacks**: Function pointers for change notifications

### Key Features

1. **In-Memory Storage**: Fast key-value operations using a hash table
2. **Thread-Safe**: All operations protected by mutexes
3. **Subscriptions**: Up to 100 concurrent change subscriptions
4. **Buffer Resizing**: Two-step get operation for variable-sized values
5. **Error Handling**: Standard error codes (0=success, 1=buffer too small, 2=not found)

### Memory Layout

```
ditto_db_t
├── hash_table_t (256 buckets)
│   ├── kv_entry_t (linked list per bucket)
│   │   ├── key (string)
│   │   ├── data (uint8_t array)
│   │   └── len (size)
│   └── lock (pthread_mutex)
└── subscriptions[100]
    ├── callback function pointer
    ├── user_data
    └── active flag
```

## 🐛 Troubleshooting

### Build Fails with "pthread not found"

**Solution**: Ensure pthread is available on your system.

- **Linux**: `sudo apt-get install libpthread-stubs0-dev`
- **macOS**: Should be included by default
- **Windows**: Handled by CMake automatically

### "Library not found" when running Flutter app

**Solution**: Ensure the library is in the correct location:

```bash
# Check library exists
ls C/macos/libdittoffi.dylib  # or linux/windows equivalent

# Verify permissions
chmod 755 C/macos/libdittoffi.dylib
```

### macOS: "cannot be opened because the developer cannot be verified"

**Solution**: Allow unsigned libraries (development only):

```bash
xattr -d com.apple.quarantine C/macos/libdittoffi.dylib
```

### Windows: DLL dependencies missing

**Solution**: Ensure Visual C++ Redistributables are installed, or build with static linking.

## 🔧 Advanced Configuration

### Debug Build

For debugging with symbols:

```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . --config Debug
```

### Custom Installation Path

```bash
cmake -DCMAKE_INSTALL_PREFIX=/custom/path ..
cmake --install . --config Release --prefix /custom/path
```

### Cross-Compilation

For building Linux binaries on macOS (requires Docker):

```bash
docker run --rm -v $(pwd):/workspace -w /workspace ubuntu:22.04 bash -c "
  apt-get update && apt-get install -y build-essential cmake &&
  cd /workspace/C &&
  ./build.sh
"
```

## 📝 Modifying the Library

If you need to modify the library implementation:

1. Edit `C/src/ditto.c`
2. Keep the API in `C/include/ditto.h` unchanged (Flutter depends on it)
3. Rebuild using `./build.sh`
4. Test with the Flutter app

### Adding New Features

Example: Add a "count" operation:

1. Add to `ditto.h`:
```c
DITTO_API int32_t ditto_count(ditto_db_t* db, int32_t* out_count);
```

2. Implement in `ditto.c`:
```c
DITTO_API int32_t ditto_count(ditto_db_t* db, int32_t* out_count) {
    if (!db || !out_count) return -1;

    pthread_mutex_lock(&db->table->lock);
    int32_t count = 0;
    for (int i = 0; i < HASH_SIZE; i++) {
        kv_entry_t* entry = db->table->buckets[i];
        while (entry) {
            count++;
            entry = entry->next;
        }
    }
    pthread_mutex_unlock(&db->table->lock);

    *out_count = count;
    return 0;
}
```

3. Rebuild and update Flutter bindings

## 📚 Additional Resources

- [CMake Documentation](https://cmake.org/documentation/)
- [Flutter FFI Guide](https://dart.dev/guides/libraries/c-interop)
- [pthread Programming](https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/pthread.h.html)

## 🤝 For Interview Candidates

**Note**: The library is pre-built for you. You don't need to build it yourself unless:
- You want to understand the implementation
- You need to debug native code
- You're modifying the C library as part of the exercise

Your main task is to integrate the pre-built library into Flutter using FFI.

---

**Questions?** Check the main README.md or contact the interviewer.
