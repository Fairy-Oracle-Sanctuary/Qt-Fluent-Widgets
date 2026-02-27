#include "components/widgets/cycle_list_widget.h"

#include <QEasingCurve>
#include <QEnterEvent>
#include <QEvent>
#include <QPainter>
#include <QTime>
#include <QWheelEvent>

#include "common/icon.h"
#include "components/widgets/scroll_bar.h"

namespace qfw {

// ============================================================================
// ScrollButton
// ============================================================================

ScrollButton::ScrollButton(FluentIconEnum icon, QWidget* parent)
    : QToolButton(parent), icon_(icon) {
    setProperty("qssClass", QStringLiteral("ScrollButton"));
    installEventFilter(this);
}

bool ScrollButton::eventFilter(QObject* obj, QEvent* e) {
    if (obj == this) {
        if (e->type() == QEvent::MouseButtonPress) {
            isPressed_ = true;
            update();
        } else if (e->type() == QEvent::MouseButtonRelease) {
            isPressed_ = false;
            update();
        }
    }

    return QToolButton::eventFilter(obj, e);
}

void ScrollButton::paintEvent(QPaintEvent* e) {
    QToolButton::paintEvent(e);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);

    int w = isPressed_ ? 8 : 10;
    int h = isPressed_ ? 8 : 10;

    qreal x = (width() - w) / 2.0;
    qreal y = (height() - h) / 2.0;

    FluentIcon(icon_).render(
        &painter, QRect(qRound(x), qRound(y), w, h), Theme::Auto,
        isDarkTheme() ? QVariantMap{}
                      : QVariantMap{{QStringLiteral("fill"), QStringLiteral("#5e5e5e")}});
}

// ============================================================================
// CycleListWidget
// ============================================================================

CycleListWidget::CycleListWidget(const QStringList& items, const QSize& itemSize,
                                 Qt::Alignment align, QWidget* parent)
    : QListWidget(parent), itemSize_(itemSize), align_(align) {
    setProperty("qssClass", QStringLiteral("CycleListWidget"));
    upButton_ = new ScrollButton(FluentIconEnum::CareUpSolid, this);
    downButton_ = new ScrollButton(FluentIconEnum::CareDownSolid, this);

    originItems_ = items;
    lastScrollTime_ = QTime::currentTime();

    vScrollBar_ = new SmoothScrollBar(Qt::Vertical, this);

    setItems(items);

    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    vScrollBar_->setScrollAnimation(scrollDuration_);
    vScrollBar_->setForceHidden(true);

    setViewportMargins(0, 0, 0, 0);
    setFixedSize(itemSize_.width() + 8, itemSize_.height() * visibleNumber_);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    connect(this, &QListWidget::itemClicked, this, &CycleListWidget::onItemClicked);
    installEventFilter(this);

    connect(upButton_, &QToolButton::clicked, this, &CycleListWidget::scrollUp);
    connect(downButton_, &QToolButton::clicked, this, &CycleListWidget::scrollDown);

    upButton_->setAutoRepeatDelay(500);
    upButton_->setAutoRepeatInterval(50);
    downButton_->setAutoRepeatDelay(500);
    downButton_->setAutoRepeatInterval(50);

    setScrollButtonRepeatEnabled(true);
    setButtonsVisible(false);
}

int CycleListWidget::visibleNumber() const { return visibleNumber_; }

void CycleListWidget::setVisibleNumber(int n) {
    if (n <= 0 || n == visibleNumber_) {
        return;
    }

    visibleNumber_ = n;
    setFixedSize(itemSize_.width() + 8, itemSize_.height() * visibleNumber_);
    setItems(originItems_);
}

void CycleListWidget::setItems(const QStringList& items) {
    originItems_ = items;
    clear();
    createItems(items);
}

void CycleListWidget::createItems(const QStringList& items) {
    const int N = items.size();
    isCycle_ = N > visibleNumber_;

    if (isCycle_) {
        addColumnItems(items);
        addColumnItems(items);

        currentIndex_ = items.size();
        if (currentIndex_ - visibleNumber_ / 2 >= 0) {
            QListWidget::scrollToItem(item(currentIndex_ - visibleNumber_ / 2), PositionAtTop);
        }
    } else {
        const int n = visibleNumber_ / 2;
        QStringList empties;
        empties.reserve(n);
        for (int i = 0; i < n; ++i) {
            empties << QString();
        }

        addColumnItems(empties, true);
        addColumnItems(items);
        addColumnItems(empties, true);

        currentIndex_ = n;
    }
}

void CycleListWidget::addColumnItems(const QStringList& items, bool disabled) {
    for (const QString& s : items) {
        auto* it = new QListWidgetItem(s, this);
        it->setSizeHint(itemSize_);
        it->setTextAlignment(align_ | Qt::AlignVCenter);
        if (disabled) {
            it->setFlags(Qt::NoItemFlags);
        }
        addItem(it);
    }
}

