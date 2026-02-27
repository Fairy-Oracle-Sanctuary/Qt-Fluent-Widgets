#pragma once

#include <QAbstractButton>
#include <QColor>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QWidget>

namespace qfw {

enum class TitleBarButtonState {
    Normal = 0,
    Hover = 1,
    Pressed = 2,
};

class TitleBarButton : public QAbstractButton {
    Q_OBJECT
    Q_PROPERTY(QColor normalColor READ normalColor WRITE setNormalColor)
    Q_PROPERTY(QColor hoverColor READ hoverColor WRITE setHoverColor)
    Q_PROPERTY(QColor pressedColor READ pressedColor WRITE setPressedColor)
    Q_PROPERTY(
        QColor normalBackgroundColor READ normalBackgroundColor WRITE setNormalBackgroundColor)
    Q_PROPERTY(QColor hoverBackgroundColor READ hoverBackgroundColor WRITE setHoverBackgroundColor)
    Q_PROPERTY(
        QColor pressedBackgroundColor READ pressedBackgroundColor WRITE setPressedBackgroundColor)

public:
    explicit TitleBarButton(QWidget* parent = nullptr);

    void setState(TitleBarButtonState state);
    TitleBarButtonState state() const;
    bool isPressed() const;

    QColor normalColor() const;
    QColor hoverColor() const;
    QColor pressedColor() const;
    QColor normalBackgroundColor() const;
    QColor hoverBackgroundColor() const;
    QColor pressedBackgroundColor() const;

    void setNormalColor(const QColor& color);
    void setHoverColor(const QColor& color);
    void setPressedColor(const QColor& color);
    void setNormalBackgroundColor(const QColor& color);
    void setHoverBackgroundColor(const QColor& color);
    void setPressedBackgroundColor(const QColor& color);

protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

    std::pair<QColor, QColor> getColors() const;

private:
    TitleBarButtonState state_ = TitleBarButtonState::Normal;

    QColor normalColor_;
    QColor hoverColor_;
    QColor pressedColor_;

    QColor normalBgColor_;
    QColor hoverBgColor_;
    QColor pressedBgColor_;
};

class MinimizeButton : public TitleBarButton {
    Q_OBJECT
public:
    explicit MinimizeButton(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
};

class MaximizeButton : public TitleBarButton {
    Q_OBJECT
public:
    explicit MaximizeButton(QWidget* parent = nullptr);

    void setMaximized(bool maximized);
    bool isMaximized() const;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    bool maximized_ = false;
};

class CloseButton : public TitleBarButton {
    Q_OBJECT
public:
    explicit CloseButton(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
};

class TitleBarBase : public QWidget {
    Q_OBJECT
public:
    explicit TitleBarBase(QWidget* parent);
    ~TitleBarBase() override;

    MinimizeButton* minimizeButton() const;
    MaximizeButton* maximizeButton() const;
    CloseButton* closeButton() const;

    bool isDoubleClickEnabled() const;
    void setDoubleClickEnabled(bool enabled);

    bool canDrag(const QPoint& pos) const;

protected:
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

    bool isDragRegion(const QPoint& pos) const;
    bool hasButtonPressed() const;

    MinimizeButton* minBtn_ = nullptr;
    MaximizeButton* maxBtn_ = nullptr;
    CloseButton* closeBtn_ = nullptr;

private:
    void updateButtonColors();
    void toggleMaximized();

    bool doubleClickEnabled_ = true;
};

class TitleBar : public TitleBarBase {
    Q_OBJECT
public:
    explicit TitleBar(QWidget* parent);

protected:
    QHBoxLayout* hBoxLayout() const;

private:
    QHBoxLayout* hBoxLayout_ = nullptr;
};

class StandardTitleBar : public TitleBar {
    Q_OBJECT
public:
    explicit StandardTitleBar(QWidget* parent);

    void setTitle(const QString& title);
    void setIcon(const QIcon& icon);

private:
    QLabel* iconLabel_ = nullptr;
    QLabel* titleLabel_ = nullptr;
};

}  // namespace qfw
