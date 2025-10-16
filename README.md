# Flutter FFI Interview Module - Candidate Instructions

Welcome! This exercise tests your ability to integrate a pre-built C library into a Flutter desktop application using Dart's Foreign Function Interface (FFI).

## 🎯 Objective

Build a Flutter desktop application that integrates with the **Ditto KV Store** - a simple key-value database implemented as a native C library. You'll need to:

1. Load the native library for your platform
2. Create FFI bindings to call C functions from Dart
3. Handle native callbacks (C → Dart communication)
4. Build a functional UI to interact with the database

## ⏱️ Time Estimate

**90 minutes** (core functionality)

You can take longer if you'd like to add polish, but aim to have the basics working within this timeframe.

## 📚 What's Provided

### 1. Native C Library (Pre-built)

Located in `C/` directory:
- **Header file**: `C/include/ditto.h` - Complete API documentation
- **Libraries**:
  - macOS: `C/macos/libdittoffi.dylib`
  - Linux: `C/linux/libdittoffi.so`
  - Windows: `C/windows/dittoffi.dll`

### 2. API Overview

The Ditto library provides 8 functions:

```c
// Database lifecycle
int32_t ditto_open(const char* path, ditto_db_t** out_db);
void ditto_close(ditto_db_t* db);

// CRUD operations
int32_t ditto_put(ditto_db_t* db, const char* key, const uint8_t* data, size_t len);
int32_t ditto_get(ditto_db_t* db, const char* key, uint8_t* out_buf, size_t* inout_len);
int32_t ditto_delete(ditto_db_t* db, const char* key);

// Change notifications
int32_t ditto_subscribe(ditto_db_t* db, ditto_on_change_cb cb, void* user_data, int32_t* out_sub_id);
int32_t ditto_unsubscribe(ditto_db_t* db, int32_t sub_id);

// Utility
const char* ditto_version(void);
```

**Error Codes:**
- `0` = Success
- `1` = Buffer too small (get operation only)
- `2` = Key not found
- Other non-zero = General error

### 3. Flutter Project Scaffold

A basic Flutter project is set up in `app/flutter/`:
- `pubspec.yaml` - Dependencies configured (includes `ffi: ^2.1.0`)
- Desktop support enabled for macOS, Linux, Windows

## 🎓 What You Need to Build

### Part 1: Native Library Loading (15 min)

Create `lib/native/ditto_loader.dart`:
- Detect the current platform (macOS/Linux/Windows)
- Load the appropriate shared library using `DynamicLibrary.open()`
- Handle errors gracefully with clear error messages

**Key Concepts:**
- `dart:ffi` and `dart:io`
- Platform detection
- Dynamic library loading

### Part 2: FFI Bindings (30 min)

Create `lib/native/ditto_bindings.dart`:

1. **Define C function signatures** using FFI typedefs:
   ```dart
   typedef _OpenC = Int32 Function(Pointer<Utf8>, Pointer<Pointer<Void>>);
   typedef _OpenDart = int Function(Pointer<Utf8>, Pointer<Pointer<Void>>);
   ```

2. **Create a wrapper class** (`DittoDb`) with Dart-friendly methods:
   - `DittoDb.open(String path)` - Opens database
   - `put(String key, Uint8List value)` - Stores data
   - `get(String key)` - Retrieves data (handle buffer resizing!)
   - `delete(String key)` - Removes key
   - `subscribe()` - Returns a `Stream<String>` for change notifications
   - `close()` - Cleanup

3. **Handle memory management:**
   - Use `package:ffi` (`calloc`, `free`)
   - Always free allocated memory (try-finally blocks)
   - String conversions: `toNativeUtf8()` and `toDartString()`

4. **Implement the get() two-step buffer resize pattern:**
   - First call with `nullptr` to get size
   - Allocate buffer with exact size
   - Second call to retrieve data

**Key Concepts:**
- FFI typedefs (C vs Dart signatures)
- Pointer manipulation
- Memory management (allocate/free)
- String and byte array conversions

### Part 3: Native → Dart Callbacks (25 min)

Implement change notifications:

1. **Static callback trampoline:**
   ```dart
   void _nativeChangeCallback(Pointer<Void> userData, Pointer<Utf8> keyPtr) {
     // Extract key and route to Dart
   }

   final callbackPtr = Pointer.fromFunction<_OnChangeCallbackC>(_nativeChangeCallback);
   ```

2. **Subscription management:**
   - Use a global registry to map subscription IDs to Dart callbacks
   - Return a `Stream<String>` from `subscribe()`
   - Use `scheduleMicrotask()` for thread safety
   - Handle cleanup in `unsubscribe()`

**Key Concepts:**
- Static callback functions
- `Pointer.fromFunction()`
- Stream controllers
- Cross-thread communication

### Part 4: Flutter UI (20 min)

Create `lib/main.dart` with a functional interface:

**Required UI Elements:**
- Text fields for key and value input
- Buttons: Put, Get, Delete, Clear
- Display area for retrieved values
- Live updates panel showing change notifications
- Version display
- Error handling (dialogs/snackbars)

