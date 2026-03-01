#pragma once

/**
 * Cross-platform frameless window factory
 * Similar to Python's qframelesswindow/__init__.py
 *
 * NOTE: This header provides type aliases for platform-specific implementations.
 * The actual classes are defined in platform-specific headers which already
 * use the qfw namespace. We DO NOT wrap these in namespace qfw here because
 * this header may be included from within an existing qfw namespace block.
 */

#include <QtGlobal>

#ifdef Q_OS_WIN
#include "windows_frameless_window.h"
#include "windows_window_effect.h"
// Type aliases - classes are already in qfw namespace from included headers
// These using declarations work whether we're inside qfw namespace or not
using FramelessWindow = ::qfw::WindowsFramelessWindow;
using FramelessMainWindow = ::qfw::WindowsFramelessMainWindow;
using FramelessDialog = ::qfw::WindowsFramelessDialog;
using AcrylicWindow = ::qfw::WindowsFramelessWindow;
using WindowEffect = ::qfw::WindowsWindowEffect;
#elif defined(Q_OS_MAC)
#include "mac_frameless_window.h"
#include "mac_window_effect.h"
using FramelessWindow = ::qfw::MacFramelessWindow;
using FramelessMainWindow = ::qfw::MacFramelessMainWindow;
using FramelessDialog = ::qfw::MacFramelessDialog;
using AcrylicWindow = ::qfw::MacAcrylicWindow;
using WindowEffect = ::qfw::MacWindowEffect;
#else
#include "linux_frameless_window.h"
using FramelessWindow = ::qfw::LinuxFramelessWindow;
using FramelessMainWindow = ::qfw::LinuxFramelessMainWindow;
using FramelessDialog = ::qfw::LinuxFramelessDialog;
using AcrylicWindow = ::qfw::LinuxFramelessWindow;
using WindowEffect = ::qfw::LinuxWindowEffect;
#endif
