#include "components/widgets/exclusive_filter.h"

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QHideEvent>
#include <QPropertyAnimation>
#include <QScrollArea>
#include <QScrollBar>
#include <QSignalBlocker>
#include <QWheelEvent>
#include <algorithm>

#include "components/widgets/extended_button.h"

namespace qfw {
LiteFilterBase::LiteFilterBase(bool multi, bool outlined, QWidget* parent)
    : QWidget(parent), multiSelection_(multi), outlined_(outlined) {
    setFixedHeight(36);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    hBoxLayout = new QHBoxLayout(this);
    hBoxLayout->setContentsMargins(0, 0, 0, 0);
    hBoxLayout->setSpacing(0);
    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setFixedHeight(36);
    scrollArea->setStyleSheet(
        QStringLiteral("QScrollArea { background: transparent; border: none; }"));
    scrollArea->viewport()->setAutoFillBackground(false);
    scrollArea->viewport()->installEventFilter(this);
    container = new QWidget;
    container->setAutoFillBackground(false);
    itemsLayout = new QHBoxLayout(container);
    itemsLayout->setContentsMargins(0, 0, 0, 0);
    itemsLayout->setSpacing(8);
    itemsLayout->setAlignment(Qt::AlignLeading | Qt::AlignVCenter);
    itemsLayout->setSizeConstraint(QLayout::SetMinimumSize);
    scrollArea->setWidget(container);
    // setWidget() enables autofill on its content, overriding the earlier
    // setting and exposing the application's (possibly dark) Window palette.
    container->setAutoFillBackground(false);
    scrollArea->viewport()->setAutoFillBackground(false);
    rightArrowBtn = new TransparentToolButton(FluentIcon(FluentIconEnum::CareRightSolid), this);
    rightArrowBtn->setFixedSize(24, 24);
    rightArrowBtn->setIconSize(QSize(12, 12));
    rightArrowBtn->setToolTip(tr("Scroll to more options"));
    rightArrowBtn->setAccessibleName(rightArrowBtn->toolTip());
    spacer = new QWidget(this);
    spacer->setFixedSize(24, 24);
    hBoxLayout->addWidget(scrollArea, 1);
    hBoxLayout->addWidget(rightArrowBtn, 0, Qt::AlignVCenter);
    hBoxLayout->addWidget(spacer, 0, Qt::AlignVCenter);
    group_ = new QButtonGroup(this);
    group_->setExclusive(!multi);
    auto* bar = scrollArea->horizontalScrollBar();
    scrollAnimation_ = new QPropertyAnimation(bar, "value", this);
    scrollAnimation_->setDuration(160);
    scrollAnimation_->setEasingCurve(QEasingCurve::OutCubic);
    connect(rightArrowBtn, &QAbstractButton::clicked, this, [this] { scrollBy(100); });
    connect(bar, &QScrollBar::valueChanged, this, [this] { updateArrowVisibility(); });
    connect(bar, &QScrollBar::rangeChanged, this, [this] {
        scrollAnimation_->stop();
        updateArrowVisibility();
    });
    updateArrowVisibility();
}
LiteFilterBase::~LiteFilterBase() {
    clearing_ = true;
    scrollAnimation_->stop();
    for (auto* item : buttons_) {
        item->disconnect(this);
        item->removeEventFilter(this);
    }
}
FluentPushButton* LiteFilterBase::addItem(const QString& text) {
    if (buttons_.contains(text)) {
        return buttons_.value(text);
    }
    auto* item = outlined_ ? static_cast<FluentPushButton*>(new OutlinedPushButton(text, container))
                           : static_cast<FluentPushButton*>(new PillPushButton(text, container));
    item->setCheckable(true);
    resizeItem(item);
    item->setToolTip(text);
    item->setAccessibleName(text);
    item->installEventFilter(this);
    group_->addButton(item);
    buttons_.insert(text, item);
    items_.append(text);
    itemsLayout->addWidget(item);
    connect(item, &QAbstractButton::toggled, this, [this](bool checked) {
        if (updating_ || (!multiSelection_ && !checked)) {
            return;
        }
        QStringList selected;
        for (const auto& key : items_) {
            if (buttons_.value(key)->isChecked()) {
                selected.append(key);
            }
        }
        applySelection(selected);
    });
    // If a caller deletes an owned item, remove the key and repair selection as well.
    connect(item, &QObject::destroyed, this, [this, text] {
        if (!clearing_ && buttons_.contains(text)) {
            buttons_.remove(text);
            items_.removeAll(text);
            auto selected = selection_;
            selected.removeAll(text);
            if (!multiSelection_ && selected.isEmpty() && !items_.isEmpty()) {
                selected.append(items_.first());
            }
            applySelection(selected);
        }
    });
    updateGeometry();
    if (!multiSelection_ && selection_.isEmpty()) {
        applySelection({text});
    }
    return item;
}
void LiteFilterBase::addItems(const QStringList& texts) {
    for (const auto& text : texts) {
        addItem(text);
    }
}
void LiteFilterBase::removeItem(const QString& text) {
    auto* item = buttons_.take(text);
    if (!item) {
        return;
    }
    items_.removeAll(text);
    group_->removeButton(item);
    itemsLayout->removeWidget(item);
    item->disconnect(this);
    item->removeEventFilter(this);
    item->hide();
    item->deleteLater();
    auto selected = selection_;
    selected.removeAll(text);
    if (!multiSelection_ && selected.isEmpty() && !items_.isEmpty()) {
        selected.append(items_.first());
    }
    updateGeometry();
    applySelection(selected);
}
void LiteFilterBase::clear() {
    scrollAnimation_->stop();
    clearing_ = true;
    const auto keys = items_;
    for (const auto& text : keys) {
        auto* item = buttons_.take(text);
        group_->removeButton(item);
        itemsLayout->removeWidget(item);
        item->disconnect(this);
        item->removeEventFilter(this);
        item->hide();
        item->deleteLater();
    }
    items_.clear();
    clearing_ = false;
    updateGeometry();
    applySelection({});
}
QStringList LiteFilterBase::items() const { return items_; }
FluentPushButton* LiteFilterBase::button(const QString& text) const { return buttons_.value(text); }
QStringList LiteFilterBase::selection() const { return selection_; }
void LiteFilterBase::applySelection(const QStringList& texts) {
    QStringList selected;
    for (const auto& key : items_) {
        if (texts.contains(key)) {
            selected.append(key);
            if (!multiSelection_) {
                break;
            }
        }
    }
    const bool changed = selected != selection_;
    selection_ = selected;
    updating_ = true;
    group_->setExclusive(false);
    for (const auto& key : items_) {
        auto* item = buttons_.value(key);
        const QSignalBlocker blocker(item);
        item->setChecked(selected.contains(key));
    }
    group_->setExclusive(!multiSelection_);
    updating_ = false;
    if (changed) {
        notifySelectionChanged();
    }
}
void LiteFilterBase::resizeItem(FluentPushButton* item) {
    item->setFixedSize(qMax(32, item->sizeHint().width()), 32);
}
void LiteFilterBase::ensureItemVisible(FluentPushButton* item) {
    scrollAnimation_->stop();
    auto* viewport = scrollArea->viewport();
    const QRect bounds(item->mapTo(viewport, QPoint()), item->size());
    int distance = 0;
    if (bounds.width() > viewport->width() - 16) {
        distance = bounds.center().x() - viewport->rect().center().x();
    } else if (bounds.left() < 8) {
        distance = bounds.left() - 8;
    } else if (bounds.right() > viewport->width() - 9) {
        distance = bounds.right() - (viewport->width() - 9);
    }
    // Scrollbar values move content in opposite physical directions in RTL.
    auto* bar = scrollArea->horizontalScrollBar();
    const auto target = qint64(bar->value()) + (isRightToLeft() ? -qint64(distance) : distance);
    bar->setValue(int(std::clamp(target, qint64(bar->minimum()), qint64(bar->maximum()))));
}
QSize LiteFilterBase::sizeHint() const {
    return QSize(qBound(80, itemsLayout->sizeHint().width() + 24, 480), 36);
}
QSize LiteFilterBase::minimumSizeHint() const { return QSize(80, 36); }
void LiteFilterBase::updateArrowVisibility() {
    auto* bar = scrollArea->horizontalScrollBar();
    const bool more = bar->value() < bar->maximum();
    rightArrowBtn->setVisible(more);
    spacer->setVisible(!more);
}
void LiteFilterBase::scrollBy(int distance) {
    auto* bar = scrollArea->horizontalScrollBar();
    const int base = scrollAnimation_->state() == QAbstractAnimation::Running
                         ? scrollAnimation_->endValue().toInt()
                         : bar->value();
    const int target =
        int(std::clamp(qint64(base) + distance, qint64(bar->minimum()), qint64(bar->maximum())));
    scrollAnimation_->stop();
    if (!isVisible() || target == bar->value()) {
        bar->setValue(target);
        return;
    }
    scrollAnimation_->setStartValue(bar->value());
    scrollAnimation_->setEndValue(target);
    scrollAnimation_->start();
}
void LiteFilterBase::wheelEvent(QWheelEvent* event) {
    if (!isEnabled()) {
        event->ignore();
        return;
    }
    const auto pixel = event->pixelDelta();
    const auto angle = event->angleDelta();
    const int delta =
        !pixel.isNull() ? (pixel.x() ? pixel.x() : pixel.y()) : (angle.x() ? angle.x() : angle.y());
    if (!delta || scrollArea->horizontalScrollBar()->maximum() == 0) {
        event->ignore();
        return;
    }
    scrollBy(-delta);
    event->accept();
}
bool LiteFilterBase::eventFilter(QObject* watched, QEvent* event) {
    if (event->type() == QEvent::Wheel) {
        wheelEvent(static_cast<QWheelEvent*>(event));
        return event->isAccepted();
    }
    if (auto* item = qobject_cast<FluentPushButton*>(watched)) {
        if (event->type() == QEvent::FocusIn) {
            ensureItemVisible(item);
        } else if (event->type() == QEvent::FontChange || event->type() == QEvent::StyleChange) {
            resizeItem(item);
        }
    }
    return QWidget::eventFilter(watched, event);
}
void LiteFilterBase::hideEvent(QHideEvent* event) {
    scrollAnimation_->stop();
    QWidget::hideEvent(event);
}
void LiteFilterBase::changeEvent(QEvent* event) {
    if (event->type() == QEvent::EnabledChange && !isEnabled()) {
        scrollAnimation_->stop();
    }
    if (event->type() == QEvent::LayoutDirectionChange) {
        scrollAnimation_->stop();
        rightArrowBtn->setIcon(FluentIcon(isRightToLeft() ? FluentIconEnum::CareLeftSolid
                                                          : FluentIconEnum::CareRightSolid));
    }
    QWidget::changeEvent(event);
}
ExclusiveLiteFilterBase::ExclusiveLiteFilterBase(bool outlined, QWidget* parent)
    : LiteFilterBase(false, outlined, parent) {}
QString ExclusiveLiteFilterBase::currentItem() const {
    return selection().isEmpty() ? QString() : selection().first();
}
void ExclusiveLiteFilterBase::setCurrentItem(const QString& text) {
    if (button(text)) {
        applySelection({text});
    }
}
void ExclusiveLiteFilterBase::notifySelectionChanged() { emit currentTextChanged(currentItem()); }
ExclusiveLiteFilter::ExclusiveLiteFilter(QWidget* parent)
    : ExclusiveLiteFilterBase(false, parent) {}
OutlinedExclusiveLiteFilter::OutlinedExclusiveLiteFilter(QWidget* parent)
    : ExclusiveLiteFilterBase(true, parent) {}
MultiSelectionLiteFilterBase::MultiSelectionLiteFilterBase(bool outlined, QWidget* parent)
    : LiteFilterBase(true, outlined, parent) {}
QStringList MultiSelectionLiteFilterBase::currentItems() const { return selection(); }
void MultiSelectionLiteFilterBase::setCurrentItems(const QStringList& texts) {
    applySelection(texts);
}
void MultiSelectionLiteFilterBase::clearSelection() { applySelection({}); }
void MultiSelectionLiteFilterBase::notifySelectionChanged() {
    emit currentItemsChanged(currentItems());
}
MultiSelectionLiteFilter::MultiSelectionLiteFilter(QWidget* parent)
    : MultiSelectionLiteFilterBase(false, parent) {}
OutlinedMultiSelectionLiteFilter::OutlinedMultiSelectionLiteFilter(QWidget* parent)
    : MultiSelectionLiteFilterBase(true, parent) {}
}  // namespace qfw
