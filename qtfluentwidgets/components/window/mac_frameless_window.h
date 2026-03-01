#pragma once

/**
 * macOS Frameless Window - Placeholder
 * Full implementation needed for macOS support
 */

#include <QDialog>
#include <QMainWindow>
#include <QShowEvent>
#include <QWidget>

namespace qfw {

class MacFramelessWindowBase {
public:
    MacFramelessWindowBase();
    virtual ~MacFramelessWindowBase();

    void setResizeEnabled(bool enabled);
    bool isResizeEnabled() const;

protected:
    void initFrameless(QWidget* window);
    void applyCocoaWindowStyle(QWidget* window);

    bool resizeEnabled_ = true;
};

class MacFramelessWindow : public QWidget, public MacFramelessWindowBase {
    Q_OBJECT
public:
    explicit MacFramelessWindow(QWidget* parent = nullptr);

protected:
    void showEvent(QShowEvent* e) override;

private:
    bool cocoaApplied_ = false;
};

class MacFramelessMainWindow : public QMainWindow, public MacFramelessWindowBase {
    Q_OBJECT
public:
    explicit MacFramelessMainWindow(QWidget* parent = nullptr);

protected:
    void showEvent(QShowEvent* e) override;
    bool event(QEvent* e) override;

private:
    bool cocoaApplied_ = false;
};

class MacFramelessDialog : public QDialog, public MacFramelessWindowBase {
    Q_OBJECT
public:
    explicit MacFramelessDialog(QWidget* parent = nullptr);

protected:
    void showEvent(QShowEvent* e) override;

private:
    bool cocoaApplied_ = false;
};

class MacWindowEffect {
public:
    static void setAcrylicEffect(QWidget* widget) { Q_UNUSED(widget) }
    static void setMicaEffect(QWidget* widget) { Q_UNUSED(widget) }
};

} // namespace qfw
