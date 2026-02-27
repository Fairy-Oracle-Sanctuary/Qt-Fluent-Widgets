#include "components/widgets/info_bar.h"

#include <QApplication>
#include <QCloseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QResizeEvent>
#include <QScreen>
#include <QShowEvent>

#include "common/style_sheet.h"
#include "components/widgets/button.h"

namespace qfw {

// ============================================================================
// InfoBarIcon
// ============================================================================

InfoBarIcon::InfoBarIcon(InfoBarIconEnum icon) : icon_(icon) {}

QString InfoBarIcon::path(Theme theme) const {
    QString color;
    if (theme == Theme::Auto) {
        color = isDarkTheme() ? "dark" : "light";
    } else {
        color = theme == Theme::Dark ? "dark" : "light";
    }

    QString iconName;
    switch (icon_) {
        case InfoBarIconEnum::Information:
            iconName = "Info";
            break;
        case InfoBarIconEnum::Success:
            iconName = "Success";
            break;
        case InfoBarIconEnum::Warning:
            iconName = "Warning";
            break;
        case InfoBarIconEnum::Error:
            iconName = "Error";
            break;
    }

    return QString(":/qfluentwidgets/images/info_bar/%1_%2.svg").arg(iconName, color);
}

void InfoBarIcon::render(QPainter* painter, const QRect& rect, Theme theme,
                         const QVariantMap& attributes) const {
    if (icon_ == InfoBarIconEnum::Information && !attributes.contains(QStringLiteral("fill")) &&
        !attributes.contains(QStringLiteral("stroke"))) {
        // Match Python: drawIcon(self.icon, ..., indexes=[0], fill=themeColor().name())
        const QString iconPath = path(theme);
        QVariantMap attrs;
        attrs.insert(QStringLiteral("fill"), themeColor().name());
        const QString svg = writeSvg(iconPath, {0}, attrs);
        if (!svg.isEmpty()) {
            drawSvgIcon(svg.toUtf8(), painter, rect);
            return;
        }
        drawSvgIcon(iconPath, painter, rect);
        return;
    }

    FluentIconBase::render(painter, rect, theme, attributes);
}

FluentIconBase* InfoBarIcon::clone() const { return new InfoBarIcon(*this); }

// ============================================================================
// InfoIconWidget
// ============================================================================

InfoIconWidget::InfoIconWidget(InfoBarIconEnum icon, QWidget* parent)
    : QWidget(parent), icon_(icon) {
    setFixedSize(36, 36);
}

void InfoIconWidget::setIcon(InfoBarIconEnum icon) {
    icon_ = icon;
    update();
}

void InfoIconWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QRect rect(10, 10, 15, 15);
    InfoBarIcon icon(icon_);
    icon.render(&painter, rect);
}

// ============================================================================
// InfoBar
// ============================================================================

InfoBar::InfoBar(InfoBarIconEnum icon, const QString& title, const QString& content,
                 Qt::Orientation orient, bool isClosable, int duration, InfoBarPosition position,
                 QWidget* parent)
    : QFrame(parent),
      title_(title),
      content_(content),
      orient_(orient),
      iconType_(icon),
      duration_(duration),
      isClosable_(isClosable),
      position_(position) {
    initWidget();
}

void InfoBar::initWidget() {
    titleLabel_ = new QLabel(this);
    contentLabel_ = new QLabel(this);

    static FluentIcon closeIcon(FluentIconEnum::Close);
    closeButton_ = new TransparentToolButton(closeIcon.qicon(), this);

    iconWidget_ = new InfoIconWidget(iconType_, this);

    hBoxLayout_ = new QHBoxLayout(this);
    textLayout_ = orient_ == Qt::Horizontal ? static_cast<QBoxLayout*>(new QHBoxLayout())
                                            : static_cast<QBoxLayout*>(new QVBoxLayout());
    widgetLayout_ = orient_ == Qt::Horizontal ? static_cast<QBoxLayout*>(new QHBoxLayout())
                                              : static_cast<QBoxLayout*>(new QVBoxLayout());

    opacityEffect_ = new QGraphicsOpacityEffect(this);
    opacityAni_ = new QPropertyAnimation(opacityEffect_, "opacity", this);

    opacityEffect_->setOpacity(1);
    setGraphicsEffect(opacityEffect_);

    closeButton_->setFixedSize(36, 36);
    closeButton_->setIconSize(QSize(12, 12));
    closeButton_->setCursor(Qt::PointingHandCursor);
    closeButton_->setVisible(isClosable_);

    setQss();
    initLayout();

    connect(closeButton_, &QToolButton::clicked, this, &InfoBar::close);
}

