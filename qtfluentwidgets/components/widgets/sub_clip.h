#pragma once

#include <QWidget>
#include <QPushButton>
#include "common/qtcompat.h"

class QLabel;
class QHBoxLayout;

namespace qfw {
class SubClipCloseButton : public QPushButton {
    Q_OBJECT
public:
    explicit SubClipCloseButton(QWidget* parent = nullptr);
protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(enterEvent_QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
private:
    bool hovered_ = false;
};

// A compact removable token; the application decides what to do on closed().
class SubClip : public QWidget {
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText)
public:
    explicit SubClip(const QString& text, QWidget* parent = nullptr);
    QString text() const;
    void setText(const QString& text);
    QLabel* textLabel() const { return textLabel_; }
    SubClipCloseButton* closeButton() const { return closeButton_; }
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
signals:
    void closed(const QString& text);
protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void changeEvent(QEvent* event) override;
private:
    void setPressed(bool pressed);
    QLabel* textLabel_;
    SubClipCloseButton* closeButton_;
    QHBoxLayout* layout_;
};
} // namespace qfw
