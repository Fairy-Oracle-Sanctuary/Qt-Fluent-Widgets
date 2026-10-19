#include "components/widgets/tool_box.h"

#include <QHideEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QVariantAnimation>
#include <algorithm>
#include <functional>
#include <memory>
#include <stdexcept>

#include "common/config.h"
#include "common/font.h"
#include "components/widgets/card_widget.h"

namespace qfw {
namespace {
class ToolBoxHeader : public QAbstractButton {
public:
    explicit ToolBoxHeader(QWidget* parent) : QAbstractButton(parent) {
        setFixedHeight(48);
        setCheckable(true);
        setFocusPolicy(Qt::StrongFocus);
        qfw::setFont(this, 14);
        connect(&QConfig::instance(), &QConfig::themeChanged, this, [this] { update(); });
    }
    void setFluentIcon(const FluentIconBase& icon) {
        fluent_.reset(icon.clone());
        QAbstractButton::setIcon(icon.qicon());
        update();
    }
    void setPlainIcon(const QIcon& icon) {
        fluent_.reset();
        QAbstractButton::setIcon(icon);
        update();
    }
    QSize sizeHint() const override {
        return QSize(fontMetrics().horizontalAdvance(text()) + (icon().isNull() ? 66 : 96), 48);
    }

protected:
    void enterEvent(enterEvent_QEnterEvent* event) override {
        hover_ = true;
        update();
        QAbstractButton::enterEvent(event);
    }
    void leaveEvent(QEvent* event) override {
        hover_ = false;
        update();
        QAbstractButton::leaveEvent(event);
    }
    void mousePressEvent(QMouseEvent* event) override {
        keyboardFocus_ = false;
        QAbstractButton::mousePressEvent(event);
    }
    void focusInEvent(QFocusEvent* event) override {
        keyboardFocus_ =
            event->reason() == Qt::TabFocusReason || event->reason() == Qt::BacktabFocusReason;
        QAbstractButton::focusInEvent(event);
        update();
    }
    void focusOutEvent(QFocusEvent* event) override {
        keyboardFocus_ = false;
        QAbstractButton::focusOutEvent(event);
        update();
    }
    void keyPressEvent(QKeyEvent* event) override {
        if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
            click();
            event->accept();
        } else {
            QAbstractButton::keyPressEvent(event);
        }
    }
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        if (!isEnabled()) {
            painter.setOpacity(0.43);
        }
        const bool rtl = layoutDirection() == Qt::RightToLeft;
        const QRectF arrow(rtl ? 7 : width() - 37, 9, 30, 30);
        if (hover_ || keyboardFocus_ || isDown()) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(isDarkTheme() ? QColor(255, 255, 255, isDown() ? 10 : 16)
                                           : QColor(0, 0, 0, isDown() ? 8 : 12));
            painter.drawRoundedRect(arrow, 4, 4);
        }
        QColor foreground = isDarkTheme() ? QColor(Qt::white) : QColor(Qt::black);
        painter.setPen(foreground);
        const bool hasIcon = !icon().isNull();
        if (hasIcon) {
            const QRectF iconRect(rtl ? width() - 38 : 18, 14, 20, 20);
            if (fluent_) {
                fluent_->render(&painter, iconRect.toRect());
            } else {
                icon().paint(&painter, iconRect.toRect());
            }
        }
        const int left = rtl ? 48 : (hasIcon ? 48 : 18);
        const int right = rtl ? (hasIcon ? 48 : 18) : 48;
        const QRect textRect(left, 0, std::max(0, width() - left - right), height());
        painter.drawText(textRect, Qt::AlignVCenter | (rtl ? Qt::AlignRight : Qt::AlignLeft),
                         fontMetrics().elidedText(text(), Qt::ElideRight, textRect.width()));
        painter.setPen(QPen(foreground, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        const QPointF center = arrow.center();
        const qreal direction = isChecked() ? -1 : 1;
        QPainterPath chevron;
        chevron.moveTo(center.x() - 3.5, center.y() - direction * 1.75);
        chevron.lineTo(center.x(), center.y() + direction * 1.75);
        chevron.lineTo(center.x() + 3.5, center.y() - direction * 1.75);
        painter.drawPath(chevron);
    }

private:
    std::unique_ptr<FluentIconBase> fluent_;
    bool hover_ = false;
    bool keyboardFocus_ = false;
};
}  // namespace

