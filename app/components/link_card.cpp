#include "link_card.h"

#include <QDesktopServices>
#include <QMouseEvent>

#include "common/auto_wrap.h"
#include "common/gallery_style_sheet.h"
#include "components/widgets/icon_widget.h"

namespace qfw {

LinkCard::LinkCard(const FluentIconBase& icon, const QString& title, const QString& content,
                   const QString& url, QWidget* parent)
    : QFrame(parent), url_(url) {
    setFixedSize(198, 220);
    setProperty("qssClass", "LinkCard");
    setAttribute(Qt::WA_StyledBackground);

    iconWidget_ = new IconWidget(icon, this);
    titleLabel_ = new QLabel(title, this);
    contentLabel_ = new QLabel(TextWrap::wrap(content, 28, false).first, this);
    urlWidget_ = new IconWidget(FluentIcon(FluentIconEnum::Link), this);

    initWidget();
}

LinkCard::LinkCard(FluentIconEnum icon, const QString& title, const QString& content,
                   const QString& url, QWidget* parent)
    : LinkCard(FluentIcon(icon), title, content, url, parent) {}

LinkCard::LinkCard(const QString& icon, const QString& title, const QString& content,
                   const QString& url, QWidget* parent)
    : QFrame(parent), url_(url) {
    setFixedSize(198, 220);
    setProperty("qssClass", "LinkCard");
    setAttribute(Qt::WA_StyledBackground);

    iconWidget_ = new IconWidget(icon, this);
    titleLabel_ = new QLabel(title, this);
    contentLabel_ = new QLabel(TextWrap::wrap(content, 28, false).first, this);
    urlWidget_ = new IconWidget(FluentIcon(FluentIconEnum::Link), this);

    initWidget();
}

void LinkCard::initWidget() {
    setCursor(Qt::PointingHandCursor);

    iconWidget_->setFixedSize(54, 54);
    urlWidget_->setFixedSize(16, 16);

    vBoxLayout_ = new QVBoxLayout(this);
    vBoxLayout_->setSpacing(0);
    vBoxLayout_->setContentsMargins(24, 24, 0, 13);
    vBoxLayout_->addWidget(iconWidget_);
    vBoxLayout_->addSpacing(16);
    vBoxLayout_->addWidget(titleLabel_);
    vBoxLayout_->addSpacing(8);
    vBoxLayout_->addWidget(contentLabel_);
    vBoxLayout_->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    urlWidget_->move(170, 192);

    titleLabel_->setObjectName(QStringLiteral("titleLabel"));
    contentLabel_->setObjectName(QStringLiteral("contentLabel"));
    contentLabel_->setWordWrap(true);
}

void LinkCard::mouseReleaseEvent(QMouseEvent* event) {
    QFrame::mouseReleaseEvent(event);
    QDesktopServices::openUrl(url_);
}

// LinkCardView implementation
LinkCardView::LinkCardView(QWidget* parent) : SingleDirectionScrollArea(parent, Qt::Horizontal) {
    setProperty("qssClass", "LinkCardView");
    applyGalleryStyleSheet(this, GalleryStyleSheet::LinkCard);

    view_ = new QWidget(this);
    hBoxLayout_ = new QHBoxLayout(view_);

    hBoxLayout_->setContentsMargins(36, 0, 0, 0);
    hBoxLayout_->setSpacing(12);
    hBoxLayout_->setAlignment(Qt::AlignLeft);

    setWidget(view_);
    setWidgetResizable(true);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    view_->setObjectName(QStringLiteral("view"));
}

void LinkCardView::addCard(const FluentIconBase& icon, const QString& title, const QString& content,
                           const QString& url) {
    auto* card = new LinkCard(icon, title, content, url, view_);
    hBoxLayout_->addWidget(card, 0, Qt::AlignLeft);
}

void LinkCardView::addCard(FluentIconEnum icon, const QString& title, const QString& content,
                           const QString& url) {
    addCard(FluentIcon(icon), title, content, url);
}

void LinkCardView::addCard(const QString& icon, const QString& title, const QString& content,
                           const QString& url) {
    auto* card = new LinkCard(icon, title, content, url, view_);
    hBoxLayout_->addWidget(card, 0, Qt::AlignLeft);
}

}  // namespace qfw
