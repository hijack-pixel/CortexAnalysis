# CortexAnalysis

CortexAnalysis is a Windows desktop application for mouse cortex imaging analysis. It is built with C++ and Qt Widgets, with a small QML status indicator, and uses an external Octave-based analysis runtime for the core computation.

The application focuses on experiment workflow management: selecting imaging data, configuring analysis parameters, launching analysis tasks, tracking progress, viewing logs, previewing generated results, and exporting selected output files.

## Features

- Six-step cortex imaging analysis workflow:
  - image registration and ROI time-series analysis
  - connectivity matrix and network graph generation
  - power spectrum analysis
  - time correlation map analysis
  - ROI connectivity analysis
  - signal-to-noise analysis
- TIF/MAT data selection and validation.
- Automatic grouping of mouse TIF files by filename prefix.
- Manual registration point input and TXT-based coordinate import.
- JSON configuration generation for the external analysis runtime.
- External task execution through `QProcess`.
- Real-time progress updates through `QFileSystemWatcher` and per-step `progress.txt` files.
- Per-step log display and log export.
- Result preview for images, EMF files, Excel files, and other generated artifacts.
- Batch export of selected result files with conflict handling and progress feedback.

## Tech Stack

- C++
- Qt Widgets
- QML
- JSON
- Octave runtime integration
- qmake
- Windows

## Architecture

The project uses a GUI frontend plus external analysis backend architecture.

```text
Qt desktop application
        |
        | writes analysis parameters
        v
data/config.json
        |
        | starts external task
        v
data/exec.bat <step>
        |
        | runs analysis runtime
        v
data/MyExecutable.exe + octave/
        |
        | writes progress and results
        v
data/stepN/progress.txt
data/stepN/*
```

The Qt application is responsible for interaction and workflow control. The external runtime reads `config.json`, executes the selected analysis step, writes progress values to `progress.txt`, and outputs images, tables, MAT files, EPS files, or other artifacts to the corresponding `data/stepN` directory.

## Project Structure

```text
.
|-- MouseAnalysis.pro          # qmake project file
|-- main.cpp                   # application entry point
|-- mainwindow.h/.cpp/.ui      # main UI and workflow logic
|-- assets.qrc                 # Qt resource file
|-- icon/                      # application and step icons
|-- qml/                       # QML status indicator
|-- qss/                       # Qt stylesheet
|-- component/
|   |-- commandprocess.*       # QProcess wrapper for external tasks
|   |-- configsaver.*          # JSON configuration persistence
|   |-- filelistwidget.*       # result file list and export entry
|   |-- mygraphicsview.*       # image/EMF preview
|   |-- tablewidget.*          # Excel preview
|   |-- copyworker.*           # asynchronous result export
|   |-- csvparser.*            # coordinate TXT/CSV parser
|   |-- libqemf/               # EMF rendering support
|   |-- xlnt/                  # Excel parsing dependency
|   `-- FramelessHelper/       # frameless window helper library
```

## Runtime Package Layout

The source code expects the deployed executable to run beside the analysis runtime files:

```text
CortexAnalysis.exe
data/
|-- exec.bat
|-- MyExecutable.exe
|-- config.json
|-- step1/
|-- step2/
|-- step3/
|-- step4/
|-- step5/
|-- step6/
`-- Toolbox4PYCM/
octave/
```

`data/` and `octave/` are runtime dependencies. They are not the main Qt source code, but the application needs them for full analysis execution.

## Build

This project is configured with qmake.

1. Open `MouseAnalysis.pro` in Qt Creator.
2. Select a Windows Qt kit compatible with the project.
3. Build the project.
4. Place the generated executable beside the required `data/` and `octave/` runtime folders.

The qmake target name is:

```text
CortexAnalysis
```

## Notes

- The UI is mainly implemented with Qt Widgets. QML is used for the small breathing-light status indicator.
- Long-running analysis is executed as an external process to avoid blocking the UI thread.
- Image thumbnail loading, directory size calculation, and file export are handled asynchronously.
- Large Excel files are opened through the system application instead of being fully rendered in the embedded table preview.

