#pragma once

/**
 * Cross-platform frameless window factory
 * Similar to Python's qframelesswindow/__init__.py
 */

#include <QtGlobal>

#ifdef Q_OS_WIN
#include "windows_frameless_window.h"
#include "windows_window_effect.h"
namespace qfw {
using FramelessWindow = WindowsFramelessWindow;
using FramelessMainWindow = WindowsFramelessMainWindow;
using FramelessDialog = WindowsFramelessDialog;
using AcrylicWindow = WindowsFramelessWindow;
using WindowEffect = WindowsWindowEffect;
}  // namespace qfw
#elif defined(Q_OS_MAC)
// macOS implementations
#include "mac_frameless_window.h"
#include "mac_window_effect.h"
namespace qfw {
using FramelessWindow = MacFramelessWindow;
using FramelessMainWindow = MacFramelessMainWindow;
using FramelessDialog = MacFramelessDialog;
using AcrylicWindow = MacAcrylicWindow;
using WindowEffect = MacWindowEffect;
}  // namespace qfw
#else
// Linux implementations (to be implemented)
#include "linux_frameless_window.h"
namespace qfw {
using FramelessWindow = LinuxFramelessWindow;
using FramelessMainWindow = LinuxFramelessMainWindow;
using FramelessDialog = LinuxFramelessDialog;
using AcrylicWindow = LinuxFramelessWindow;
using WindowEffect = LinuxWindowEffect;
}  // namespace qfw
#endif
