<p align="center">
  <img width="18%" align="center" src="qtfluentwidgets\resources\images\logo.png" alt="logo">
</p>
  <h1 align="center">
  Qt-Fluent-Widgets
</h1>
<p align="center">
  A fluent design widgets library based on <a href="https://github.com/zhiyiYo/PyQt-Fluent-Widgets">PyQt-Fluent-Widgets</a>
</p>

<div align="center">

[![Version](https://img.shields.io/badge/Version-1.0.0-blue.svg)](https://github.com/Fairy-Oracle-Sanctuary/Qt-Fluent-Widgets)
[![GPLv3](https://img.shields.io/badge/License-GPLv3-blue?color=#4ec820)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20macOS-blue?color=#4ec820)]()
[![Qt6](https://img.shields.io/badge/Qt-6.3+-green.svg)](https://www.qt.io)

</div>

<p align="center">
English | <a href="docs/README_zh.md">简体中文</a>
</p>

<p align="center">
  <img src="docs\source\_static\Interface_en.png" alt="interface"/>
</p>

## 📖 Introduction

Qt-Fluent-Widgets is a **C++ native port** of the popular [PyQt-Fluent-Widgets](https://github.com/zhiyiYo/PyQt-Fluent-Widgets) library by **zhiyiYo**. It provides a comprehensive set of modern Fluent Design UI widgets for Qt6 applications.

This library aims to bring the beautiful Fluent Design System to native C++ Qt applications, offering:
- 🎨 **Fluent Design aesthetics** - Acrylic, Mica, and modern styling
- 🧩 **Rich widget collection** - Buttons, menus, dialogs, navigation, and more
- ⚡ **High performance** - Native C++ implementation
- 🔧 **Easy integration** - Static library that links directly to your Qt project

## ✨ Features

| Category | Widgets |
|----------|---------|
| **Buttons** | PushButton, ToolButton, RadioButton, CheckBox, ToggleButton, SplitButton |
| **Input** | LineEdit, ComboBox, SpinBox, DoubleSpinBox, TextEdit |
| **Dialogs** | MessageBox, ColorDialog, Flyout, TeachingTip |
| **Navigation** | NavigationView, BreadcrumbBar, Pivot, SegmentedWidget, TabBar |
| **Status** | InfoBar, ProgressBar, ProgressRing, StateToolTip, ToolTip, InfoBadge |
| **Menus** | RoundMenu, CommandBar, CheckableMenu |
| **Material** | Acrylic, Mica effect (Windows 11) |
| **Layout** | FlowLayout with animation support |

## 📋 Requirements

- **Qt 6.3+**
  - Windows: Qt 6.3+ recommended
  - macOS: Qt 6.9.0 recommended
- **CMake 3.16+**
- **C++17 compiler**
  - MSVC 2019+ (Windows)
  - Clang (macOS, via Xcode or Command Line Tools)

## 🚀 Quick Start

### 1. Clone the Repository

```bash
git clone https://github.com/Fairy-Oracle-Sanctuary/Qt-Fluent-Widgets.git
cd Qt-Fluent-Widgets
```

### 2. Build the Library

```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

### 3. Integrate into Your Project

Add to your `CMakeLists.txt`:

```cmake
add_subdirectory(Qt-Fluent-Widgets)

target_link_libraries(your_app PRIVATE
    qtfluentwidgets
    Qt6::Widgets
    Qt6::Svg
)
```

### 4. Basic Usage

```cpp
#include <QApplication>
#include <qtfluentwidgets.h>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // Set theme (Light, Dark, or Auto)
    qfw::setTheme(qfw::Theme::Light);
    
    // Create a fluent window
    qfw::FluentWindow window;
    window.setWindowTitle("My Fluent App");
    window.resize(800, 600);
    window.show();
    
    return app.exec();
}
```

## 🎯 Gallery Application

The repository includes a demo gallery application showcasing all widgets:

```bash
cd build
./app/qtfluentwidgets_app  # Linux/macOS
qtfluentwidgets_app.exe     # Windows
```

## 🌐 Supported Platforms

| Platform | Status | Notes |
|----------|--------|-------|
| Windows | ✅ Full support | Acrylic/Mica effects, frameless window |
| macOS | ✅ Full support | Frameless window with native Cocoa integration |
| Linux | 🚧 Placeholder | Frameless window not yet implemented |

## 📝 License

This project is licensed under **GPLv3** - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- **zhiyiYo** - Creator of the original [PyQt-Fluent-Widgets](https://github.com/zhiyiYo/PyQt-Fluent-Widgets) library
- **zhiyiYo** - [Official documentation & demos](https://qfluentwidgets.com/) (Python version)
- Microsoft - Fluent Design System inspiration
- Qt Framework - The foundation for cross-platform UI development

> **Note**: This project is a C++ implementation referenced from the Python version of PyQt-Fluent-Widgets. The original author offers a commercial C++ version, but this project was independently developed by studying the open-source Python codebase.

## 🤝 Contributing

Contributions are welcome! Please feel free to submit issues and pull requests.

## 🏆 Contributors

<a href="https://github.com/Fairy-Oracle-Sanctuary/Qt-Fluent-Widgets/graphs/contributors">
  <img src="https://contrib.rocks/image?repo=Fairy-Oracle-Sanctuary/Qt-Fluent-Widgets" />
</a>
