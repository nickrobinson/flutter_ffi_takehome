# Linux Ditto Library

This directory contains the pre-built Ditto FFI library for Linux.

## File

- `libdittoffi.so` - Linux shared object (x86_64)

## Requirements

- Linux kernel 3.2 or later
- glibc 2.17 or later
- pthread support (included in most distributions)

## Usage with Flutter

The Flutter app will automatically load this library when running on Linux. No additional configuration needed.

```bash
cd ../../app/flutter
flutter run -d linux
```

## Verifying the Library

Check library information:

```bash
# Show file size and permissions
ls -lh libdittoffi.so

# Show dependencies
ldd libdittoffi.so

# Show exported symbols
nm -D libdittoffi.so | grep ditto_
```

Expected symbols:
- `ditto_open`
- `ditto_close`
- `ditto_put`
- `ditto_get`
- `ditto_delete`
- `ditto_subscribe`
- `ditto_unsubscribe`
- `ditto_version`

## Rebuilding

If you need to rebuild the library:

```bash
cd ..
./build.sh
```

See `../BUILD.md` for detailed build instructions.

## Troubleshooting

### "error while loading shared libraries: libdittoffi.so: cannot open shared object file"

The library is not in the system's library path. Options:

1. **Use LD_LIBRARY_PATH** (temporary):
   ```bash
   export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)
   ```

2. **Add to ldconfig** (permanent, requires root):
   ```bash
   sudo cp libdittoffi.so /usr/local/lib/
   sudo ldconfig
   ```

3. **Let Flutter handle it** (recommended):
   The Flutter FFI loader will find it automatically in this directory.

### Permission denied

Ensure the file has execute permissions:
```bash
chmod 755 libdittoffi.so
```

---

**Library Version**: 1.0.0
**Built with**: GCC/Clang, C11 standard
**Thread Safety**: pthread mutexes
**Dependencies**: glibc, pthread