class ToolBoxItem : public SimpleCardWidget {
public:
    ToolBoxItem(QWidget* page, const QString& text, QWidget* parent, std::function<void()> finished)
        : SimpleCardWidget(parent), animation_(this), finished_(std::move(finished)) {
        setBorderRadius(6);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        header = new ToolBoxHeader(this);
        header->setText(text);
        header->setAccessibleName(text);
        clip_ = new QWidget(this);
        body_ = new QWidget(clip_);
        auto* layout = new QVBoxLayout(body_);
        layout->setContentsMargins(12, 12, 12, 12);
        layout->setSpacing(0);
        layout->addWidget(page);
        layout->setSizeConstraint(QLayout::SetNoConstraint);
        page_ = page;
        body_->installEventFilter(this);
        clip_->hide();
        setFixedHeight(50);
        animation_.setEasingCurve(QEasingCurve::InOutCubic);
        connect(&animation_, &QVariantAnimation::valueChanged, this, [this](const QVariant& value) {
            extent_ = value.toReal();
            applyGeometry();
        });
        connect(&animation_, &QVariantAnimation::finished, this, [this] {
            extent_ = expanded_ ? naturalExtent() : 0;
            applyGeometry();
            finished_();
        });
    }
    ToolBoxHeader* header = nullptr;
    QWidget* page() const { return page_; }
    qreal extent() const { return extent_; }
    void stop() { animation_.stop(); }
    void setExpanded(bool expanded, int duration) {
        if (expanded == expanded_ && duration &&
            (animation_.state() == QAbstractAnimation::Running ||
             qFuzzyCompare(extent_ + 1, (expanded ? naturalExtent() : 0) + 1))) {
            return;
        }
        animation_.stop();
        expanded_ = expanded;
        const qreal end = expanded ? naturalExtent() : 0;
        if (!duration || qFuzzyCompare(end + 1, extent_ + 1)) {
            extent_ = end;
            applyGeometry();
            return;
        }
        if (expanded || extent_ > 0) {
            clip_->show();
            body_->show();
        }
        animation_.setStartValue(extent_);
        animation_.setEndValue(end);
        animation_.setDuration(duration);
        animation_.start();
    }

protected:
    void resizeEvent(QResizeEvent* event) override {
        SimpleCardWidget::resizeEvent(event);
        if (!header) {
            return;
        }
        if (event->size().width() != event->oldSize().width() && expanded_) {
            if (animation_.state() == QAbstractAnimation::Running) {
                animation_.setEndValue(naturalExtent());
            } else {
                extent_ = naturalExtent();
            }
        }
        applyGeometry();
    }
    bool eventFilter(QObject* watched, QEvent* event) override {
        if (watched == body_ && event->type() == QEvent::LayoutRequest && expanded_) {
            if (animation_.state() == QAbstractAnimation::Running) {
                animation_.setEndValue(naturalExtent());
            } else {
                extent_ = naturalExtent();
                applyGeometry();
            }
        }
        return SimpleCardWidget::eventFilter(watched, event);
    }
    void paintEvent(QPaintEvent* event) override {
        SimpleCardWidget::paintEvent(event);
        if (extent_ > 0) {
            QPainter painter(this);
            painter.fillRect(QRect(1, 49, width() - 2, 1),
                             isDarkTheme() ? QColor(0, 0, 0, 55) : QColor(0, 0, 0, 18));
        }
    }

private:
    qreal naturalExtent() const {
        auto* layout = body_->layout();
        const int width = std::max(0, this->width() - 2);
        const int hint = layout->hasHeightForWidth() ? layout->totalHeightForWidth(width)
                                                     : layout->sizeHint().height();
        return std::max({hint, layout->minimumSize().height(), 24}) + 1;
    }
    void applyGeometry() {
        const int extent = std::max(0, qRound(extent_));
        header->setGeometry(1, 1, std::max(0, width() - 2), 48);
        clip_->setGeometry(1, 50, std::max(0, width() - 2), extent);
        body_->setGeometry(0, 0, clip_->width(), qRound(naturalExtent()) - 1);
        clip_->setVisible(extent > 0);
        body_->setVisible(extent > 0 || expanded_);
        setFixedHeight(50 + extent);
        updateGeometry();
        update();
    }
    QPointer<QWidget> page_;
    QWidget* clip_ = nullptr;
    QWidget* body_ = nullptr;
    QVariantAnimation animation_;
    std::function<void()> finished_;
    qreal extent_ = 0;
    bool expanded_ = false;
};