**UI Flow:**
1. App starts → opens database
2. User enters key/value → clicks Put → data stored + notification appears
3. User enters key → clicks Get → value displayed
4. User enters key → clicks Delete → key removed + notification appears

**Key Concepts:**
- `StatefulWidget`
- Async operations (`Future`, `async/await`)
- Stream subscriptions
- Error handling in UI

## 📋 Evaluation Criteria

Your submission will be evaluated on:

### 1. Correct FFI Integration (40%)
- ✅ Library loads successfully on your platform
- ✅ All C functions properly bound with correct types
- ✅ Memory management (no leaks, proper cleanup)
- ✅ Error codes handled appropriately

### 2. Callback Implementation (30%)
- ✅ Native → Dart callbacks work correctly
- ✅ Static trampoline properly implemented
- ✅ Thread-safe callback delivery
- ✅ Subscription lifecycle managed

### 3. Functionality (20%)
- ✅ Put, Get, Delete operations work
- ✅ Change notifications display in real-time
- ✅ Version string displays correctly
- ✅ App doesn't crash on errors

### 4. Code Quality (10%)
- ✅ Clean, readable code
- ✅ Proper error handling
- ✅ Sensible architecture (separation of concerns)
- ✅ Comments where helpful

## 🚀 Getting Started

### Step 1: Verify Prerequisites

```bash
# Check Flutter installation
flutter --version

# Ensure desktop support is enabled
flutter config --enable-macos-desktop   # or linux/windows
```

### Step 2: Set Up Project

```bash
cd app/flutter
flutter pub get
flutter create --platforms=macos,linux,windows .  # If needed
```

### Step 3: Verify Native Library

```bash
# macOS
ls -lh ../../C/macos/libdittoffi.dylib
otool -L ../../C/macos/libdittoffi.dylib

# Linux
ls -lh ../../C/linux/libdittoffi.so
ldd ../../C/linux/libdittoffi.so

# Windows
dir ..\..\C\windows\dittoffi.dll
```

### Step 4: Start Coding!

Create your implementation:
1. `lib/native/ditto_loader.dart`
2. `lib/native/ditto_bindings.dart`
3. `lib/main.dart`

### Step 5: Test

```bash
flutter run -d macos  # or linux/windows
```

## 💡 Hints & Tips

### Memory Management
- Always use try-finally for allocations
- `calloc` allocates, `calloc.free()` deallocates
- Strings from Dart need `toNativeUtf8()` and must be freed
- Consider using a `Finalizer` as a safety net

### Buffer Resizing (Get Operation)
```dart
// First call - get size
sizePtr.value = 0;
int rc = _get(handle, keyPtr, nullptr, sizePtr);

if (rc == 1) {
  // Allocate buffer with required size
  final bufPtr = calloc<Uint8>(sizePtr.value);
  // Second call - retrieve data
  rc = _get(handle, keyPtr, bufPtr, sizePtr);
}
```

### Callbacks
- Must be static functions (no closures)
- Use `Pointer.fromFunction<T>(staticFunction)`
- Registry pattern to route to instance methods
- `scheduleMicrotask()` for thread safety

### Common Pitfalls
- ❌ Forgetting to free allocated memory
- ❌ Using instance methods as callbacks (use static + registry)
- ❌ Not handling the two-step get operation
- ❌ Ignoring error codes
- ❌ Not thread-safe callback delivery

## 📖 Resources

### Official Documentation
- [Dart FFI Documentation](https://dart.dev/guides/libraries/c-interop)
- [package:ffi API Reference](https://pub.dev/packages/ffi)
- [Flutter Desktop Support](https://docs.flutter.dev/desktop)

### Example Code Patterns
Check `C/include/ditto.h` for:
- Function signatures
- Expected behavior
- Error codes
- Thread safety notes

## 🏆 Stretch Goals (Optional)

If you finish early, consider:

1. **Better error handling**
   - Custom exception types
   - Contextual error messages
   - Retry logic

2. **Performance optimizations**
   - Connection pooling
   - Caching layer
   - Batch operations

3. **Enhanced UI**
   - Search/filter functionality
   - Export/import data
   - Hex viewer for binary values
   - Dark mode

4. **Testing**
   - Unit tests for bindings
   - Widget tests for UI
   - Integration tests

5. **Production polish**
   - App icon
   - Better styling
   - Loading states
   - Keyboard shortcuts

## 📝 Submission

When complete, provide:

1. **Your code** (`lib/` directory)
2. **Brief notes** on:
   - Challenges faced
   - Design decisions
   - What you'd improve with more time

3. **Demo** (if possible):
   - Screenshot or screen recording showing the app working

## ❓ Questions?

If you get stuck:
1. Check `C/include/ditto.h` for API details
2. Review the resources section
3. Make reasonable assumptions and document them
4. Focus on getting core functionality working first

---

**Good luck! We're excited to see your solution! 🚀**
