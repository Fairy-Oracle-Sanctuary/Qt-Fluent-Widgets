#pragma once

#include <QDialog>
#include <QMainWindow>
#include <QPaintEvent>
#include <QWidget>

namespace qfw {

class WindowsWindowEffect;

class WindowsFramelessWindowBase {
public:
    static constexpr int BORDER_WIDTH = 5;

    WindowsFramelessWindowBase();
    virtual ~WindowsFramelessWindowBase();

    void setResizeEnabled(bool enabled);
    bool isResizeEnabled() const;

protected:
    void initFrameless(QWidget* window);

    bool handleNativeEvent(QWidget* window, void* message, qintptr* result);

    WindowsWindowEffect* windowEffect_ = nullptr;
    bool resizeEnabled_ = true;
};

class WindowsFramelessWindow : public QWidget, public WindowsFramelessWindowBase {
    Q_OBJECT

public:
    explicit WindowsFramelessWindow(QWidget* parent = nullptr);

protected:
    bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override;
    void showEvent(QShowEvent* e) override;
    void paintEvent(QPaintEvent* e) override;

private:
    bool effectsApplied_ = false;
};

class WindowsFramelessMainWindow : public QMainWindow, public WindowsFramelessWindowBase {
    Q_OBJECT

public:
    explicit WindowsFramelessMainWindow(QWidget* parent = nullptr);

protected:
    bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override;
    void showEvent(QShowEvent* e) override;

private:
    bool effectsApplied_ = false;
};

class WindowsFramelessDialog : public QDialog, public WindowsFramelessWindowBase {
    Q_OBJECT

public:
    explicit WindowsFramelessDialog(QWidget* parent = nullptr);

protected:
    bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override;
    void showEvent(QShowEvent* e) override;

private:
    bool effectsApplied_ = false;
};

}  // namespace qfw
