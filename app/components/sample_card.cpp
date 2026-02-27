#include "sample_card.h"

#include <QMouseEvent>

#include "common/auto_wrap.h"
#include "common/gallery_style_sheet.h"
#include "common/signal_bus.h"

namespace qfw {

SampleCard::SampleCard(const FluentIconBase& icon, const QString& title, const QString& content,
                       const QString& routeKey, int index, QWidget* parent)
    : CardWidget(parent), routeKey_(routeKey), index_(index) {
    setFixedSize(360, 90);
    setProperty("qssClass", "SampleCard");
    setAttribute(Qt::WA_StyledBackground);

    iconWidget_ = new IconWidget(icon, this);
    titleLabel_ = new QLabel(title, this);
    contentLabel_ = new QLabel(TextWrap::wrap(content, 45, false).first, this);

    iconWidget_->setFixedSize(48, 48);

    hBoxLayout_ = new QHBoxLayout(this);
    vBoxLayout_ = new QVBoxLayout();

    hBoxLayout_->setSpacing(28);
    hBoxLayout_->setContentsMargins(20, 0, 0, 0);
    vBoxLayout_->setSpacing(2);
    vBoxLayout_->setContentsMargins(0, 0, 0, 0);
    vBoxLayout_->setAlignment(Qt::AlignVCenter);

    hBoxLayout_->setAlignment(Qt::AlignVCenter);
    hBoxLayout_->addWidget(iconWidget_);
    hBoxLayout_->addLayout(vBoxLayout_);
    vBoxLayout_->addStretch(1);
    vBoxLayout_->addWidget(titleLabel_);
    vBoxLayout_->addWidget(contentLabel_);
    vBoxLayout_->addStretch(1);

    titleLabel_->setObjectName(QStringLiteral("titleLabel"));
    contentLabel_->setObjectName(QStringLiteral("contentLabel"));

    titleLabel_->setWordWrap(true);
    contentLabel_->setWordWrap(true);
}

SampleCard::SampleCard(FluentIconEnum icon, const QString& title, const QString& content,
                       const QString& routeKey, int index, QWidget* parent)
    : SampleCard(FluentIcon(icon), title, content, routeKey, index, parent) {}

SampleCard::SampleCard(const QString& icon, const QString& title, const QString& content,
                       const QString& routeKey, int index, QWidget* parent)
    : CardWidget(parent), routeKey_(routeKey), index_(index) {
    setFixedSize(360, 90);
    setProperty("qssClass", "SampleCard");
    setAttribute(Qt::WA_StyledBackground);

    iconWidget_ = new IconWidget(icon, this);
    titleLabel_ = new QLabel(title, this);
    contentLabel_ = new QLabel(TextWrap::wrap(content, 45, false).first, this);

    iconWidget_->setFixedSize(48, 48);

    hBoxLayout_ = new QHBoxLayout(this);
    vBoxLayout_ = new QVBoxLayout();

    hBoxLayout_->setSpacing(28);
    hBoxLayout_->setContentsMargins(20, 0, 0, 0);
    vBoxLayout_->setSpacing(2);
    vBoxLayout_->setContentsMargins(0, 0, 0, 0);
    vBoxLayout_->setAlignment(Qt::AlignVCenter);

    hBoxLayout_->setAlignment(Qt::AlignVCenter);
    hBoxLayout_->addWidget(iconWidget_);
    hBoxLayout_->addLayout(vBoxLayout_);
    vBoxLayout_->addStretch(1);
    vBoxLayout_->addWidget(titleLabel_);
    vBoxLayout_->addWidget(contentLabel_);
    vBoxLayout_->addStretch(1);

    titleLabel_->setObjectName(QStringLiteral("titleLabel"));
    contentLabel_->setObjectName(QStringLiteral("contentLabel"));

    titleLabel_->setWordWrap(true);
    contentLabel_->setWordWrap(true);
}

void SampleCard::mouseReleaseEvent(QMouseEvent* event) {
    CardWidget::mouseReleaseEvent(event);
    emit signalBus().switchToSampleCard(routeKey_, index_);
}

// SampleCardView implementation
SampleCardView::SampleCardView(const QString& title, QWidget* parent) : QWidget(parent) {
    titleLabel_ = new QLabel(title, this);
    vBoxLayout_ = new QVBoxLayout(this);
    flowLayout_ = new FlowLayout();

    setProperty("qssClass", "SampleCardView");

    vBoxLayout_->setContentsMargins(36, 0, 36, 0);
    vBoxLayout_->setSpacing(10);
    flowLayout_->setContentsMargins(0, 0, 0, 0);
    flowLayout_->setHorizontalSpacing(12);
    flowLayout_->setVerticalSpacing(12);

    vBoxLayout_->addWidget(titleLabel_);
    vBoxLayout_->addLayout(flowLayout_, 1);

    titleLabel_->setObjectName(QStringLiteral("viewTitleLabel"));
    applyGalleryStyleSheet(this, GalleryStyleSheet::SampleCard);
}

void SampleCardView::addSampleCard(const FluentIconBase& icon, const QString& title,
                                   const QString& content, const QString& routeKey, int index) {
    auto* card = new SampleCard(icon, title, content, routeKey, index, this);
    flowLayout_->addWidget(card);
}

void SampleCardView::addSampleCard(FluentIconEnum icon, const QString& title,
                                   const QString& content, const QString& routeKey, int index) {
    addSampleCard(FluentIcon(icon), title, content, routeKey, index);
}

void SampleCardView::addSampleCard(const QString& icon, const QString& title,
                                   const QString& content, const QString& routeKey, int index) {
    auto* card = new SampleCard(icon, title, content, routeKey, index, this);
    flowLayout_->addWidget(card);
}

}  // namespace qfw
