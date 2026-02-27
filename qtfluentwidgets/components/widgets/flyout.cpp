#include "components/widgets/flyout.h"

#include <QAbstractButton>
#include <QApplication>
#include <QCloseEvent>
#include <QCursor>
#include <QEasingCurve>
#include <QMouseEvent>
#include <QPainter>
#include <QScreen>
#include <memory>

#include "common/auto_wrap.h"
#include "common/config.h"
#include "common/screen.h"
#include "common/style_sheet.h"
#include "components/widgets/button.h"
#include "components/widgets/label.h"

namespace qfw {

namespace {

int clampInt(int v, int lo, int hi) { return qMax(lo, qMin(v, hi)); }

class IconWidget : public QWidget {
public:
    explicit IconWidget(QWidget* parent = nullptr) : QWidget(parent) { setFixedSize(36, 54); }

    void setIcon(const QIcon& icon) {
        icon_ = icon;
        update();
    }

protected:
    void paintEvent(QPaintEvent* e) override {
        Q_UNUSED(e);

        if (icon_.isNull()) {
            return;
        }

        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

        QRect rect(8, static_cast<int>((height() - 20) / 2.0), 20, 20);
        QPixmap pm = icon_.pixmap(rect.size(), QIcon::Normal, QIcon::Off);
        painter.drawPixmap(rect, pm);
    }

private:
    QIcon icon_;
};

}  // namespace

FlyoutViewBase::FlyoutViewBase(QWidget* parent) : QWidget(parent) {
    setAttribute(Qt::WA_StyledBackground);
    setAttribute(Qt::WA_TranslucentBackground);
}

QColor FlyoutViewBase::backgroundColor() const {
    return isDarkTheme() ? QColor(40, 40, 40) : QColor(248, 248, 248);
}

QColor FlyoutViewBase::borderColor() const {
    return isDarkTheme() ? QColor(0, 0, 0, 45) : QColor(0, 0, 0, 17);
}

void FlyoutViewBase::paintEvent(QPaintEvent* e) {
    Q_UNUSED(e);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);

    painter.setBrush(backgroundColor());
    painter.setPen(borderColor());

    QRect rect = this->rect().adjusted(1, 1, -1, -1);
    painter.drawRoundedRect(rect, 8, 8);
}

FlyoutView::FlyoutView(const QString& title, const QString& content, QWidget* parent)
    : FlyoutViewBase(parent), title_(title), content_(content) {
    initWidgets();
}

FlyoutView::FlyoutView(const QString& title, const QString& content, const QIcon& icon,
                       QWidget* parent)
    : FlyoutViewBase(parent), title_(title), content_(content), icon_(icon) {
    initWidgets();
}

FlyoutView::FlyoutView(const QString& title, const QString& content, const FluentIconBase& icon,
                       QWidget* parent)
    : FlyoutViewBase(parent), title_(title), content_(content), icon_(toQIcon(icon)) {
    initWidgets();
}

FlyoutView::FlyoutView(const QString& title, const QString& content, const QString& image,
                       QWidget* parent)
    : FlyoutViewBase(parent), title_(title), content_(content) {
    initWidgets();
    setImage(image);
}

void FlyoutView::setIcon(const QIcon& icon) {
    icon_ = icon;

    if (auto* w = dynamic_cast<IconWidget*>(iconWidget_)) {
        w->setIcon(icon_);
        w->setHidden(icon_.isNull());
    }

    update();
}

void FlyoutView::setImage(const QString& imagePath) {
    if (imageLabel_) {
        imageLabel_->setImage(imagePath);
        addImageToLayout();
        adjustImage();
        adjustSize();
    }
}

void FlyoutView::setImage(const QImage& image) {
    if (imageLabel_) {
        imageLabel_->setImage(image);
        addImageToLayout();
        adjustImage();
        adjustSize();
    }
}

void FlyoutView::setImage(const QPixmap& pixmap) {
    if (imageLabel_) {
        imageLabel_->setImage(pixmap);
        addImageToLayout();
        adjustImage();
        adjustSize();
    }
}

