#pragma once

#include <QGraphicsOpacityEffect>
#include <QLabel>
#include <QPoint>
#include <QPropertyAnimation>
#include <QTimer>
#include <QToolButton>
#include <QWidget>

namespace qfw {

class StateCloseButton : public QToolButton {
    Q_OBJECT

public:
    explicit StateCloseButton(QWidget* parent = nullptr);

protected:
    void enterEvent(QEnterEvent* e) override;
    void leaveEvent(QEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void paintEvent(QPaintEvent* e) override;

private:
    bool isPressed_ = false;
    bool isEnter_ = false;
};

class StateToolTip : public QWidget {
    Q_OBJECT

public:
    explicit StateToolTip(const QString& title, const QString& content, QWidget* parent = nullptr);

    QString title() const;
    void setTitle(const QString& title);

    QString content() const;
    void setContent(const QString& content);

    void setState(bool isDone);
    bool isDone() const;

    QPoint getSuitablePos();

signals:
    void closedSignal();

protected:
    void paintEvent(QPaintEvent* e) override;

private:
    void initUi();
    void initLayout();
    void setQss();
    void fadeOut();
    void onRotateTimerTimeout();
    void onCloseButtonClicked();

    QString title_;
    QString content_;

    QLabel* titleLabel_ = nullptr;
    QLabel* contentLabel_ = nullptr;
    QTimer* rotateTimer_ = nullptr;
    QGraphicsOpacityEffect* opacityEffect_ = nullptr;
    QPropertyAnimation* opacityAnimation_ = nullptr;
    StateCloseButton* closeButton_ = nullptr;

    bool isDone_ = false;
    int rotateAngle_ = 0;
    int deltaAngle_ = 20;
};

}  // namespace qfw
