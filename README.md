# Workpiece Inspector

Workpiece Inspector is a C++/Qt upper-computer portfolio project for a simulated workpiece inspection cell. The MVP currently includes Task 001 project skeleton through Task 014 documentation and review.

The application demonstrates a complete software workflow without requiring industrial hardware: deterministic generated workpiece images, optional OpenCV-backed network camera capture, metadata-based rule inspection, deterministic motion simulation, high-level communication simulation, local SQLite persistence, controlled image file storage, synchronous single-cycle workflow orchestration, a Qt Widgets HMI, and persisted history/statistics display.

This is a simulation-first MVP. It does not claim real industrial camera calibration, real motion-control accuracy, real PLC deployment, or production-grade defect measurement.

## Requirements

- CMake 3.21 or newer
- C++17 compiler
- Qt 6 with Charts, Core, Gui, Sql, Widgets, and Test modules
- Qt 6 SQLite SQL driver
- Optional: OpenCV with `core`, `videoio`, and `imgproc` components for network camera support

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

The current automated suite contains tests for:

- project smoke startup
- core domain models
- configuration
- simulated image source
- optional network camera source failure paths
- rule-based inspector
- motion simulation
- communication simulation
- SQLite persistence
- image storage
- workflow controller
- main Qt Widgets UI

## Run

```sh
./build/workpiece-inspector
```

If running inside WSL without a display server, the executable may build successfully but not open a visible window. Automated UI testing uses Qt's offscreen platform where configured.

## Manual UI Test

1. Launch `./build/workpiece-inspector`.
2. Confirm the top bar shows `SIMULATION MODE`.
3. Click `Start Cycle`.
4. Confirm the station state changes to `completed`.
5. Confirm the image area, latest result, motion status, communication status, and message area update.
6. Confirm the history table, total/pass/fail counts, pass rate, and pass/fail chart update.

## Demo Notes

- Use the default simulated image source for reproducible demos and tests.
- The same simulation seed and configuration should produce repeatable generated workpiece cases.
- `Start Cycle` runs one synchronous inspection cycle in the current MVP.
- The UI displays simulated device status, inspection output, motion state, communication state, persisted history, summary counts, pass rate, and a pass/fail chart.
- Saved images are local PNG files when image storage is enabled; SQLite stores image references, not image blobs.

## Simulation Assumptions And Limitations

- Workpieces, defects, positioning offsets, motion behavior, and communication state are simulated.
- Rule-based inspection uses generated frame metadata for the MVP; it is not a calibrated pixel-measurement algorithm.
- Motion simulation reports deterministic target/actual values but does not model real controller timing, acceleration, or hardware errors.
- Communication simulation exposes high-level connection, polling, command, and fault state; it does not expose or implement a real register table.
- Optional network camera support is an adapter path only. Simulation remains the baseline because network streams and OpenCV backends vary by platform.
- The MVP stores local SQLite records and local image files only. It has no authentication, cloud upload, report export, real PLC integration, or production deployment workflow.

## WSL Notes

- The primary implementation and automated validation were performed in the current WSL workspace.
- GUI display from WSL depends on the local display server setup. Use the automated offscreen UI test to validate widget construction and workflow binding when a visible display is unavailable.
- OpenCV is optional at configure time. If it is missing, the project still builds and network camera mode reports a clear runtime fault.

## Windows Notes

The project uses CMake and Qt 6 and has been checked out and tested by the user on Windows during MVP development through Task 013. Windows builds still require a matching local Qt/CMake setup and, for network camera support, an OpenCV installation discoverable by CMake.

Windows validation is user-performed from a Windows checkout. The WSL test run does not prove that a separate Windows Qt/OpenCV installation is correctly configured.

Known Windows-sensitive areas that have explicit fixes/tests in this MVP:

- Qt debug builds for QImage-heavy tests require a Qt application object.
- Simulated image generation uses balanced `QPainter` save/restore lifecycle.
- Network camera OpenCV ownership is hidden behind a private handle.
- Persistence history filters avoid Qt-owning fields in the public filter structure.
- Image storage uses Qt path APIs rather than native separator string prefix checks.

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
