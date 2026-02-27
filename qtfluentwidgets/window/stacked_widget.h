#pragma once

#include <QFrame>
#include <QPointer>

class QHBoxLayout;
class QStackedWidget;

namespace qfw {

class PopUpAniStackedWidget;

class StackedWidget : public QFrame {
    Q_OBJECT

public:
    explicit StackedWidget(QWidget* parent = nullptr);

    bool isAnimationEnabled() const;
    void setAnimationEnabled(bool enabled);

    void addWidget(QWidget* widget);
    void removeWidget(QWidget* widget);

    QWidget* widget(int index) const;

    void setCurrentWidget(QWidget* widget, bool popOut = true);
    void setCurrentIndex(int index, bool popOut = true);

    int currentIndex() const;
    QWidget* currentWidget() const;

    int indexOf(QWidget* widget) const;
    int count() const;

    QStackedWidget* view() const;

signals:
    void currentChanged(int index);

private:
    QPointer<QHBoxLayout> hBoxLayout_;
    QPointer<PopUpAniStackedWidget> view_;
};

}  // namespace qfw