void FlyoutView::setClosable(bool closable) {
    closable_ = closable;
    if (closeButton_) {
        closeButton_->setVisible(closable_);
    }

    if (viewLayout_) {
        QMargins margins(6, 5, 6, 5);
        margins.setLeft(icon_.isNull() ? 20 : 5);
        margins.setRight(closable_ ? 6 : 20);
        viewLayout_->setContentsMargins(margins);
    }
}

bool FlyoutView::isClosable() const { return closable_; }

QVBoxLayout* FlyoutView::widgetLayout() const { return widgetLayout_; }

void FlyoutView::addWidget(QWidget* widget, int stretch, Qt::Alignment align) {
    if (!widgetLayout_ || !widget) {
        return;
    }

    widgetLayout_->addSpacing(8);
    widgetLayout_->addWidget(widget, stretch, align);
}

void FlyoutView::showEvent(QShowEvent* e) {
    FlyoutViewBase::showEvent(e);
    adjustImage();
    adjustSize();
}

void FlyoutView::initWidgets() {
    vBoxLayout_ = new QVBoxLayout(this);
    viewLayout_ = new QHBoxLayout();
    widgetLayout_ = new QVBoxLayout();

    titleLabel_ = new QLabel(title_, this);
    contentLabel_ = new QLabel(content_, this);

    auto* iconWidget = new IconWidget(this);
    iconWidget->setIcon(icon_);
    iconWidget_ = iconWidget;

    imageLabel_ = new ImageLabel(this);

    const QIcon closeIcon = toQIcon(FluentIcon(FluentIconEnum::Close));
    closeButton_ = new TransparentToolButton(closeIcon, this);

    closeButton_->setFixedSize(32, 32);
    closeButton_->setIconSize(QSize(12, 12));
    closeButton_->setVisible(closable_);

    titleLabel_->setVisible(!title_.isEmpty());
    contentLabel_->setVisible(!content_.isEmpty());
    iconWidget_->setHidden(icon_.isNull());

    connect(closeButton_, &QAbstractButton::clicked, this, &FlyoutView::closed);

    titleLabel_->setObjectName(QStringLiteral("titleLabel"));
    contentLabel_->setObjectName(QStringLiteral("contentLabel"));

    qfw::setStyleSheet(this, qfw::FluentStyleSheet::TeachingTip);

    initLayout();
}

void FlyoutView::initLayout() {
    if (!vBoxLayout_ || !viewLayout_ || !widgetLayout_) {
        return;
    }

    vBoxLayout_->setContentsMargins(1, 1, 1, 1);
    widgetLayout_->setContentsMargins(0, 8, 0, 8);
    viewLayout_->setSpacing(4);
    widgetLayout_->setSpacing(0);
    vBoxLayout_->setSpacing(0);

    if (title_.isEmpty() || content_.isEmpty()) {
        if (iconWidget_) {
            iconWidget_->setFixedHeight(36);
        }
    }

    vBoxLayout_->addLayout(viewLayout_);
    if (iconWidget_) {
        viewLayout_->addWidget(iconWidget_, 0, Qt::AlignTop);
    }

    adjustText();
    widgetLayout_->addWidget(titleLabel_);
    widgetLayout_->addWidget(contentLabel_);
    viewLayout_->addLayout(widgetLayout_);

    if (closeButton_) {
        viewLayout_->addWidget(closeButton_, 0, Qt::AlignRight | Qt::AlignTop);
    }

    QMargins margins(6, 5, 6, 5);
    margins.setLeft(icon_.isNull() ? 20 : 5);
    margins.setRight(closable_ ? 6 : 20);
    viewLayout_->setContentsMargins(margins);

    addImageToLayout();
}

