#include "components/dialog_box/waiting_dialog.h"

#include <QApplication>
#include <QEasingCurve>
#include <QHideEvent>
#include <QKeyEvent>
#include <QShowEvent>
#include <stdexcept>

#include "common/config.h"
#include "common/style_sheet.h"

namespace qfw {
namespace {
QWidget* requiredOwner(QWidget* parent) {
    if (!parent) throw std::invalid_argument("WaitingDialog requires a parent window");
    return parent->window();
}
}  // namespace

WaitingDialog::WaitingDialog(QWidget* parent) : WaitingDialog(QString(), QString(), parent) {}

WaitingDialog::WaitingDialog(const QString& title, const QString& content, QWidget* parent)
    : MaskDialogBase(requiredOwner(parent)), owner_(parentWidget()) {
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setWindowModality(Qt::WindowModal);
    setFocusPolicy(Qt::StrongFocus);
    setLayoutDirection(owner_->layoutDirection());
    setProperty("waitingDialog", true);
    windowMask->setObjectName("waitingDialogMask");
    windowMask->lower();

    progressRing = new IndeterminateProgressRing(widget, false);
    progressRing->setFixedSize(56, 56);
    progressRing->setStrokeWidth(6);
    progressRing->setAttribute(Qt::WA_TransparentForMouseEvents);
    titleLabel = new QLabel(widget);
    contentLabel = new QLabel(widget);
    titleLabel->setObjectName("titleLabel");
    contentLabel->setObjectName("contentLabel");
    for (auto* label : {titleLabel, contentLabel}) {
        label->setTextFormat(Qt::PlainText);
        label->setWordWrap(true);
        label->setMinimumWidth(0);
        label->setFocusPolicy(Qt::NoFocus);
    }
    textLayout_ = new QVBoxLayout;
    textLayout_->setContentsMargins(0, 0, 0, 0);
    textLayout_->setSpacing(10);
    textLayout_->setAlignment(Qt::AlignVCenter);
    textLayout_->addWidget(titleLabel);
    textLayout_->addWidget(contentLabel);
    contentLayout_ = new QHBoxLayout(widget);
    contentLayout_->setContentsMargins(39, 32, 29, 32);
    contentLayout_->setSpacing(30);
    contentLayout_->addWidget(progressRing, 0, Qt::AlignVCenter);
    contentLayout_->addLayout(textLayout_, 1);
    hBoxLayout->setContentsMargins(0, 0, 0, 0);
    hBoxLayout->setSizeConstraint(QLayout::SetNoConstraint);
    hBoxLayout->removeWidget(widget);
    hBoxLayout->addWidget(widget, 0, Qt::AlignCenter);
    qfw::setStyleSheet(this, FluentStyleSheet::Dialog);
    const QString transparent =
        QStringLiteral("QDialog[waitingDialog=true] { background: transparent; }");
    qfw::setCustomStyleSheet(this, transparent, transparent);
    setMaskColor(QColor(0, 0, 0, 76));

    opacityAnimation_ = new QPropertyAnimation(this);
    connect(opacityAnimation_, &QPropertyAnimation::finished, this, &WaitingDialog::finishFade);
    connect(&QConfig::instance(), &QConfig::themeColorChanged, progressRing,
            [this] { progressRing->update(); });
    connect(&QConfig::instance(), &QConfig::themeChangedFinished, this, [this] {
        progressRing->update();
        fitToOwner();
    });
    setTitle(title.isNull() ? tr("Please wait...") : title);
    setContent(content);
}

WaitingDialog::~WaitingDialog() { stopWaiting(); }
QString WaitingDialog::title() const { return titleLabel->text(); }
void WaitingDialog::setTitle(const QString& title) {
    titleLabel->setText(title);
    titleLabel->setVisible(!title.isEmpty());
    setWindowTitle(title);
    fitToOwner();
}
QString WaitingDialog::content() const { return contentLabel->text(); }
void WaitingDialog::setContent(const QString& content) {
    contentLabel->setText(content);
    contentLabel->setVisible(!content.isEmpty());
    fitToOwner();
}
QFrame* WaitingDialog::panel() const { return widget; }

void WaitingDialog::fitToOwner() {
    if (!owner_ || !contentLayout_) return;
    setGeometry(QRect(owner_->mapToGlobal(QPoint()), owner_->size()));
    const int width = qMin(300, qMax(1, owner_->width() - 24));
    const bool compact = width < 280;
    const int left = compact ? 16 : 39;
    const int right = compact ? 16 : 29;
    const int gap = compact ? 16 : 30;
    const int ringSize = compact ? 40 : 56;
    progressRing->setFixedSize(ringSize, ringSize);
    contentLayout_->setContentsMargins(isRightToLeft() ? right : left, 32,
                                       isRightToLeft() ? left : right, 32);
    contentLayout_->setSpacing(gap);
    const int textWidth = qMax(1, width - left - right - gap - ringSize - 2);
    int textHeight = 0;
    int labels = 0;
    for (auto* label : {titleLabel, contentLabel}) {
        if (label->isHidden()) continue;
        label->ensurePolished();
        textHeight += qMax(0, label->heightForWidth(textWidth));
        ++labels;
    }
    if (labels > 1) textHeight += textLayout_->spacing();
    const int height = qMax(132, qMax(ringSize, textHeight) + 66);
    widget->setFixedSize(width, qMin(height, qMax(1, owner_->height() - 24)));
    hBoxLayout->activate();
}

void WaitingDialog::animateOpacity(qreal start, qreal end, int duration) {
    // One owned animation prevents a late fade-in callback from clearing a
    // newer fade-out effect when Esc is pressed immediately after opening.
    opacityAnimation_->stop();
    opacityAnimation_->setTargetObject(nullptr);
    auto* effect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(effect);
    opacityAnimation_->setTargetObject(effect);
    opacityAnimation_->setPropertyName("opacity");
    opacityAnimation_->setStartValue(start);
    opacityAnimation_->setEndValue(end);
    opacityAnimation_->setDuration(duration);
    opacityAnimation_->setEasingCurve(QEasingCurve::InSine);
    opacityAnimation_->start();
}
void WaitingDialog::showEvent(QShowEvent* event) {
    closing_ = false;
    setLayoutDirection(owner_->layoutDirection());
    fitToOwner();
    setShadowEffect(60, QPoint(0, 10), QColor(0, 0, 0, 50));
    progressRing->start();
    qApp->installEventFilter(this);
    filterInstalled_ = true;
    animateOpacity(0, 1, 200);
    QDialog::showEvent(event);
    setFocus(Qt::OtherFocusReason);
}
void WaitingDialog::stopWaiting() {
    if (progressRing) progressRing->stop();
    if (filterInstalled_ && qApp) qApp->removeEventFilter(this);
    filterInstalled_ = false;
}
void WaitingDialog::done(int code) {
    if (closing_) return;
    closing_ = true;
    resultCode_ = code;
    stopWaiting();
    if (!isVisible()) {
        QDialog::done(code);
        return;
    }
    auto* effect = qobject_cast<QGraphicsOpacityEffect*>(graphicsEffect());
    const qreal opacity = effect ? effect->opacity() : 1;
    widget->setGraphicsEffect(nullptr);
    animateOpacity(opacity, 0, 100);
}
void WaitingDialog::finishFade() {
    opacityAnimation_->setTargetObject(nullptr);
    setGraphicsEffect(nullptr);
    if (closing_) QDialog::done(resultCode_);
}
void WaitingDialog::hideEvent(QHideEvent* event) {
    stopWaiting();
    opacityAnimation_->stop();
    opacityAnimation_->setTargetObject(nullptr);
    setGraphicsEffect(nullptr);
    QDialog::hideEvent(event);
}
void WaitingDialog::changeEvent(QEvent* event) {
    QDialog::changeEvent(event);
    if (event->type() == QEvent::FontChange || event->type() == QEvent::LayoutDirectionChange)
        fitToOwner();
}
bool WaitingDialog::eventFilter(QObject* obj, QEvent* event) {
    if (filterInstalled_) {
        if (obj == owner_) {
            if (event->type() == QEvent::LayoutDirectionChange)
                setLayoutDirection(owner_->layoutDirection());
            if (event->type() == QEvent::Resize || event->type() == QEvent::Move)
                fitToOwner();
            else if (event->type() == QEvent::Hide || event->type() == QEvent::Close)
                reject();
        }
        auto* target = qobject_cast<QWidget*>(obj);
        const bool inside = target && (target == this || isAncestorOf(target));
        if (inside && event->type() == QEvent::ShortcutOverride) {
            event->accept();
            return true;
        }
        if (inside && event->type() == QEvent::KeyPress) {
            auto* key = static_cast<QKeyEvent*>(event);
            if (key->key() == Qt::Key_Escape) {
                event->accept();
                if (!key->isAutoRepeat()) reject();
                return true;
            }
        }
    }
    return MaskDialogBase::eventFilter(obj, event);
}

}  // namespace qfw
