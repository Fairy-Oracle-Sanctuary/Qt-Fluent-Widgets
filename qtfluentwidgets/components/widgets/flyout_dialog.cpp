#include "components/widgets/flyout_dialog.h"

#include <QApplication>
#include <QCloseEvent>
#include <QHideEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QShowEvent>
#include <memory>
#include <stdexcept>

#include "common/config.h"
#include "common/qtcompat.h"
#include "common/style_sheet.h"
#include "components/widgets/button.h"
#include "components/widgets/label.h"

namespace qfw {
namespace {
class DialogFlyout : public Flyout {
public:
    DialogFlyout(FlyoutDialog* view, QWidget* parent)
        : Flyout(view, parent), view_(view), owner_(parent ? parent->window() : nullptr) {}

protected:
    bool eventFilter(QObject* watched, QEvent* event) override {
        if (isVisible()) {
            if (watched == owner_ &&
                (event->type() == QEvent::Hide || event->type() == QEvent::Close)) {
                close();
                return false;
            }
            if (event->type() == QEvent::MouseButtonPress &&
                outsideView(static_cast<QMouseEvent*>(event))) {
                close();
                return true;
            }
            auto* widget = qobject_cast<QWidget*>(watched);
            if (widget && (widget == this || isAncestorOf(widget)) &&
                event->type() == QEvent::KeyPress &&
                static_cast<QKeyEvent*>(event)->key() == Qt::Key_Escape) {
                event->accept();
                if (view_) view_->reject();
                return true;
            }
        }
        return Flyout::eventFilter(watched, event);
    }
    void mousePressEvent(QMouseEvent* event) override {
        if (outsideView(event)) {
            event->accept();
            close();
        } else
            Flyout::mousePressEvent(event);
    }
    void showEvent(QShowEvent* event) override {
        closing_ = false;
        qApp->installEventFilter(this);
        Flyout::showEvent(event);
    }
    void closeEvent(QCloseEvent* event) override {
        if (closing_) {
            event->accept();
            return;
        }
        closing_ = true;
        qApp->removeEventFilter(this);
        Flyout::closeEvent(event);
    }
    void hideEvent(QHideEvent* event) override {
        Flyout::hideEvent(event);
        // Native popup dismissal can hide the window without sending closeEvent.
        if (!closing_) close();
    }

private:
    bool outsideView(QMouseEvent* event) const {
        return view_ && !view_->rect().contains(
                            view_->mapFromGlobal(event->QMouseEvent_globalPosition_toPoint()));
    }
    QPointer<FlyoutDialog> view_;
    QPointer<QWidget> owner_;
    bool closing_ = false;
};
}  // namespace
FlyoutDialog::FlyoutDialog(const QString& title, const QString& content, QWidget* parent)
    : FlyoutViewBase(parent) {
    vBoxLayout = new QVBoxLayout(this);
    vBoxLayout->setContentsMargins(1, 1, 1, 1);
    vBoxLayout->setSpacing(0);
    contentWidget = new QWidget(this);
    viewLayout = new QVBoxLayout(contentWidget);
    viewLayout->setContentsMargins(15, 15, 15, 15);
    viewLayout->setSpacing(8);
    titleLabel = new SubtitleLabel(title, contentWidget);
    contentLabel = new BodyLabel(content, contentWidget);
    contentLabel->setWordWrap(true);
    titleLabel->setVisible(!title.isEmpty());
    contentLabel->setVisible(!content.isEmpty());
    viewLayout->addWidget(titleLabel);
    viewLayout->addWidget(contentLabel);
    buttonBar = new QWidget(this);
    buttonBar->setFixedHeight(40);
    buttonLayout = new QHBoxLayout(buttonBar);
    buttonLayout->setContentsMargins(4, 4, 4, 4);
    buttonLayout->setSpacing(0);
    yesButton = new TransparentToolButton(FluentIconEnum::Accept, buttonBar);
    cancelButton = new TransparentToolButton(FluentIconEnum::Close, buttonBar);
    yesButton->setAccessibleName(tr("Confirm"));
    cancelButton->setAccessibleName(tr("Cancel"));
    for (auto* button : {yesButton, cancelButton}) {
        button->setIconSize(QSize(16, 16));
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        buttonLayout->addWidget(button, 1);
    }
    vBoxLayout->addWidget(contentWidget);
    vBoxLayout->addWidget(buttonBar);
    setMinimumWidth(256);
    setFocusPolicy(Qt::StrongFocus);
    connect(yesButton, &QAbstractButton::clicked, this, &FlyoutDialog::accept);
    connect(cancelButton, &QAbstractButton::clicked, this, &FlyoutDialog::reject);
    connect(&QConfig::instance(), &QConfig::themeChangedFinished, this, [this] { update(); });
}
void FlyoutDialog::addWidget(QWidget* widget, int stretch, Qt::Alignment align) {
    if (widget) viewLayout->addWidget(widget, stretch, align);
}
QSize FlyoutDialog::sizeHint() const {
    const auto hint = vBoxLayout->totalSizeHint();
    int textWidth = 0;
    for (const auto& line : contentLabel->text().split('\n'))
        textWidth = qMax(textWidth, contentLabel->fontMetrics().horizontalAdvance(line));
    const auto margins = viewLayout->contentsMargins();
    const int width = qMax(qMax(minimumWidth(), hint.width()),
                           qMin(480, textWidth + margins.left() + margins.right() + 3));
    const int height = vBoxLayout->totalHeightForWidth(width);
    return QSize(width, height >= 0 ? height : hint.height());
}
QColor FlyoutDialog::borderColor() const {
    return isDarkTheme() ? QColor(255, 255, 255, 20) : FlyoutViewBase::borderColor();
}
Flyout* FlyoutDialog::flyout() const { return flyout_; }
void FlyoutDialog::bindFlyout(Flyout* popup) {
    if (flyout_ == popup) return;
    flyout_ = popup;
    connect(this, &FlyoutDialog::closed, popup, &QWidget::close);
    QPointer<FlyoutDialog> guard(this);
    connect(popup, &Flyout::closed, this, [guard] {
        if (guard && !guard->resolved_) guard->reject();
    });
    connect(this, &QObject::destroyed, popup, &QWidget::close);
}
Flyout* FlyoutDialog::showAt(QWidget* target, QWidget* parent, FlyoutAnimationType type) {
    return showAt(target ? QVariant::fromValue(target) : QVariant(), parent, type);
}
Flyout* FlyoutDialog::showAt(const QVariant& target, QWidget* parent, FlyoutAnimationType type) {
    if (flyout_) return flyout_;
    QWidget* anchor = nullptr;
    if (target.isValid() && !target.canConvert<QPoint>()) {
        if (!target.canConvert<QWidget*>() || !(anchor = target.value<QWidget*>()))
            throw std::invalid_argument("FlyoutDialog target must be a widget or a point");
    }
    resolved_ = false;
    auto* popup = new DialogFlyout(this, parent);
    bindFlyout(popup);
    if (anchor) connect(anchor, &QObject::destroyed, popup, &QWidget::close);
    if (target.isValid()) {
        QPointer<FlyoutDialog> guard(this);
        QPointer<Flyout> popupGuard(popup);
        popup->show();
        if (!guard || !popupGuard) return nullptr;
        QPoint position;
        if (anchor) {
            std::unique_ptr<FlyoutAnimationManager> manager(
                FlyoutAnimationManager::make(type, popup));
            position = manager->position(anchor);
        } else
            position = target.toPoint();
        popup->exec(position, type);
        if (!guard || !popupGuard) return nullptr;
        setFocus(Qt::PopupFocusReason);
        return popupGuard;
    }
    return popup;
}
void FlyoutDialog::accept() { finish(true); }
void FlyoutDialog::reject() { finish(false); }
void FlyoutDialog::finish(bool acceptedResult) {
    if (resolved_) return;
    resolved_ = true;
    QPointer<FlyoutDialog> guard(this);
    if (acceptedResult)
        emit accepted();
    else
        emit rejected();
    if (guard) emit closed();
}
void FlyoutDialog::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter ||
        event->key() == Qt::Key_Escape) {
        event->accept();
        if (event->key() == Qt::Key_Escape)
            reject();
        else
            accept();
    } else
        FlyoutViewBase::keyPressEvent(event);
}
void FlyoutDialog::showEvent(QShowEvent* event) {
    resolved_ = false;
    if (auto* popup = qobject_cast<Flyout*>(parentWidget())) bindFlyout(popup);
    FlyoutViewBase::showEvent(event);
}
void FlyoutDialog::paintEvent(QPaintEvent* event) {
    FlyoutViewBase::paintEvent(event);
    QPainter painter(this);
    painter.fillRect(1, buttonBar->y(), width() - 2, 2,
                     isDarkTheme() ? QColor(255, 255, 255, 15) : QColor(0, 0, 0, 17));
}
}  // namespace qfw