void FlyoutView::adjustText() {
    QRect rect = getCurrentScreenGeometry(true);
    const int w = qMin(900, rect.width() - 200);

    int titleChars = clampInt(static_cast<int>(w / 10.0), 30, 120);
    int contentChars = clampInt(static_cast<int>(w / 9.0), 30, 120);

    if (titleLabel_) {
        titleLabel_->setText(TextWrap::wrap(title_, titleChars, false).first);
    }
    if (contentLabel_) {
        contentLabel_->setText(TextWrap::wrap(content_, contentChars, false).first);
    }
}

void FlyoutView::adjustImage() {
    if (!imageLabel_ || !vBoxLayout_ || imageLabel_->isNull()) {
        return;
    }

    // Limit image width to screen width minus margins
    QRect screenRect = getCurrentScreenGeometry(true);
    const int maxWidth = screenRect.width() - 200;

    // Use a reasonable default width
    const int w = qMax(200, qMin(400, maxWidth));
    imageLabel_->scaledToWidth(w);
}

void FlyoutView::addImageToLayout() {
    if (!imageLabel_ || !vBoxLayout_) {
        return;
    }

    imageLabel_->setBorderRadius(8, 8, 0, 0);
    imageLabel_->setHidden(imageLabel_->isNull());

    if (vBoxLayout_->indexOf(imageLabel_) < 0) {
        vBoxLayout_->insertWidget(0, imageLabel_);
    }
}

Flyout::Flyout(FlyoutViewBase* view, QWidget* parent, bool deleteOnClose)
    : QWidget(parent), view_(view), deleteOnClose_(deleteOnClose) {
    hBoxLayout_ = new QHBoxLayout(this);
    hBoxLayout_->setContentsMargins(15, 8, 15, 20);

    if (view_) {
        hBoxLayout_->addWidget(view_);
        setShadowEffect();
    }

    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);

    // Install event filter to handle click outside
    qApp->installEventFilter(this);
}

void Flyout::setShadowEffect(int blurRadius, const QPoint& offset) {
    if (!view_) {
        return;
    }

    QColor color(0, 0, 0, isDarkTheme() ? 80 : 30);
    auto* shadowEffect = new QGraphicsDropShadowEffect(view_);
    shadowEffect->setBlurRadius(blurRadius);
    shadowEffect->setOffset(offset);
    shadowEffect->setColor(color);

    view_->setGraphicsEffect(nullptr);
    view_->setGraphicsEffect(shadowEffect);
}

void Flyout::closeEvent(QCloseEvent* e) {
    if (deleteOnClose_) {
        deleteLater();
    }

    QWidget::closeEvent(e);
    emit closed();
}

void Flyout::showEvent(QShowEvent* e) {
    activateWindow();
    QWidget::showEvent(e);
}

