#pragma once

#include <QIcon>
#include <QPointer>
#include <QSize>
#include <QVariant>
#include <QWidget>

class QEvent;
class QGraphicsDropShadowEffect;
class QPaintEvent;
class QResizeEvent;

namespace qfw {

class FluentIconBase;
class IconWidget;

/**
 * @brief Splash screen
 *
 * Shows a centered icon (with an optional drop shadow) on a solid background
 * while the application is starting up. The widget installs an event filter on
 * its parent so it always covers the whole window.
 *
 * Ported 1:1 from the Python implementation in
 * `libs/qfluentwidgets_pro/window/splash_screen.py`.
 */
class SplashScreen : public QWidget {
    Q_OBJECT

public:
    explicit SplashScreen(const QIcon& icon = QIcon(), QWidget* parent = nullptr,
                          bool enableShadow = true);
    explicit SplashScreen(const QString& icon, QWidget* parent = nullptr, bool enableShadow = true);
    explicit SplashScreen(const FluentIconBase& icon, QWidget* parent = nullptr,
                          bool enableShadow = true);

    void setIcon(const QIcon& icon);
    void setIcon(const QString& icon);
    void setIcon(const FluentIconBase& icon);
    QIcon icon() const;

    void setIconSize(const QSize& size);
    QSize iconSize() const;

    void setTitleBar(QWidget* titleBar);
    QWidget* titleBar() const;

    /** Close the splash screen. */
    void finish();

protected:
    bool eventFilter(QObject* obj, QEvent* e) override;
    void resizeEvent(QResizeEvent* e) override;
    void paintEvent(QPaintEvent* e) override;

private:
    void initialize(const QVariant& icon, bool enableShadow);
    void setup(bool enableShadow);

    QVariant icon_;
    QSize iconSize_ = QSize(96, 96);
    QPointer<QWidget> titleBar_;
    QPointer<IconWidget> iconWidget_;
    QPointer<QGraphicsDropShadowEffect> shadowEffect_;
};

}  // namespace qfw
