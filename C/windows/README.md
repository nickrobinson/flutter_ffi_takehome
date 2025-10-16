# Windows Ditto Library

This directory contains the pre-built Ditto FFI library for Windows.

## File

- `dittoffi.dll` - Windows dynamic-link library (x64)

## Requirements

- Windows 10 or later (x64)
- Visual C++ Redistributable 2015-2022 (usually pre-installed)

## Usage with Flutter

The Flutter app will automatically load this library when running on Windows. No additional configuration needed.

```cmd
cd ..\..\app\flutter
flutter run -d windows
```

## Verifying the Library

Check library information:

```cmd
# Show file size
dir dittoffi.dll

# Show dependencies (requires dumpbin from Visual Studio)
dumpbin /DEPENDENTS dittoffi.dll

# Show exported symbols
dumpbin /EXPORTS dittoffi.dll
```

Expected exports:
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

```cmd
cd ..
# Ensure you have CMake and Visual Studio installed
mkdir build
cd build
cmake -G "Visual Studio 16 2019" -A x64 ..
cmake --build . --config Release
cmake --install . --config Release
```

See `../BUILD.md` for detailed build instructions.

## Troubleshooting

### "The specified module could not be found"

The DLL or its dependencies are missing:

1. **Check DLL location**: Ensure `dittoffi.dll` is in this directory
2. **Install VC++ Redistributable**: Download from Microsoft's website
3. **Check dependencies**: Use [Dependencies.exe](https://github.com/lucasg/Dependencies) to inspect

### "Access is denied" or permission errors

Run your command prompt as Administrator, or adjust file permissions:
```cmd
icacls dittoffi.dll /grant Everyone:RX
```

### DLL blocked by Windows

If downloaded from the internet, Windows may block it:
```powershell
Unblock-File -Path dittoffi.dll
```

---

**Library Version**: 1.0.0
**Built with**: Visual Studio 2019+, C11 standard
**Thread Safety**: Windows threading primitives
**Dependencies**: MSVCRT, Kernel32