bool Flyout::eventFilter(QObject* watched, QEvent* event) {
    if (isVisible() && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* me = static_cast<QMouseEvent*>(event);
        QPoint globalPos = me->globalPosition().toPoint();
        if (!rect().contains(mapFromGlobal(globalPos))) {
            // Clicked outside the flyout, close it
            close();
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void Flyout::exec(const QPoint& pos, FlyoutAnimationType aniType) {
    delete aniManager_;
    aniManager_ = FlyoutAnimationManager::make(aniType, this);

    show();

    if (aniManager_) {
        aniManager_->exec(pos);
    }
}

Flyout* Flyout::make(FlyoutViewBase* view, const QVariant& target, QWidget* parent,
                     FlyoutAnimationType aniType, bool deleteOnClose) {
    auto* w = new Flyout(view, parent, deleteOnClose);

    if (!target.isValid()) {
        return w;
    }

    w->show();

    QPoint p;

    if (target.canConvert<QPoint>()) {
        p = target.toPoint();
    } else if (target.canConvert<QWidget*>()) {
        QWidget* t = target.value<QWidget*>();
        if (t) {
            std::unique_ptr<FlyoutAnimationManager> m(FlyoutAnimationManager::make(aniType, w));
            if (m) {
                p = m->position(t);
            }
        }
    }

    w->exec(p, aniType);
    return w;
}

Flyout* Flyout::create(const QString& title, const QString& content, const QVariant& icon,
                       const QVariant& image, bool closable, const QVariant& target,
                       QWidget* parent, FlyoutAnimationType aniType, bool deleteOnClose) {
    auto* view = new FlyoutView(title, content, nullptr);
    view->setClosable(closable);

    if (icon.canConvert<QIcon>()) {
        view->setIcon(icon.value<QIcon>());
    }

    if (image.canConvert<QPixmap>()) {
        view->setImage(image.value<QPixmap>());
    } else if (image.canConvert<QImage>()) {
        view->setImage(image.value<QImage>());
    } else if (image.canConvert<QString>()) {
        view->setImage(image.toString());
    }

    Flyout* flyout = Flyout::make(view, target, parent, aniType, deleteOnClose);
    QObject::connect(view, &FlyoutView::closed, flyout, &Flyout::close);
    return flyout;
}

void Flyout::fadeOut() {
    fadeOutAni_ = new QPropertyAnimation(this, QByteArrayLiteral("windowOpacity"), this);
    connect(fadeOutAni_, &QPropertyAnimation::finished, this, &Flyout::close);
    fadeOutAni_->setStartValue(1.0);
    fadeOutAni_->setEndValue(0.0);
    fadeOutAni_->setDuration(120);
    fadeOutAni_->start();
}

FlyoutAnimationManager::FlyoutAnimationManager(Flyout* flyout) : QObject(flyout), flyout_(flyout) {
    group_ = new QParallelAnimationGroup(this);
    slideAni_ = new QPropertyAnimation(flyout, QByteArrayLiteral("pos"), this);
    opacityAni_ = new QPropertyAnimation(flyout, QByteArrayLiteral("windowOpacity"), this);

    slideAni_->setDuration(187);
    opacityAni_->setDuration(187);

    opacityAni_->setStartValue(0.0);
    opacityAni_->setEndValue(1.0);

    slideAni_->setEasingCurve(QEasingCurve::OutQuad);
    opacityAni_->setEasingCurve(QEasingCurve::OutQuad);

    group_->addAnimation(slideAni_);
    group_->addAnimation(opacityAni_);
}

QPoint FlyoutAnimationManager::adjustPosition(const QPoint& pos) const {
    const QRect rect = getCurrentScreenGeometry(true);

    const int w = flyout_ ? (flyout_->sizeHint().width() + 5) : 0;
    const int h = flyout_ ? (flyout_->sizeHint().height()) : 0;

    const int x = qMax(rect.left(), qMin(pos.x(), rect.right() - w));
    const int y = qMax(rect.top(), qMin(pos.y() - 4, rect.bottom() - h + 5));

    return QPoint(x, y);
}

FlyoutAnimationManager* FlyoutAnimationManager::make(FlyoutAnimationType type, Flyout* flyout) {
    switch (type) {
        case FlyoutAnimationType::PullUp:
            return new PullUpFlyoutAnimationManager(flyout);
        case FlyoutAnimationType::DropDown:
            return new DropDownFlyoutAnimationManager(flyout);
        case FlyoutAnimationType::SlideLeft:
            return new SlideLeftFlyoutAnimationManager(flyout);
        case FlyoutAnimationType::SlideRight:
            return new SlideRightFlyoutAnimationManager(flyout);
        case FlyoutAnimationType::FadeIn:
            return new FadeInFlyoutAnimationManager(flyout);
        case FlyoutAnimationType::None:
            return new DummyFlyoutAnimationManager(flyout);
    }

    return new DummyFlyoutAnimationManager(flyout);
}

QPoint PullUpFlyoutAnimationManager::position(QWidget* target) const {
    if (!flyout_ || !target) {
        return QPoint();
    }

    QPoint pos = target->mapToGlobal(QPoint());
    int x = pos.x() + target->width() / 2 - flyout_->sizeHint().width() / 2;
    int y = pos.y() - flyout_->sizeHint().height() + flyout_->layout()->contentsMargins().bottom();
    return QPoint(x, y);
}

void PullUpFlyoutAnimationManager::exec(const QPoint& pos) {
    const QPoint p = adjustPosition(pos);
    slideAni_->setStartValue(p + QPoint(0, 8));
    slideAni_->setEndValue(p);
    group_->start();
}

QPoint DropDownFlyoutAnimationManager::position(QWidget* target) const {
    if (!flyout_ || !target) {
        return QPoint();
    }

    QPoint pos = target->mapToGlobal(QPoint(0, target->height()));
    int x = pos.x() + target->width() / 2 - flyout_->sizeHint().width() / 2;
    int y = pos.y() - flyout_->layout()->contentsMargins().top() + 8;
    return QPoint(x, y);
}

void DropDownFlyoutAnimationManager::exec(const QPoint& pos) {
    const QPoint p = adjustPosition(pos);
    slideAni_->setStartValue(p - QPoint(0, 8));
    slideAni_->setEndValue(p);
    group_->start();
}

QPoint SlideLeftFlyoutAnimationManager::position(QWidget* target) const {
    if (!flyout_ || !target) {
        return QPoint();
    }

    QPoint pos = target->mapToGlobal(QPoint(0, 0));
    int x = pos.x() - flyout_->sizeHint().width() + 8;
    int y = pos.y() - flyout_->sizeHint().height() / 2 + target->height() / 2 +
            flyout_->layout()->contentsMargins().top();
    return QPoint(x, y);
}

void SlideLeftFlyoutAnimationManager::exec(const QPoint& pos) {
    const QPoint p = adjustPosition(pos);
    slideAni_->setStartValue(p + QPoint(8, 0));
    slideAni_->setEndValue(p);
    group_->start();
}

QPoint SlideRightFlyoutAnimationManager::position(QWidget* target) const {
    if (!flyout_ || !target) {
        return QPoint();
    }

    QPoint pos = target->mapToGlobal(QPoint(0, 0));
    int x = pos.x() + target->width() - 8;
    int y = pos.y() - flyout_->sizeHint().height() / 2 + target->height() / 2 +
            flyout_->layout()->contentsMargins().top();
    return QPoint(x, y);
}

void SlideRightFlyoutAnimationManager::exec(const QPoint& pos) {
    const QPoint p = adjustPosition(pos);
    slideAni_->setStartValue(p - QPoint(8, 0));
    slideAni_->setEndValue(p);
    group_->start();
}

FadeInFlyoutAnimationManager::FadeInFlyoutAnimationManager(Flyout* flyout)
    : FlyoutAnimationManager(flyout) {
    if (group_ && slideAni_) {
        group_->removeAnimation(slideAni_);
    }
}

QPoint FadeInFlyoutAnimationManager::position(QWidget* target) const {
    if (!flyout_ || !target) {
        return QPoint();
    }

    QPoint pos = target->mapToGlobal(QPoint());
    int x = pos.x() + target->width() / 2 - flyout_->sizeHint().width() / 2;
    int y = pos.y() - flyout_->sizeHint().height() + flyout_->layout()->contentsMargins().bottom();
    return QPoint(x, y);
}

void FadeInFlyoutAnimationManager::exec(const QPoint& pos) {
    if (flyout_) {
        flyout_->move(adjustPosition(pos));
    }
    group_->start();
}

QPoint DummyFlyoutAnimationManager::position(QWidget* target) const {
    if (!flyout_ || !target) {
        return QPoint();
    }

    QMargins m = flyout_->contentsMargins();
    if (auto* layout = flyout_->layout()) {
        m = layout->contentsMargins();
    }

    return target->mapToGlobal(QPoint(-m.left(), -flyout_->sizeHint().height() + m.bottom() - 8));
}

void DummyFlyoutAnimationManager::exec(const QPoint& pos) {
    if (!flyout_) {
        return;
    }
    flyout_->move(adjustPosition(pos));
}

}  // namespace qfw
