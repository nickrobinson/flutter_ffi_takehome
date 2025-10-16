# macOS Ditto Library

This directory contains the pre-built Ditto FFI library for macOS.

## File

- `libdittoffi.dylib` - macOS dynamic library (ARM64 + x86_64)

## Requirements

- macOS 10.15 (Catalina) or later
- Xcode Command Line Tools (for development)

## Usage with Flutter

The Flutter app will automatically load this library when running on macOS. No additional configuration needed.

```bash
cd ../../app/flutter
flutter run -d macos
```

## Verifying the Library

Check library information:

```bash
# Show file size and permissions
ls -lh libdittoffi.dylib

# Show dependencies
otool -L libdittoffi.dylib

# Show exported symbols
nm -g libdittoffi.dylib | grep ditto_
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

### "Library not loaded" error

Ensure the file exists and has proper permissions:
```bash
chmod 755 libdittoffi.dylib
```

### Security warning: "cannot be opened because the developer cannot be verified"

For development purposes, you can bypass this:
```bash
xattr -d com.apple.quarantine libdittoffi.dylib
```

**Note**: This is only for local development. Production apps should be properly signed.

---

**Library Version**: 1.0.0
**Built with**: Xcode AppleClang, C11 standard
**Thread Safety**: pthread mutexes
