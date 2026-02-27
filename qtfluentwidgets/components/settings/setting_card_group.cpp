#include "components/settings/setting_card_group.h"

#include "common/config.h"
#include "common/font.h"
#include "common/style_sheet.h"
#include "components/layout/expand_layout.h"

namespace qfw {

SettingCardGroup::SettingCardGroup(const QString& title, QWidget* parent)
    : QWidget(parent),
      titleLabel_(new QLabel(title, this)),
      vBoxLayout_(new QVBoxLayout(this)),
      cardLayout_(new ExpandLayout()) {
    setProperty("qssClass", "SettingCardGroup");

    vBoxLayout_->setContentsMargins(0, 0, 0, 0);
    vBoxLayout_->setAlignment(Qt::AlignTop);
    vBoxLayout_->setSpacing(0);

    cardLayout_->setContentsMargins(0, 0, 0, 0);
    cardLayout_->setSpacing(2);

    vBoxLayout_->addWidget(titleLabel_);
    vBoxLayout_->addSpacing(12);
    vBoxLayout_->addLayout(cardLayout_, 1);

    FluentStyleSheetSource source(FluentStyleSheet::SettingCardGroup);
    qfw::setStyleSheet(this, source, Theme::Auto, true);
    qfw::setFont(titleLabel_, 20, QFont::Normal);
    titleLabel_->adjustSize();
}

void SettingCardGroup::addSettingCard(QWidget* card) {
    card->setParent(this);
    cardLayout_->addWidget(card);
    adjustSize();
}

void SettingCardGroup::addSettingCards(const QList<QWidget*>& cards) {
    for (auto* card : cards) {
        addSettingCard(card);
    }
}

void SettingCardGroup::adjustSize() {
    const int h = cardLayout_->heightForWidth(width()) + 46;
    resize(width(), h);
}

}  // namespace qfw