void InfoBar::initLayout() {
    hBoxLayout_->setContentsMargins(6, 6, 6, 6);
    hBoxLayout_->setSizeConstraint(QBoxLayout::SetMinimumSize);
    textLayout_->setSizeConstraint(QBoxLayout::SetMinimumSize);
    textLayout_->setAlignment(Qt::AlignTop);
    textLayout_->setContentsMargins(1, 10, 0, 8);

    hBoxLayout_->setSpacing(0);
    textLayout_->setSpacing(5);

    // Add icon to layout
    hBoxLayout_->addWidget(iconWidget_, 0, Qt::AlignTop | Qt::AlignLeft);

    // Add title to layout
    textLayout_->addWidget(titleLabel_, 1, Qt::AlignTop);
    titleLabel_->setVisible(!title_.isEmpty());

    // Add content label to layout
    if (orient_ == Qt::Horizontal) {
        textLayout_->addSpacing(7);
    }

    textLayout_->addWidget(contentLabel_, 1, Qt::AlignTop);
    contentLabel_->setVisible(!content_.isEmpty());
    hBoxLayout_->addLayout(textLayout_);

    // Add widget layout
    if (orient_ == Qt::Horizontal) {
        hBoxLayout_->addLayout(widgetLayout_);
        widgetLayout_->setSpacing(10);
    } else {
        textLayout_->addLayout(widgetLayout_);
    }

    // Add close button to layout
    hBoxLayout_->addSpacing(12);
    hBoxLayout_->addWidget(closeButton_, 0, Qt::AlignTop | Qt::AlignLeft);

    adjustText();
}

void InfoBar::setQss() {
    titleLabel_->setObjectName("titleLabel");
    contentLabel_->setObjectName("contentLabel");

    QString typeStr;
    switch (iconType_) {
        case InfoBarIconEnum::Information:
            typeStr = "Info";
            break;
        case InfoBarIconEnum::Success:
            typeStr = "Success";
            break;
        case InfoBarIconEnum::Warning:
            typeStr = "Warning";
            break;
        case InfoBarIconEnum::Error:
            typeStr = "Error";
            break;
    }
    setProperty("type", typeStr);
    setProperty("infoBar", true);

    qfw::setStyleSheet(this, FluentStyleSheet::InfoBar);
}

void InfoBar::fadeOut() {
    opacityAni_->setDuration(200);
    opacityAni_->setStartValue(1.0);
    opacityAni_->setEndValue(0.0);
    connect(opacityAni_, &QPropertyAnimation::finished, this, &InfoBar::close);
    opacityAni_->start();
}

void InfoBar::adjustText() {
    QWidget* parentWidget = qobject_cast<QWidget*>(parent());
    int w = parentWidget ? (parentWidget->width() - 50) : 900;

    // Adjust title
    int chars = qBound(30, qMin(w / 10, 120), 120);
    QString wrappedTitle = title_;
    if (wrappedTitle.length() > chars) {
        wrappedTitle = wrappedTitle.left(chars) + "...";
    }
    titleLabel_->setText(wrappedTitle);

    // Adjust content
    chars = qBound(30, qMin(w / 9, 120), 120);
    QString wrappedContent = content_;
    if (wrappedContent.length() > chars) {
        wrappedContent = wrappedContent.left(chars) + "...";
    }
    contentLabel_->setText(wrappedContent);
    adjustSize();
}

void InfoBar::addWidget(QWidget* widget, int stretch) {
    widgetLayout_->addSpacing(6);
    Qt::Alignment align = orient_ == Qt::Vertical ? Qt::AlignTop : Qt::AlignVCenter;
    widgetLayout_->addWidget(widget, stretch, Qt::AlignLeft | align);
}

void InfoBar::setCustomBackgroundColor(const QColor& light, const QColor& dark) {
    lightBackgroundColor_ = light;
    darkBackgroundColor_ = dark;
    update();
}

bool InfoBar::eventFilter(QObject* obj, QEvent* event) {
    if (obj == parent()) {
        if (event->type() == QEvent::Resize || event->type() == QEvent::WindowStateChange) {
            adjustText();
        }
    }
    return QFrame::eventFilter(obj, event);
}

