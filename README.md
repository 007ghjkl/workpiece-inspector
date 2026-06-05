# Workpiece Inspector

Workpiece Inspector is a C++/Qt upper-computer portfolio project for a simulated workpiece inspection cell. The current stage includes Task 001 project skeleton through Task 012 main Qt Widgets UI.

The application is intentionally minimal at this stage. Shared domain models, validated configuration, deterministic generated workpiece images, an optional OpenCV-backed network camera adapter, metadata-based rule inspection, deterministic motion simulation, high-level communication simulation, local SQLite persistence, controlled image file storage, synchronous single-cycle workflow orchestration, and a main Qt Widgets HMI exist for later modules. History charts will be added only after their task and spec are written.

## Requirements

- CMake 3.21 or newer
- C++17 compiler
- Qt 6 with Widgets and Test modules
- Qt 6 SQLite SQL driver

The current WSL environment has been checked with Qt 6.11.1.

## Configure

```sh
cmake -S . -B build
```

## Build

```sh
cmake --build build
```

## Test

```sh
ctest --test-dir build --output-on-failure
```

## Run

```sh
./build/workpiece-inspector
```

If running inside WSL without a display server, the executable may build successfully but not open a visible window. Automated tests for Task 001 do not require a display server.

## Manual UI Test

1. Launch `./build/workpiece-inspector`.
2. Confirm the top bar shows `SIMULATION MODE`.
3. Click `Start Cycle`.
4. Confirm the station state changes to `completed`.
5. Confirm the image area, latest result, motion status, communication status, and message area update.

## Windows Notes

The project uses CMake and Qt 6 and has been checked out and tested by the user on Windows during Tasks 001-009. Windows builds still require a matching local Qt/CMake setup and, for network camera support, an OpenCV installation discoverable by CMake.

## Optional OpenCV Notes

Task 005 treats OpenCV as optional. If CMake finds OpenCV components `core`, `videoio`, and `imgproc`, `NetworkCameraImageSource` uses OpenCV `VideoCapture`. If OpenCV is not found, the project still builds and the network camera source reports a clear fault at runtime.

Manual network stream validation is only needed when a real stream is available. Use a config value like:

```json
{
  "imageSource": {
    "mode": "network_camera",
    "networkCameraUrl": "rtsp://example.com/stream"
  }
}
```
