# Workpiece Inspector

Workpiece Inspector is a C++/Qt upper-computer portfolio project for a simulated workpiece inspection cell. The current stage includes Task 001 project skeleton and Task 002 core domain models.

The application is intentionally minimal at this stage. Shared domain models exist for later modules, but inspection workflow, simulated devices, image processing, persistence, and charts will be added only after their tasks and specs are written.

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