void InfoBar::closeEvent(QCloseEvent* event) {
    emit closedSignal();
    deleteLater();
    event->ignore();
}

void InfoBar::showEvent(QShowEvent* event) {
    QFrame::showEvent(event);
    adjustText();

    if (duration_ >= 0) {
        QTimer::singleShot(duration_, this, &InfoBar::fadeOut);
    }

    if (position_ != InfoBarPosition::None) {
        InfoBarManager* manager = InfoBarManager::make(position_);
        manager->add(this);
    }

    if (parent()) {
        parent()->installEventFilter(this);
    }
}

void InfoBar::paintEvent(QPaintEvent* event) {
    QFrame::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    if (lightBackgroundColor_.isValid()) {
        if (isDarkTheme()) {
            painter.setBrush(darkBackgroundColor_);
        } else {
            painter.setBrush(lightBackgroundColor_);
        }
        QRect rect = this->rect().adjusted(1, 1, -1, -1);
        painter.drawRoundedRect(rect, 6, 6);
    }
}

InfoBar* InfoBar::create(InfoBarIconEnum icon, const QString& title, const QString& content,
                         Qt::Orientation orient, bool isClosable, int duration,
                         InfoBarPosition position, QWidget* parent) {
    InfoBar* w = new InfoBar(icon, title, content, orient, isClosable, duration, position, parent);
    w->show();
    return w;
}

InfoBar* InfoBar::info(const QString& title, const QString& content, Qt::Orientation orient,
                       bool isClosable, int duration, InfoBarPosition position, QWidget* parent) {
    return create(InfoBarIconEnum::Information, title, content, orient, isClosable, duration,
                  position, parent);
}

InfoBar* InfoBar::success(const QString& title, const QString& content, Qt::Orientation orient,
                          bool isClosable, int duration, InfoBarPosition position,
                          QWidget* parent) {
    return create(InfoBarIconEnum::Success, title, content, orient, isClosable, duration, position,
                  parent);
}

InfoBar* InfoBar::warning(const QString& title, const QString& content, Qt::Orientation orient,
                          bool isClosable, int duration, InfoBarPosition position,
                          QWidget* parent) {
    return create(InfoBarIconEnum::Warning, title, content, orient, isClosable, duration, position,
                  parent);
}

InfoBar* InfoBar::error(const QString& title, const QString& content, Qt::Orientation orient,
                        bool isClosable, int duration, InfoBarPosition position, QWidget* parent) {
    return create(InfoBarIconEnum::Error, title, content, orient, isClosable, duration, position,
                  parent);
}

// ============================================================================
// InfoBarManager
// ============================================================================

QMap<InfoBarPosition, InfoBarManager*>& InfoBarManager::managers() {
    static QMap<InfoBarPosition, InfoBarManager*> managers;
    if (managers.isEmpty()) {
        managers[InfoBarPosition::Top] = TopInfoBarManager::create();
        managers[InfoBarPosition::TopRight] = TopRightInfoBarManager::create();
        managers[InfoBarPosition::BottomRight] = BottomRightInfoBarManager::create();
        managers[InfoBarPosition::TopLeft] = TopLeftInfoBarManager::create();
        managers[InfoBarPosition::BottomLeft] = BottomLeftInfoBarManager::create();
        managers[InfoBarPosition::Bottom] = BottomInfoBarManager::create();
    }
    return managers;
}

InfoBarManager::InfoBarManager(QObject* parent) : QObject(parent) {}

InfoBarManager* InfoBarManager::make(InfoBarPosition position) {
    if (!managers().contains(position)) {
        return nullptr;
    }
    return managers()[position];
}

void InfoBarManager::add(InfoBar* infoBar) {
    QWidget* p = infoBar->parentWidget();
    if (!p) {
        return;
    }

    if (!infoBars_.contains(p)) {
        p->installEventFilter(this);
        infoBars_[p] = QList<InfoBar*>();
        aniGroups_[p] = new QParallelAnimationGroup(this);
    }

    if (infoBars_[p].contains(infoBar)) {
        return;
    }

    // Add drop animation
    if (!infoBars_[p].isEmpty()) {
        QPropertyAnimation* dropAni = createDropAni(infoBar);
        aniGroups_[p]->addAnimation(dropAni);
        dropAnis_.append(dropAni);
        infoBar->setProperty("dropAni", QVariant::fromValue(dropAni));
    }

    // Add slide animation
    infoBars_[p].append(infoBar);
    createSlideAni(infoBar);

    connect(infoBar, &InfoBar::closedSignal, this, [this, infoBar]() { remove(infoBar); });
}