ToolBox::ToolBox(QWidget* parent) : QWidget(parent), syncTimer_(this) {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    layout_ = new QVBoxLayout(this);
    layout_->setContentsMargins(0, 0, 0, 0);
    layout_->setSpacing(6);
    layout_->setAlignment(Qt::AlignTop);
    syncTimer_.setSingleShot(true);
    connect(&syncTimer_, &QTimer::timeout, this, &ToolBox::syncPanels);
}
int ToolBox::count() const { return items_.size(); }
int ToolBox::currentIndex() const { return current_; }
QWidget* ToolBox::currentWidget() const { return current_ < 0 ? nullptr : widget(current_); }
ToolBoxItem* ToolBox::item(int index) const {
    if (index < 0 || index >= count()) {
        throw std::out_of_range("Invalid toolbox index");
    }
    return items_.at(index);
}
QWidget* ToolBox::widget(int index) const { return item(index)->page(); }
int ToolBox::indexOf(QWidget* widget) const {
    for (int i = 0; i < count(); ++i) {
        if (items_.at(i)->page() == widget) {
            return i;
        }
    }
    return -1;
}
int ToolBox::addItem(QWidget* widget, const QString& text) {
    return insertItem(count(), widget, text);
}
int ToolBox::addItem(QWidget* widget, const QIcon& icon, const QString& text) {
    const int index = addItem(widget, text);
    setItemIcon(index, icon);
    return index;
}
int ToolBox::addItem(QWidget* widget, const FluentIconBase& icon, const QString& text) {
    const int index = addItem(widget, text);
    setItemIcon(index, icon);
    return index;
}
int ToolBox::insertItem(int index, QWidget* widget, const QString& text) {
    if (!widget || widget == this || widget->isAncestorOf(this) || indexOf(widget) >= 0) {
        throw std::invalid_argument("Invalid or duplicate toolbox page");
    }
    if (index < 0 || index > count()) {
        throw std::out_of_range("Invalid toolbox insertion index");
    }
    auto* panel = new ToolBoxItem(widget, text, this, [this] { syncTimer_.start(0); });
    items_.insert(index, panel);
    layout_->insertWidget(index, panel);
    connect(panel->header, &QAbstractButton::clicked, this, [this, panel] {
        const int index = items_.indexOf(panel);
        if (index >= 0) {
            setCurrentIndex(current_ == index ? -1 : index);
        }
    });
    widget->show();
    panel->show();
    if (count() == 1) {
        setCurrentIndex(0);
    } else if (current_ >= index) {
        ++current_;
        syncPanels();
        emit currentChanged(current_);
    }
    return index;
}
void ToolBox::setCurrentIndex(int index) {
    if (index < -1 || index >= count()) {
        throw std::out_of_range("Invalid toolbox selection");
    }
    if (index >= 0 && !isItemEnabled(index)) {
        return;
    }
    if (current_ == index) {
        syncPanels();
        return;
    }
    current_ = index;
    syncPanels();
    emit currentChanged(current_);
}
void ToolBox::syncPanels() {
    const int duration = isVisible() ? duration_ : 0;
    bool closing = false;
    for (int i = 0; i < count(); ++i) {
        auto* panel = items_.at(i);
        panel->header->setChecked(i == current_);
        if (i != current_) {
            panel->setExpanded(false, duration);
            closing |= panel->extent() > 0;
        }
    }
    if (!closing && current_ >= 0) {
        items_.at(current_)->setExpanded(true, duration);
    }
}
void ToolBox::removeItem(int index) {
    auto* panel = item(index);
    const bool selected = current_ == index;
    const int old = current_;
    panel->stop();
    disconnect(panel->header, nullptr, this, nullptr);
    if (auto* page = panel->page()) {
        page->hide();
        page->setParent(nullptr);
    }
    items_.removeAt(index);
    layout_->removeWidget(panel);
    panel->hide();
    panel->deleteLater();
    if (selected) {
        current_ = -1;
        for (int i = std::min(index, count() - 1); i >= 0 && i < count(); ++i) {
            if (isItemEnabled(i)) {
                current_ = i;
                break;
            }
        }
        if (current_ < 0) {
            for (int i = std::min(index - 1, count() - 1); i >= 0; --i) {
                if (isItemEnabled(i)) {
                    current_ = i;
                    break;
                }
            }
        }
    } else if (current_ > index) {
        --current_;
    }
    syncPanels();
    if (selected || old != current_) {
        emit currentChanged(current_);
    }
}
QString ToolBox::itemText(int index) const { return item(index)->header->text(); }
void ToolBox::setItemText(int index, const QString& text) {
    auto* header = item(index)->header;
    header->setText(text);
    header->setAccessibleName(text);
    header->updateGeometry();
}
QIcon ToolBox::itemIcon(int index) const { return item(index)->header->icon(); }
void ToolBox::setItemIcon(int index, const QIcon& icon) { item(index)->header->setPlainIcon(icon); }
void ToolBox::setItemIcon(int index, const FluentIconBase& icon) {
    item(index)->header->setFluentIcon(icon);
}
QAbstractButton* ToolBox::itemHeader(int index) const { return item(index)->header; }
bool ToolBox::isItemEnabled(int index) const { return item(index)->header->isEnabled(); }
void ToolBox::setItemEnabled(int index, bool enabled) {
    auto* panel = item(index);
    panel->header->setEnabled(enabled);
    if (panel->page()) {
        panel->page()->setEnabled(enabled);
    }
    if (!enabled && current_ == index) {
        int next = -1;
        for (int i = 0; i < count(); ++i) {
            if (isItemEnabled(i)) {
                next = i;
                break;
            }
        }
        setCurrentIndex(next);
    }
}
int ToolBox::animationDuration() const { return duration_; }
void ToolBox::setAnimationDuration(int duration) {
    if (duration < 0) {
        throw std::invalid_argument("Invalid toolbox duration");
    }
    duration_ = duration;
    if (!duration) {
        syncPanels();
    }
}
void ToolBox::hideEvent(QHideEvent* event) {
    QWidget::hideEvent(event);
    syncTimer_.stop();
    for (int i = 0; i < count(); ++i) {
        items_.at(i)->setExpanded(i == current_, 0);
    }
}
}  // namespace qfw
