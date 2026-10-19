#include "components/widgets/toast.h"

#include <QCloseEvent>
#include <QApplication>
#include <QMap>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QHideEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QShowEvent>
#include <QTimer>
#include <QVBoxLayout>
#include <algorithm>
#include <array>
#include <stdexcept>

#include "common/config.h"
#include "common/font.h"
#include "common/style_sheet.h"
#include "components/widgets/button.h"
#include "components/widgets/label.h"

namespace qfw {
namespace {
int positionIndex(ToastPosition position) {
    const int index = int(position);
    if (index < 0 || index >= 6) throw std::invalid_argument("Invalid toast position");
    return index;
}
QSize labelTextSize(QLabel* label, int width = -1) {
    if (label->text().isEmpty()) return {};
    // Measure with QLabel's own text engine (including fallback-font bearings
    // and DPI rounding), without the displayed label's fixed-height constraint.
    QLabel measure(label->parentWidget());
    measure.ensurePolished();
    measure.setFont(label->font());
    measure.setTextFormat(Qt::PlainText);
    measure.setText(label->text());
    if (width < 0) return measure.sizeHint();
    measure.setWordWrap(true);
    return QSize(width, measure.heightForWidth(width));
}
}  // namespace

namespace {
QMap<ToastPosition, ToastManager::Factory>& toastFactories() {
    static QMap<ToastPosition, ToastManager::Factory> factories;
    return factories;
}
QMap<ToastPosition, QPointer<ToastManager>>& toastManagers() {
    static QMap<ToastPosition, QPointer<ToastManager>> managers;
    return managers;
}
}
ToastManager::ToastManager(ToastPosition position, QObject* parent) : QObject(parent), position_(position) {}
ToastManager* ToastManager::get(ToastPosition position) {
    positionIndex(position);
    auto& manager=toastManagers()[position];
    if (!manager) {
        auto factory=toastFactories().value(position);
        manager=factory?factory():new ToastManager(position);
        if (!manager) throw std::runtime_error("Toast manager factory returned null");
        manager->position_=position;
        if (!manager->parent()) manager->setParent(qApp);
    }
    return manager;
}
void ToastManager::registerFactory(ToastPosition position, Factory factory) {
    positionIndex(position);
    auto& old=toastManagers()[position];
    if (old) {
        for (auto bar : old->bars_) if (bar) throw std::logic_error("Cannot replace an active toast manager");
        delete old.data();
    }
    if (factory) toastFactories()[position]=std::move(factory);
    else toastFactories().remove(position);
}
void ToastManager::add(Toast* toast) {
    if (toast && !bars_.contains(toast)) bars_.append(toast);
}
void ToastManager::remove(Toast* toast) {
    const bool removed=bars_.removeAll(toast)>0;
    for (int i=bars_.size()-1;i>=0;--i) if (!bars_[i]) bars_.removeAt(i);
    if (removed) reflow();
}
QList<Toast*> ToastManager::precedingBars(Toast* toast) const {
    QList<Toast*> result;
    for (auto bar : bars_) {
        if (bar==toast) break;
        if (bar && bar->parentWidget()==toast->parentWidget()) result.append(bar);
    }
    return result;
}
void ToastManager::setMargin(int margin) {
    if (margin<0) throw std::invalid_argument("Toast margin must be nonnegative");
    if (margin_==margin) return;
    margin_=margin; reflow();
}
void ToastManager::reflow() {
    const auto snapshot=bars_;
    for (auto bar : snapshot) if (bar && !bar->closing_) bar->animateTo(slideEndPos(bar),true);
}
QPoint ToastManager::slideEndPos(Toast* toast) const {
    if (!toast || !toast->parentWidget()) return {};
    auto* window=toast->parentWidget();
    const bool bottom=position_==ToastPosition::Bottom || position_==ToastPosition::BottomLeft || position_==ToastPosition::BottomRight;
    int x=(window->width()-toast->width())/2;
    if (position_==ToastPosition::TopLeft || position_==ToastPosition::BottomLeft) x=margin_;
    if (position_==ToastPosition::TopRight || position_==ToastPosition::BottomRight) x=window->width()-toast->width()-margin_;
    int y=bottom?window->height()-margin_-toast->height():qRound(margin_/2.5+margin_);
    for (auto* bar : precedingBars(toast)) y+=(bottom?-1:1)*(bar->height()+margin_);
    return QPoint(std::clamp(x,0,std::max(0,window->width()-toast->width())),y);
}
QPoint ToastManager::slideStartPos(Toast* toast) const {
    QPoint result=slideEndPos(toast);
    if (!toast || !toast->parentWidget()) return result;
    switch(position_) {
    case ToastPosition::Top: result.ry()-=toast->height()/4; break;
    case ToastPosition::Bottom: result.ry()+=toast->height()/4; break;
    case ToastPosition::TopLeft: case ToastPosition::BottomLeft: result.setX(-toast->width()); break;
    case ToastPosition::TopRight: result.setX(toast->parentWidget()->width()+toast->width()); break;
    case ToastPosition::BottomRight: result.setX(toast->parentWidget()->width()+margin_); break;
    }
    return result;
}

// Each window owns its stack; independent windows never influence one another.
class ToastStack : public QObject {
public:
    explicit ToastStack(QWidget* window) : QObject(window), window_(window) {
        window->installEventFilter(this);
        pending_ = new QTimer(this);
        pending_->setSingleShot(true);
        connect(pending_, &QTimer::timeout, this, [this] { reflow(true); });
    }
    static ToastStack* get(QWidget* parent) {
        for (auto* child : parent->children())
            if (auto* stack = dynamic_cast<ToastStack*>(child)) return stack;
        return new ToastStack(parent);
    }
    bool contains(Toast* toast) const {
        return bars_[positionIndex(toast->position_)].contains(toast);
    }
    void add(Toast* toast, bool entering = true) {
        if (!window_ || contains(toast)) return;
        bars_[positionIndex(toast->position_)].append(toast);
        ToastManager::get(toast->position_)->add(toast);
        toast->updateSize();
        const auto end = target(toast);
        if (entering) {
            toast->move(ToastManager::get(toast->position_)->slideStartPos(toast));
        }
        toast->animateTo(end, true);
    }
    void remove(Toast* toast) {
        bool removed = false;
        for (auto& bars : bars_) {
            const auto oldSize = bars.size();
            bars.erase(
                std::remove_if(bars.begin(), bars.end(),
                               [toast](const auto& bar) { return !bar || bar.data() == toast; }),
                bars.end());
            removed |= bars.size() != oldSize;
        }
        if (removed) ToastManager::get(toast->position_)->remove(toast);
        if (removed && window_) reflow(true);
    }
    QPoint target(const Toast* toast) const {
        return ToastManager::get(toast->position_)->slideEndPos(const_cast<Toast*>(toast));
    }
    void requestReflow() {
        if (!arranging_) pending_->start(0);
    }
    void reflow(bool animate) {
        if (!window_ || arranging_) return;
        arranging_ = true;
        const auto snapshot = bars_;
        for (const auto& bars : snapshot)
            for (const auto& bar : bars)
                if (bar && !bar->closing_) {
                    bar->updateSize();
                    bar->animateTo(target(bar), animate);
                }
        arranging_ = false;
    }

protected:
    bool eventFilter(QObject* object, QEvent* event) override {
        if (object == window_ && event->type() == QEvent::Destroy) {
            pending_->stop();
            window_ = nullptr;
        }
        if (object == window_ &&
            (event->type() == QEvent::Resize || event->type() == QEvent::WindowStateChange)) {
            pending_->stop();
            reflow(false);
        }
        return QObject::eventFilter(object, event);
    }

private:
    QPointer<QWidget> window_;
    QTimer* pending_ = nullptr;
    std::array<QList<QPointer<Toast>>, 6> bars_;
    bool arranging_ = false;
};

Toast::Toast(const QString& title, const QString& content, int duration, bool isClosable,
             ToastPosition position, Qt::Orientation orient, const QColor& toastColor,
             QWidget* parent, const QColor& backgroundColor, bool useThemeColor)
    : QFrame(parent),
      title_(title),
      content_(content),
      toastColor_(toastColor),
      backgroundColor_(backgroundColor),
      position_(position),
      orient_(orient),
      duration_(duration),
      closable_(isClosable),
      useThemeColor_(useThemeColor) {
    if (!parent) throw std::invalid_argument("Toast requires a parent widget");
    positionIndex(position);
    if (orient != Qt::Horizontal && orient != Qt::Vertical)
        throw std::invalid_argument("Invalid toast orientation");
    if (!toastColor.isValid()) throw std::invalid_argument("Invalid toast color");
    setFrameShape(QFrame::NoFrame);
    titleLabel = new BodyLabel(title, this);
    contentLabel = new BodyLabel(content, this);
    for (auto* label : {titleLabel, contentLabel}) {
        label->setTextFormat(Qt::PlainText);
        label->setWordWrap(true);
        label->setMinimumWidth(0);
        label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    }
    qfw::setFont(titleLabel, 13, QFont::DemiBold);
    setCustomStyleSheet(titleLabel, "QLabel {color:black; font-size:13px; font-weight:600;}",
                        "QLabel {color:white; font-size:13px; font-weight:600;}");
    titleLabel->setVisible(!title.isEmpty());
    contentLabel->setVisible(!content.isEmpty());
    closeButton = new TransparentToolButton(FluentIcon(FluentIconEnum::Close), this);
    closeButton->setFixedSize(28, 28);
    closeButton->setIconSize(QSize(12, 12));
    closeButton->setCursor(Qt::PointingHandCursor);
    closeButton->setVisible(isClosable);
    connect(closeButton, &QAbstractButton::clicked, this, [this] { close(); });
    row_ = new QHBoxLayout(this);
    row_->setContentsMargins(20, 18, 16, 16);
    row_->setSpacing(0);
    row_->setSizeConstraint(QLayout::SetNoConstraint);
    textLayout_ = orient == Qt::Horizontal ? static_cast<QBoxLayout*>(new QHBoxLayout)
                                           : static_cast<QBoxLayout*>(new QVBoxLayout);
    textLayout_->setContentsMargins(0, 0, 0, 0);
    textLayout_->setSpacing(10);
    if (orient == Qt::Vertical) textLayout_->setAlignment(Qt::AlignTop);
    // Vertical labels must use their height-for-width, not equally stretched
    // cells aligned to a stale sizeHint. Otherwise a short toast grows a gap.
    textLayout_->addWidget(titleLabel, orient == Qt::Horizontal ? 1 : 0,
                           orient == Qt::Horizontal ? Qt::AlignTop : Qt::Alignment());
    if (orient == Qt::Horizontal) textLayout_->addSpacing(7);
    textLayout_->addWidget(contentLabel, orient == Qt::Horizontal ? 1 : 0,
                           orient == Qt::Horizontal ? Qt::AlignTop : Qt::Alignment());
    widgetLayout = orient == Qt::Horizontal ? static_cast<QBoxLayout*>(new QHBoxLayout)
                                            : static_cast<QBoxLayout*>(new QVBoxLayout);
    widgetLayout->setContentsMargins(0, 0, 0, 0);
    widgetLayout->setSpacing(10);
    row_->addLayout(textLayout_, 1);
    if (orient == Qt::Horizontal)
        row_->addLayout(widgetLayout);
    else
        textLayout_->addLayout(widgetLayout);
    row_->addSpacing(12);
    row_->addWidget(closeButton, 0, Qt::AlignTop);
    auto* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(24);
    shadow->setOffset(0, 6);
    shadow->setColor(QColor(0, 0, 0, 64));
    setGraphicsEffect(shadow);
    positionAnimation_ = new QPropertyAnimation(this, "pos", this);
    positionAnimation_->setDuration(animationDuration_);
    positionAnimation_->setEasingCurve(QEasingCurve::OutQuad);
    closeTimer_ = new QTimer(this);
    closeTimer_->setSingleShot(true);
    connect(closeTimer_, &QTimer::timeout, this, [this] { close(); });
    connect(&QConfig::instance(), &QConfig::themeChanged, this, [this] { update(); });
    connect(&QConfig::instance(), &QConfig::themeColorChanged, this, [this] { update(); });
    stack_ = ToastStack::get(parent);
    updateSize();
}
Toast::~Toast() {
    if (stack_) stack_->remove(this);
}
QColor Toast::color(ToastColor color) {
    switch (color) {
        case ToastColor::Success:
            return QColor("#0F7B0F");
        case ToastColor::Warning:
            return QColor("#FCE100");
        case ToastColor::Error:
            return QColor("#BC0E11");
    }
    throw std::invalid_argument("Invalid toast color type");
}
Toast* Toast::newToast(const QString& title, const QString& content, int duration, bool isClosable,
                       ToastPosition position, Qt::Orientation orient, const QColor& toastColor,
                       QWidget* parent, const QColor& backgroundColor, bool useThemeColor) {
    auto* toast = new Toast(title, content, duration, isClosable, position, orient, toastColor,
                            parent, backgroundColor, useThemeColor);
    toast->show();
    return toast;
}
Toast* Toast::info(const QString& title, const QString& content, int duration, bool isClosable,
                   ToastPosition position, Qt::Orientation orient, QWidget* parent) {
    return newToast(title, content, duration, isClosable, position, orient, themeColor(), parent,
                    {}, true);
}
Toast* Toast::success(const QString& title, const QString& content, int duration, bool isClosable,
                      ToastPosition position, Qt::Orientation orient, QWidget* parent) {
    return newToast(title, content, duration, isClosable, position, orient,
                    color(ToastColor::Success), parent);
}
Toast* Toast::warning(const QString& title, const QString& content, int duration, bool isClosable,
                      ToastPosition position, Qt::Orientation orient, QWidget* parent) {
    return newToast(title, content, duration, isClosable, position, orient,
                    color(ToastColor::Warning), parent);
}
Toast* Toast::error(const QString& title, const QString& content, int duration, bool isClosable,
                    ToastPosition position, Qt::Orientation orient, QWidget* parent) {
    return newToast(title, content, duration, isClosable, position, orient,
                    color(ToastColor::Error), parent);
}
Toast* Toast::custom(const QString& title, const QString& content, const QColor& toastColor,
                     const QColor& backgroundColor, QWidget* parent, int duration, bool isClosable,
                     ToastPosition position, Qt::Orientation orient) {
    return newToast(title, content, duration, isClosable, position, orient, toastColor, parent,
                    backgroundColor);
}
QString Toast::title() const { return title_; }
void Toast::setTitle(const QString& title) {
    title_ = title;
    titleLabel->setText(title);
    titleLabel->setVisible(!title.isEmpty() && !leadingWidget_);
    updateSize();
    if (stack_) stack_->requestReflow();
}
QString Toast::content() const { return content_; }
void Toast::setContent(const QString& content) {
    content_ = content;
    contentLabel->setText(content);
    contentLabel->setVisible(!content.isEmpty());
    updateSize();
    if (stack_) stack_->requestReflow();
}
int Toast::duration() const { return duration_; }
void Toast::setDuration(int duration) {
    duration_ = duration;
    closeTimer_->stop();
    if (!closing_ && stack_ && stack_->contains(this) && duration >= 0)
        closeTimer_->start(duration);
}
bool Toast::isClosable() const { return closable_; }
void Toast::setClosable(bool closable) {
    closable_ = closable;
    closeButton->setVisible(closable);
    updateSize();
    if (stack_) stack_->requestReflow();
}
ToastPosition Toast::position() const { return position_; }
void Toast::setPosition(ToastPosition position) {
    positionIndex(position);
    if (position == position_) return;
    const bool active = stack_ && stack_->contains(this);
    if (active) stack_->remove(this);
    position_ = position;
    if (active) stack_->add(this, false);
}
Qt::Orientation Toast::orientation() const { return orient_; }
QColor Toast::statusColor() const { return useThemeColor_ ? themeColor() : toastColor_; }
void Toast::setToastColor(const QColor& color) {
    if (!color.isValid()) throw std::invalid_argument("Invalid toast color");
    toastColor_ = color;
    useThemeColor_ = false;
    update();
}
QColor Toast::backgroundColor() const {
    return backgroundColor_.isValid() ? backgroundColor_ : defaultBackgroundColor();
}
QColor Toast::defaultBackgroundColor() const {
    return QColor(isDarkTheme() ? "#323232" : "#F2F2F2");
}
void Toast::setBackgroundColor(const QColor& color) {
    backgroundColor_ = color;
    update();
}
void Toast::setUseThemeColor(bool enabled) {
    useThemeColor_ = enabled;
    update();
}
bool Toast::usesThemeColor() const { return useThemeColor_; }
void Toast::addWidget(QWidget* widget, int stretch, Qt::Alignment alignment) {
    if (!widget || widget == this || widget == titleLabel || widget == contentLabel ||
        widget == closeButton || widget == leadingWidget_ || widget->isAncestorOf(this))
        throw std::invalid_argument("Toast requires a new content widget");
    if (widgetLayout->indexOf(widget) >= 0) return;
    widgetLayout->addSpacing(6);
    widgetLayout->addWidget(widget, stretch, alignment);
    updateSize();
    if (stack_) stack_->requestReflow();
}
void Toast::setAnimationDuration(int duration) {
    if (duration < 0 || duration > 60000)
        throw std::invalid_argument("Invalid toast animation duration");
    animationDuration_ = duration;
    positionAnimation_->setDuration(duration);
}
int Toast::animationDuration() const { return animationDuration_; }
QPoint Toast::targetPosition() const { return stack_ ? stack_->target(this) : pos(); }
void Toast::configureCompact(QWidget* leadingWidget, const QMargins& margins,
                             const QSize& preferredMinimum, bool reserveCloseGap) {
    if (!leadingWidget || leadingWidget_ || orient_ != Qt::Horizontal)
        throw std::invalid_argument("Compact toast requires a horizontal leading widget");
    leadingWidget_ = leadingWidget;
    compactMinimum_ = preferredMinimum;
    compactMargins_ = margins;
    reserveCloseGap_ = reserveCloseGap;
    delete textLayout_->takeAt(0);  // Hidden title, not its widget.
    delete textLayout_->takeAt(0);  // Title/content separator.
    titleLabel->hide();
    textLayout_->setAlignment(Qt::AlignVCenter);
    textLayout_->setAlignment(contentLabel, Qt::AlignVCenter);
    row_->setContentsMargins(
        layoutDirection() == Qt::RightToLeft
            ? QMargins(margins.right(), margins.top(), margins.left(), margins.bottom())
            : margins);
    row_->insertWidget(0, leadingWidget, 0, Qt::AlignVCenter);
    row_->insertSpacing(1, 10);
    if (!reserveCloseGap) delete row_->takeAt(4);
    row_->setAlignment(closeButton, Qt::AlignVCenter);
    updateSize();
}
void Toast::updateSize() {
    if (!parentWidget() || !row_) return;
    const int limit = std::max(1, parentWidget()->width() - 48);
    const int textBudget = std::min(720, std::max(1, parentWidget()->width() * 2 / 3));
    titleLabel->ensurePolished();
    contentLabel->ensurePolished();
    const int titleWidth = labelTextSize(titleLabel).width();
    const int contentWidth = labelTextSize(contentLabel).width();
    if (leadingWidget_) {
        const auto margins = row_->contentsMargins();
        const int extras = widgetLayout->isEmpty() ? 0 : widgetLayout->sizeHint().width();
        const int overhead = margins.left() + margins.right() + leadingWidget_->width() + 10 +
                             (reserveCloseGap_ ? 12 : 0) + (closable_ ? 28 : 0) + extras;
        const int width = std::min(limit, std::max(compactMinimum_.width(),
                                                   std::min(contentWidth, textBudget) + overhead));
        setFixedWidth(width);
        contentLabel->setFixedHeight(
            labelTextSize(contentLabel, std::max(1, width - overhead)).height());
        row_->activate();
        const int height =
            row_->hasHeightForWidth() ? row_->heightForWidth(width) : row_->sizeHint().height();
        resize(width, std::max(compactMinimum_.height(), height));
        return;
    }
    const qint64 textWidth = orient_ == Qt::Horizontal ? qint64(titleWidth) + contentWidth + 17
                                                       : std::max(titleWidth, contentWidth);
    const int extra = widgetLayout->isEmpty() ? 0 : widgetLayout->sizeHint().width();
    const qint64 desired = std::min<qint64>(textWidth, textBudget) + (closable_ ? 76 : 48) +
                           (orient_ == Qt::Horizontal ? extra : 0);
    const int width =
        int(std::min<qint64>(limit, std::max<qint64>(100, std::max<qint64>(desired, extra + 36))));
    setFixedWidth(width);
    if (orient_ == Qt::Vertical) {
        // QLabel's wrapped sizeHint can reserve two rows even when the final
        // column fits one. Use the actual column width for each visible label.
        const int textWidth = std::max(1, width - (closable_ ? 76 : 48));
        titleLabel->setFixedHeight(labelTextSize(titleLabel, textWidth).height());
        contentLabel->setFixedHeight(labelTextSize(contentLabel, textWidth).height());
    }
    row_->activate();
    const int height =
        row_->hasHeightForWidth() ? row_->heightForWidth(width) : row_->sizeHint().height();
    resize(width, std::max(34, height));
}
void Toast::animateTo(const QPoint& target, bool animate) {
    positionAnimation_->stop();
    if (!animate || animationDuration_ == 0 || pos() == target) {
        move(target);
        return;
    }
    positionAnimation_->setStartValue(pos());
    positionAnimation_->setEndValue(target);
    positionAnimation_->start();
}
bool Toast::event(QEvent* event) {
    const bool result = QFrame::event(event);
    if (event->type() == QEvent::LayoutDirectionChange && leadingWidget_) {
        const auto& margins = compactMargins_;
        row_->setContentsMargins(
            layoutDirection() == Qt::RightToLeft
                ? QMargins(margins.right(), margins.top(), margins.left(), margins.bottom())
                : margins);
    }
    if (event->type() == QEvent::ParentChange) {
        if (stack_) stack_->remove(this);
        stack_ = parentWidget() ? ToastStack::get(parentWidget()) : nullptr;
    }
    if (event->type() == QEvent::LayoutRequest && !closing_ && stack_ && stack_->contains(this))
        stack_->requestReflow();
    return result;
}
void Toast::showEvent(QShowEvent* event) {
    QFrame::showEvent(event);
    if (closing_ || !stack_ || stack_->contains(this)) return;
    stack_->add(this);
    raise();
    if (duration_ >= 0) closeTimer_->start(duration_);
}
void Toast::hideEvent(QHideEvent* event) {
    if (isHidden()) {
        closeTimer_->stop();
        positionAnimation_->stop();
        if (stack_) stack_->remove(this);
    }
    QFrame::hideEvent(event);
}
void Toast::closeEvent(QCloseEvent* event) {
    event->accept();
    if (closing_) return;
    closing_ = true;
    closeTimer_->stop();
    positionAnimation_->stop();
    if (stack_) stack_->remove(this);
    QFrame::closeEvent(event);
    deleteLater();
    emit closed();
}
void Toast::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QPainterPath shape;
    shape.addRoundedRect(QRectF(rect()), 8, 8);
    painter.setClipPath(shape);
    painter.fillRect(rect(), backgroundColor());
    painter.fillRect(QRectF(0, 0, width(), 4), statusColor());
}
}  // namespace qfw