void InfoBarManager::remove(InfoBar* infoBar) {
    QWidget* p = infoBar->parentWidget();
    if (!p || !infoBars_.contains(p)) {
        return;
    }

    if (!infoBars_[p].contains(infoBar)) {
        return;
    }

    infoBars_[p].removeOne(infoBar);

    // Remove drop animation
    QPropertyAnimation* dropAni = infoBar->property("dropAni").value<QPropertyAnimation*>();
    if (dropAni) {
        aniGroups_[p]->removeAnimation(dropAni);
        dropAnis_.removeOne(dropAni);
    }

    // Remove slide animation
    QPropertyAnimation* slideAni = infoBar->property("slideAni").value<QPropertyAnimation*>();
    if (slideAni) {
        slideAnis_.removeOne(slideAni);
    }

    // Update remaining info bars positions
    updateDropAni(p);
    aniGroups_[p]->start();
}

void InfoBarManager::createSlideAni(InfoBar* infoBar) {
    QPropertyAnimation* slideAni = new QPropertyAnimation(infoBar, "pos");
    slideAni->setEasingCurve(QEasingCurve::OutQuad);
    slideAni->setDuration(200);
    slideAni->setStartValue(slideStartPos(infoBar));
    slideAni->setEndValue(pos(infoBar));

    slideAnis_.append(slideAni);
    infoBar->setProperty("slideAni", QVariant::fromValue(slideAni));
    slideAni->start();
}

void InfoBarManager::updateDropAni(QWidget* parent) {
    if (!infoBars_.contains(parent)) {
        return;
    }

    for (InfoBar* bar : infoBars_[parent]) {
        QPropertyAnimation* ani = bar->property("dropAni").value<QPropertyAnimation*>();
        if (!ani) {
            continue;
        }
        ani->setStartValue(bar->pos());
        ani->setEndValue(pos(bar));
    }
}

QPropertyAnimation* InfoBarManager::createDropAni(InfoBar* infoBar) {
    QPropertyAnimation* dropAni = new QPropertyAnimation(infoBar, "pos");
    dropAni->setDuration(200);
    return dropAni;
}

bool InfoBarManager::eventFilter(QObject* obj, QEvent* event) {
    QWidget* widget = qobject_cast<QWidget*>(obj);
    if (!widget || !infoBars_.contains(widget)) {
        return QObject::eventFilter(obj, event);
    }

    if (event->type() == QEvent::Resize || event->type() == QEvent::WindowStateChange) {
        QSize size;
        if (event->type() == QEvent::Resize) {
            QResizeEvent* resizeEvent = static_cast<QResizeEvent*>(event);
            size = resizeEvent->size();
        }
        for (InfoBar* bar : infoBars_[widget]) {
            bar->move(pos(bar, size));
        }
    }

    return QObject::eventFilter(obj, event);
}

// ============================================================================
// TopInfoBarManager
// ============================================================================

QPoint TopInfoBarManager::pos(InfoBar* infoBar, const QSize& parentSize) const {
    QWidget* p = infoBar->parentWidget();
    QSize size = parentSize.isValid() ? parentSize : p->size();

    int x = (p->width() - infoBar->width()) / 2;
    int y = margin_;

    if (infoBars_.contains(p)) {
        int index = infoBars_[p].indexOf(infoBar);
        for (int i = 0; i < index; ++i) {
            y += infoBars_[p][i]->height() + spacing_;
        }
    }

    return QPoint(x, y);
}

QPoint TopInfoBarManager::slideStartPos(InfoBar* infoBar) const {
    QPoint p = pos(infoBar);
    return QPoint(p.x(), p.y() - 16);
}

// ============================================================================
// TopRightInfoBarManager
// ============================================================================

QPoint TopRightInfoBarManager::pos(InfoBar* infoBar, const QSize& parentSize) const {
    QWidget* p = infoBar->parentWidget();
    QSize size = parentSize.isValid() ? parentSize : p->size();

    int x = size.width() - infoBar->width() - margin_;
    int y = margin_;

    if (infoBars_.contains(p)) {
        int index = infoBars_[p].indexOf(infoBar);
        for (int i = 0; i < index; ++i) {
            y += infoBars_[p][i]->height() + spacing_;
        }
    }

    return QPoint(x, y);
}

