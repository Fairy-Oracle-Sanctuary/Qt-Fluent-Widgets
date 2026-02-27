#pragma once

#include <QListWidget>
#include <QSize>
#include <QTime>
#include <QToolButton>

#include "common/icon.h"

namespace qfw {

class SmoothScrollBar;

class ScrollButton : public QToolButton {
    Q_OBJECT

public:
    explicit ScrollButton(FluentIconEnum icon, QWidget* parent = nullptr);

protected:
    bool eventFilter(QObject* obj, QEvent* e) override;
    void paintEvent(QPaintEvent* e) override;

private:
    FluentIconEnum icon_;
    bool isPressed_ = false;
};

class CycleListWidget : public QListWidget {
    Q_OBJECT

public:
    explicit CycleListWidget(const QStringList& items, const QSize& itemSize,
                             Qt::Alignment align = Qt::AlignCenter, QWidget* parent = nullptr);

    void setItems(const QStringList& items);
    void setSelectedItem(const QString& text);

    int visibleNumber() const;
    void setVisibleNumber(int n);

    int currentIndex() const;
    QListWidgetItem* currentItem() const;

    void setCurrentIndex(int index);

    void setScrollButtonRepeatEnabled(bool enabled);
    bool isScrollButtonRepeatEnabled() const;

    QSize itemSize() const;

public slots:
    void scrollDown();
    void scrollUp();

signals:
    void cycleCurrentItemChanged(QListWidgetItem* item);

protected:
    void wheelEvent(QWheelEvent* e) override;
    void enterEvent(QEnterEvent* e) override;
    void leaveEvent(QEvent* e) override;
    void resizeEvent(QResizeEvent* e) override;
    bool eventFilter(QObject* obj, QEvent* e) override;

private:
    void createItems(const QStringList& items);
    void addColumnItems(const QStringList& items, bool disabled = false);
    void scrollToItemInternal(QListWidgetItem* item);
    void onItemClicked(QListWidgetItem* item);

    void setButtonsVisible(bool visible);
    void scrollWithAnimation(int index);

    QSize itemSize_;
    Qt::Alignment align_;

    ScrollButton* upButton_ = nullptr;
    ScrollButton* downButton_ = nullptr;

    SmoothScrollBar* vScrollBar_ = nullptr;

    int scrollDuration_ = 250;
    QStringList originItems_;

    int visibleNumber_ = 9;
    bool isCycle_ = false;
    int currentIndex_ = 0;

    QTime lastScrollTime_;
    bool scrollButtonRepeatEnabled_ = false;
};

}  // namespace qfw
