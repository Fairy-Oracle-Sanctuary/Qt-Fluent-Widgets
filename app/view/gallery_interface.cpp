#include "view/gallery_interface.h"

#include <QDesktopServices>
#include <QEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QPainter>
#include <QPen>
#include <QResizeEvent>
#include <QScrollBar>
#include <QUrl>
#include <QVBoxLayout>

#include "common/app_config.h"
#include "common/gallery_style_sheet.h"
#include "common/signal_bus.h"
#include "common/style_sheet.h"
#include "components/widgets/button.h"
#include "components/widgets/icon_widget.h"
#include "components/widgets/label.h"
#include "components/widgets/tool_tip.h"

namespace qfw {

GallerySeparatorWidget::GallerySeparatorWidget(QWidget* parent) : QWidget(parent) {
    setFixedSize(6, 16);
    setProperty("qssClass", "GallerySeparatorWidget");
}

void GallerySeparatorWidget::paintEvent(QPaintEvent* e) {
    Q_UNUSED(e);

    QPainter painter(this);
    QPen pen(1);
    pen.setCosmetic(true);
    QColor c = isDarkTheme() ? QColor(255, 255, 255, 21) : QColor(0, 0, 0, 15);
    pen.setColor(c);
    painter.setPen(pen);

    int x = width() / 2;
    painter.drawLine(x, 0, x, height());
}

ToolBar::ToolBar(const QString& title, const QString& subtitle, QWidget* parent) : QWidget(parent) {
    setProperty("qssClass", "ToolBar");

    titleLabel_ = new TitleLabel(title, this);
    subtitleLabel_ = new CaptionLabel(subtitle, this);

    documentButton_ = new PushButton(tr("Documentation"), this);
    documentButton_->setIcon(FluentIcon(FluentIconEnum::Document));

    sourceButton_ = new PushButton(tr("Source"), this);
    sourceButton_->setIcon(FluentIcon(FluentIconEnum::Github));

    themeButton_ = new ToolButton(FluentIcon(FluentIconEnum::Constract), this);
    separator_ = new GallerySeparatorWidget(this);
    supportButton_ = new ToolButton(FluentIcon(FluentIconEnum::Heart), this);
    feedbackButton_ = new ToolButton(FluentIcon(FluentIconEnum::Feedback), this);

    vBoxLayout_ = new QVBoxLayout(this);
    buttonLayout_ = new QHBoxLayout();

    initWidget();
}

void ToolBar::initWidget() {
    setFixedHeight(138);

    vBoxLayout_->setSpacing(0);
    vBoxLayout_->setContentsMargins(36, 22, 36, 12);
    vBoxLayout_->addWidget(titleLabel_);
    vBoxLayout_->addSpacing(4);
    vBoxLayout_->addWidget(subtitleLabel_);
    vBoxLayout_->addSpacing(4);
    vBoxLayout_->addLayout(buttonLayout_, 1);
    vBoxLayout_->setAlignment(Qt::AlignTop);

    buttonLayout_->setSpacing(4);
    buttonLayout_->setContentsMargins(0, 0, 0, 0);
    buttonLayout_->addWidget(documentButton_, 0, Qt::AlignLeft);
    buttonLayout_->addWidget(sourceButton_, 0, Qt::AlignLeft);
    buttonLayout_->addStretch(1);
    buttonLayout_->addWidget(themeButton_, 0, Qt::AlignRight);
    buttonLayout_->addWidget(separator_, 0, Qt::AlignRight);
    buttonLayout_->addWidget(supportButton_, 0, Qt::AlignRight);
    buttonLayout_->addWidget(feedbackButton_, 0, Qt::AlignRight);
    buttonLayout_->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

    themeButton_->installEventFilter(new ToolTipFilter(themeButton_));
    supportButton_->installEventFilter(new ToolTipFilter(supportButton_));
    feedbackButton_->installEventFilter(new ToolTipFilter(feedbackButton_));

    themeButton_->setToolTip(tr("Toggle theme"));
    supportButton_->setToolTip(tr("Support me"));
    feedbackButton_->setToolTip(tr("Send feedback"));

    connect(themeButton_, &QToolButton::clicked, this, []() { toggleTheme(true, false); });
    connect(supportButton_, &QToolButton::clicked, this,
            []() { emit signalBus().supportSignal(); });

    connect(documentButton_, &QPushButton::clicked, this,
            []() { QDesktopServices::openUrl(QUrl(QString::fromUtf8(AppConfig::HELP_URL))); });
    connect(sourceButton_, &QPushButton::clicked, this,
            []() { QDesktopServices::openUrl(QUrl(QString::fromUtf8(AppConfig::EXAMPLE_URL))); });
    connect(feedbackButton_, &QToolButton::clicked, this,
            []() { QDesktopServices::openUrl(QUrl(QString::fromUtf8(AppConfig::FEEDBACK_URL))); });

    subtitleLabel_->setTextColor(QColor(96, 96, 96), QColor(216, 216, 216));
}

ExampleCard::ExampleCard(const QString& title, QWidget* widget, const QString& sourcePath,
                         int stretch, QWidget* parent)
    : QWidget(parent), widget_(widget), stretch_(stretch), sourcePath_(sourcePath) {
    setProperty("qssClass", "ExampleCard");

    titleLabel_ = new StrongBodyLabel(title, this);
    card_ = new QFrame(this);

    sourceWidget_ = new QFrame(card_);
    sourcePathLabel_ = new BodyLabel(tr("Source code"), sourceWidget_);
    linkIcon_ = new IconWidget(FluentIcon(FluentIconEnum::Link), sourceWidget_);

    vBoxLayout_ = new QVBoxLayout(this);
    cardLayout_ = new QVBoxLayout(card_);
    topLayout_ = new QHBoxLayout();
    bottomLayout_ = new QHBoxLayout(sourceWidget_);

    initWidget();
}

void ExampleCard::initWidget() {
    linkIcon_->setFixedSize(16, 16);
    initLayout();

    sourceWidget_->setCursor(Qt::PointingHandCursor);
    sourceWidget_->installEventFilter(this);

    card_->setObjectName(QStringLiteral("card"));
    sourceWidget_->setObjectName(QStringLiteral("sourceWidget"));
}

void ExampleCard::initLayout() {
    vBoxLayout_->setSizeConstraint(QVBoxLayout::SetMinimumSize);
    cardLayout_->setSizeConstraint(QVBoxLayout::SetMinimumSize);
    topLayout_->setSizeConstraint(QHBoxLayout::SetMinimumSize);

    vBoxLayout_->setSpacing(12);
    vBoxLayout_->setContentsMargins(0, 0, 0, 0);
    topLayout_->setContentsMargins(12, 12, 12, 12);
    bottomLayout_->setContentsMargins(18, 18, 18, 18);
    cardLayout_->setContentsMargins(0, 0, 0, 0);

    vBoxLayout_->addWidget(titleLabel_, 0, Qt::AlignTop);
    vBoxLayout_->addWidget(card_, 0, Qt::AlignTop);
    vBoxLayout_->setAlignment(Qt::AlignTop);

    cardLayout_->setSpacing(0);
    cardLayout_->setAlignment(Qt::AlignTop);
    cardLayout_->addLayout(topLayout_, 0);
    cardLayout_->addWidget(sourceWidget_, 0, Qt::AlignBottom);

    if (widget_) {
        widget_->setParent(card_);
        topLayout_->addWidget(widget_);
        if (stretch_ == 0) {
            topLayout_->addStretch(1);
        }
        widget_->show();
    }

    bottomLayout_->addWidget(sourcePathLabel_, 0, Qt::AlignLeft);
    bottomLayout_->addStretch(1);
    bottomLayout_->addWidget(linkIcon_, 0, Qt::AlignRight);
    bottomLayout_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
}

bool ExampleCard::eventFilter(QObject* obj, QEvent* e) {
    if (obj == sourceWidget_ && e->type() == QEvent::MouseButtonRelease) {
        QDesktopServices::openUrl(QUrl(sourcePath_));
    }

    return QWidget::eventFilter(obj, e);
}

GalleryInterface::GalleryInterface(const QString& title, const QString& subtitle, QWidget* parent)
    : ScrollArea(parent) {
    view_ = new QWidget(this);
    toolBar_ = new ToolBar(title, subtitle, this);
    vBoxLayout_ = new QVBoxLayout(view_);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setViewportMargins(0, toolBar_->height(), 0, 0);
    setWidget(view_);
    setWidgetResizable(true);

    vBoxLayout_->setSpacing(30);
    vBoxLayout_->setAlignment(Qt::AlignTop);
    vBoxLayout_->setContentsMargins(36, 20, 36, 36);

    view_->setObjectName(QStringLiteral("view"));
    setProperty("qssClass", "GalleryInterface");
    applyGalleryStyleSheet(this, GalleryStyleSheet::GalleryInterface);
}

ExampleCard* GalleryInterface::addExampleCard(const QString& title, QWidget* widget,
                                              const QString& sourcePath, int stretch) {
    auto* card = new ExampleCard(title, widget, sourcePath, stretch, view_);
    vBoxLayout_->addWidget(card, 0, Qt::AlignTop);
    return card;
}

QVBoxLayout* GalleryInterface::contentLayout() const { return vBoxLayout_; }

void GalleryInterface::scrollToCard(int index) {
    if (!vBoxLayout_) {
        return;
    }

    auto* item = vBoxLayout_->itemAt(index);
    if (!item) {
        return;
    }

    auto* w = item->widget();
    if (!w) {
        return;
    }

    verticalScrollBar()->setValue(w->y());
}

void GalleryInterface::resizeEvent(QResizeEvent* e) {
    ScrollArea::resizeEvent(e);

    if (toolBar_) {
        toolBar_->resize(width(), toolBar_->height());
    }
}

}  // namespace qfw