QPoint TopRightInfoBarManager::slideStartPos(InfoBar* infoBar) const {
    QWidget* p = infoBar->parentWidget();
    return QPoint(p->width(), pos(infoBar).y());
}

// ============================================================================
// BottomRightInfoBarManager
// ============================================================================

QPoint BottomRightInfoBarManager::pos(InfoBar* infoBar, const QSize& parentSize) const {
    QWidget* p = infoBar->parentWidget();
    QSize size = parentSize.isValid() ? parentSize : p->size();

    int x = size.width() - infoBar->width() - margin_;
    int y = size.height() - infoBar->height() - margin_;

    if (infoBars_.contains(p)) {
        int index = infoBars_[p].indexOf(infoBar);
        for (int i = 0; i < index; ++i) {
            y -= infoBars_[p][i]->height() + spacing_;
        }
    }

    return QPoint(x, y);
}

QPoint BottomRightInfoBarManager::slideStartPos(InfoBar* infoBar) const {
    QWidget* p = infoBar->parentWidget();
    return QPoint(p->width(), pos(infoBar).y());
}

// ============================================================================
// TopLeftInfoBarManager
// ============================================================================

QPoint TopLeftInfoBarManager::pos(InfoBar* infoBar, const QSize& parentSize) const {
    QWidget* p = infoBar->parentWidget();
    Q_UNUSED(parentSize);

    int y = margin_;

    if (infoBars_.contains(p)) {
        int index = infoBars_[p].indexOf(infoBar);
        for (int i = 0; i < index; ++i) {
            y += infoBars_[p][i]->height() + spacing_;
        }
    }

    return QPoint(margin_, y);
}

QPoint TopLeftInfoBarManager::slideStartPos(InfoBar* infoBar) const {
    return QPoint(-infoBar->width(), pos(infoBar).y());
}

// ============================================================================
// BottomLeftInfoBarManager
// ============================================================================

QPoint BottomLeftInfoBarManager::pos(InfoBar* infoBar, const QSize& parentSize) const {
    QWidget* p = infoBar->parentWidget();
    QSize size = parentSize.isValid() ? parentSize : p->size();

    int y = size.height() - infoBar->height() - margin_;

    if (infoBars_.contains(p)) {
        int index = infoBars_[p].indexOf(infoBar);
        for (int i = 0; i < index; ++i) {
            y -= infoBars_[p][i]->height() + spacing_;
        }
    }

    return QPoint(margin_, y);
}

QPoint BottomLeftInfoBarManager::slideStartPos(InfoBar* infoBar) const {
    return QPoint(-infoBar->width(), pos(infoBar).y());
}

// ============================================================================
// BottomInfoBarManager
// ============================================================================

QPoint BottomInfoBarManager::pos(InfoBar* infoBar, const QSize& parentSize) const {
    QWidget* p = infoBar->parentWidget();
    QSize size = parentSize.isValid() ? parentSize : p->size();

    int x = (size.width() - infoBar->width()) / 2;
    int y = size.height() - infoBar->height() - margin_;

    if (infoBars_.contains(p)) {
        int index = infoBars_[p].indexOf(infoBar);
        for (int i = 0; i < index; ++i) {
            y -= infoBars_[p][i]->height() + spacing_;
        }
    }

    return QPoint(x, y);
}

QPoint BottomInfoBarManager::slideStartPos(InfoBar* infoBar) const {
    QPoint p = pos(infoBar);
    return QPoint(p.x(), p.y() + 16);
}

// ============================================================================
// DesktopInfoBarView
// ============================================================================

DesktopInfoBarView::DesktopInfoBarView(QWidget* parent) : QWidget(parent) {
#ifdef Q_OS_WIN
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::SubWindow);
#else
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool |
                   Qt::WindowTransparentForInput);
#endif

    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_TranslucentBackground);
    setGeometry(QApplication::primaryScreen()->availableGeometry());
}

DesktopInfoBarView* DesktopInfoBarView::instance() {
    static DesktopInfoBarView* view = new DesktopInfoBarView();
    return view;
}

}  // namespace qfw