void CycleListWidget::onItemClicked(QListWidgetItem* item) {
    if (!item) {
        return;
    }

    setCurrentIndex(row(item));
    scrollToItemInternal(currentItem());
}

void CycleListWidget::setSelectedItem(const QString& text) {
    if (text.isNull()) {
        return;
    }

    const QList<QListWidgetItem*> items = findItems(text, Qt::MatchExactly);
    if (items.isEmpty()) {
        return;
    }

    if (items.size() >= 2) {
        setCurrentIndex(row(items[1]));
    } else {
        setCurrentIndex(row(items[0]));
    }

    QListWidget::scrollToItem(currentItem(), PositionAtCenter);
}

void CycleListWidget::scrollToItemInternal(QListWidgetItem* item) {
    if (!item) {
        return;
    }

    const int index = row(item);
    const int y = item->sizeHint().height() * (index - visibleNumber_ / 2);
    vScrollBar_->scrollTo(y);

    clearSelection();
    item->setSelected(false);

    emit cycleCurrentItemChanged(item);
}

void CycleListWidget::wheelEvent(QWheelEvent* e) {
    if (!e) {
        return;
    }

    if (e->angleDelta().y() < 0) {
        scrollDown();
    } else {
        scrollUp();
    }
}

void CycleListWidget::setScrollButtonRepeatEnabled(bool enabled) {
    if (scrollButtonRepeatEnabled_ == enabled) {
        return;
    }

    scrollButtonRepeatEnabled_ = enabled;
    upButton_->setAutoRepeat(enabled);
    downButton_->setAutoRepeat(enabled);
}

bool CycleListWidget::isScrollButtonRepeatEnabled() const { return scrollButtonRepeatEnabled_; }

QSize CycleListWidget::itemSize() const { return itemSize_; }

void CycleListWidget::scrollWithAnimation(int index) {
    const QTime t = QTime::currentTime();
    const int elapsed = lastScrollTime_.msecsTo(t);
    lastScrollTime_ = t;

    int duration = 250;
    QEasingCurve::Type easing = QEasingCurve::OutQuad;

    if ((upButton_->isDown() || downButton_->isDown()) && elapsed < 200) {
        duration = 100;
        easing = QEasingCurve::Linear;
    }

    vScrollBar_->setScrollAnimation(duration, easing);
    setCurrentIndex(index);
    scrollToItemInternal(currentItem());
}

void CycleListWidget::scrollDown() { scrollWithAnimation(currentIndex() + 1); }

void CycleListWidget::scrollUp() { scrollWithAnimation(currentIndex() - 1); }

void CycleListWidget::setButtonsVisible(bool visible) {
    upButton_->setVisible(visible);
    downButton_->setVisible(visible);
}

void CycleListWidget::enterEvent(QEnterEvent* e) {
    Q_UNUSED(e);
    setButtonsVisible(true);
}

void CycleListWidget::leaveEvent(QEvent* e) {
    Q_UNUSED(e);
    setButtonsVisible(false);
}

void CycleListWidget::resizeEvent(QResizeEvent* e) {
    QListWidget::resizeEvent(e);

    const int w = width();
    const int h = 34;

    upButton_->resize(w, h);
    downButton_->resize(w, h);
    downButton_->move(0, height() - h);
}

bool CycleListWidget::eventFilter(QObject* obj, QEvent* e) {
    if (obj != this || !e || e->type() != QEvent::KeyPress) {
        return QListWidget::eventFilter(obj, e);
    }

    auto* ke = static_cast<QKeyEvent*>(e);
    if (ke->key() == Qt::Key_Down) {
        scrollDown();
        return true;
    }

    if (ke->key() == Qt::Key_Up) {
        scrollUp();
        return true;
    }

    return QListWidget::eventFilter(obj, e);
}

QListWidgetItem* CycleListWidget::currentItem() const { return item(currentIndex()); }

int CycleListWidget::currentIndex() const { return currentIndex_; }

void CycleListWidget::setCurrentIndex(int index) {
    if (!isCycle_) {
        const int n = visibleNumber_ / 2;
        const int minIndex = n;
        const int maxIndex = n + originItems_.size() - 1;
        currentIndex_ = qMax(minIndex, qMin(maxIndex, index));
        return;
    }

    const int N = count() / 2;
    const int m = (visibleNumber_ + 1) / 2;
    currentIndex_ = index;

    // scroll to center to achieve circular scrolling
    if (index >= count() - m) {
        currentIndex_ = N + index - count();
        if (currentIndex_ - 1 >= 0) {
            QListWidget::scrollToItem(item(currentIndex_ - 1), PositionAtCenter);
        }
    } else if (index <= m - 1) {
        currentIndex_ = N + index;
        QListWidget::scrollToItem(item(N + index + 1), PositionAtCenter);
    }
}

}  // namespace qfw
