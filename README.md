# Workpiece Inspector

Workpiece Inspector is a C++/Qt upper-computer portfolio project for a simulated workpiece inspection cell. The current stage includes Task 001 project skeleton, Task 002 core domain models, Task 003 configuration, Task 004 simulated image source, and Task 005 optional network camera source.

The application is intentionally minimal at this stage. Shared domain models, validated configuration, deterministic generated workpiece images, and an optional OpenCV-backed network camera adapter exist for later modules, but inspection workflow, image processing, persistence, and charts will be added only after their tasks and specs are written.

## Requirements

- CMake 3.21 or newer
- C++17 compiler
- Qt 6 with Widgets and Test modules

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

## Windows Notes

The project uses CMake and Qt 6 so it can later be checked out and built on Windows with a matching Qt installation. Windows validation is not part of Task 001.

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
