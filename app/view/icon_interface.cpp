#include "view/icon_interface.h"

#include <QApplication>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QVBoxLayout>

#include "common/config.h"
#include "common/gallery_style_sheet.h"
#include "common/translator.h"
#include "common/trie.h"
#include "components/layout/flow_layout.h"
#include "components/widgets/icon_widget.h"
#include "components/widgets/label.h"
#include "components/widgets/line_edit.h"

namespace qfw {

static QString iconValue(FluentIconEnum icon) { return FluentIcon::enumToString(icon); }

void IconCard::setSelected(bool isSelected) { setSelected(isSelected, false); }

IconCard::IconCard(FluentIconEnum icon, QWidget* parent) : QFrame(parent), icon_(icon) {
    setFixedSize(96, 96);
    setProperty("qssClass", "IconCard");
    setAttribute(Qt::WA_StyledBackground);

    iconWidget_ = new IconWidget(FluentIcon(icon_), this);
    nameLabel_ = new QLabel(this);
    vBoxLayout_ = new QVBoxLayout(this);

    vBoxLayout_->setSpacing(0);
    vBoxLayout_->setContentsMargins(8, 28, 8, 0);
    vBoxLayout_->setAlignment(Qt::AlignTop);

    iconWidget_->setFixedSize(28, 28);
    vBoxLayout_->addWidget(iconWidget_, 0, Qt::AlignHCenter);
    vBoxLayout_->addSpacing(14);
    vBoxLayout_->addWidget(nameLabel_, 0, Qt::AlignHCenter);

    const QString text = nameLabel_->fontMetrics().elidedText(iconValue(icon_), Qt::ElideRight, 90);
    nameLabel_->setText(text);
}

void IconCard::mouseReleaseEvent(QMouseEvent* e) {
    QFrame::mouseReleaseEvent(e);

    if (isSelected_) {
        return;
    }

    emit clicked(icon_);
}

void IconCard::setSelected(bool isSelected, bool force) {
    if (isSelected == isSelected_ && !force) {
        return;
    }

    isSelected_ = isSelected;

    if (!isSelected_) {
        iconWidget_->setIcon(FluentIcon(icon_));
    } else {
        const Theme t = isDarkTheme() ? Theme::Light : Theme::Dark;
        iconWidget_->setIcon(FluentIcon(icon_).icon(t));
    }

    setProperty("isSelected", isSelected_);
    setStyle(QApplication::style());
}

IconInfoPanel::IconInfoPanel(FluentIconEnum icon, QWidget* parent) : QFrame(parent) {
    setProperty("qssClass", "IconInfoPanel");
    setAttribute(Qt::WA_StyledBackground);

    nameLabel_ = new QLabel(iconValue(icon), this);
    iconWidget_ = new IconWidget(FluentIcon(icon), this);
    iconNameTitleLabel_ = new QLabel(tr("Icon name"), this);
    iconNameLabel_ = new QLabel(iconValue(icon), this);
    enumNameTitleLabel_ = new QLabel(tr("Enum member"), this);
    enumNameLabel_ = new QLabel(QStringLiteral("FluentIconEnum::") + iconValue(icon), this);

    vBoxLayout_ = new QVBoxLayout(this);
    vBoxLayout_->setContentsMargins(16, 20, 16, 20);
    vBoxLayout_->setSpacing(0);
    vBoxLayout_->setAlignment(Qt::AlignTop);

    vBoxLayout_->addWidget(nameLabel_);
    vBoxLayout_->addSpacing(16);
    vBoxLayout_->addWidget(iconWidget_);
    vBoxLayout_->addSpacing(45);
    vBoxLayout_->addWidget(iconNameTitleLabel_);
    vBoxLayout_->addSpacing(5);
    vBoxLayout_->addWidget(iconNameLabel_);
    vBoxLayout_->addSpacing(34);
    vBoxLayout_->addWidget(enumNameTitleLabel_);
    vBoxLayout_->addSpacing(5);
    vBoxLayout_->addWidget(enumNameLabel_);

    iconWidget_->setFixedSize(48, 48);
    setFixedWidth(216);

    nameLabel_->setObjectName(QStringLiteral("nameLabel"));
    iconNameTitleLabel_->setObjectName(QStringLiteral("subTitleLabel"));
    enumNameTitleLabel_->setObjectName(QStringLiteral("subTitleLabel"));
}

void IconInfoPanel::setIcon(qfw::FluentIconEnum icon) {
    iconWidget_->setIcon(FluentIcon(icon));
    nameLabel_->setText(iconValue(icon));
    iconNameLabel_->setText(iconValue(icon));
    enumNameLabel_->setText(QStringLiteral("FluentIconEnum::") + iconValue(icon));
}

IconCardView::IconCardView(QWidget* parent) : QWidget(parent) {
    setProperty("qssClass", "IconCardView");

    trie_ = new Trie();

    iconLibraryLabel_ = new StrongBodyLabel(tr("Fluent Icons Library"), this);
    searchLineEdit_ = new SearchLineEdit(this);

    view_ = new QFrame(this);
    scrollArea_ = new IconSmoothScrollArea(view_);
    scrollWidget_ = new QWidget(scrollArea_);
    infoPanel_ = new IconInfoPanel(FluentIconEnum::Menu, this);

    vBoxLayout_ = new QVBoxLayout(this);
    hBoxLayout_ = new QHBoxLayout(view_);
    flowLayout_ = new FlowLayout(scrollWidget_, true, true);

    initWidget();
}

void IconCardView::initWidget() {
    searchLineEdit_->setPlaceholderText(tr("Search icons"));
    searchLineEdit_->setFixedWidth(304);

    scrollArea_->setWidget(scrollWidget_);
    scrollArea_->setViewportMarginsPublic(0, 5, 0, 5);
    scrollArea_->setWidgetResizable(true);
    scrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    vBoxLayout_->setContentsMargins(0, 0, 0, 0);
    vBoxLayout_->setSpacing(12);
    vBoxLayout_->addWidget(iconLibraryLabel_);
    vBoxLayout_->addWidget(searchLineEdit_);
    vBoxLayout_->addWidget(view_);

    hBoxLayout_->setSpacing(0);
    hBoxLayout_->setContentsMargins(0, 0, 0, 0);
    hBoxLayout_->addWidget(scrollArea_);
    hBoxLayout_->addWidget(infoPanel_, 0, Qt::AlignRight);

    flowLayout_->setVerticalSpacing(8);
    flowLayout_->setHorizontalSpacing(8);
    flowLayout_->setContentsMargins(8, 3, 8, 8);

    connect(searchLineEdit_, &SearchLineEdit::searchSignal, this,
            &IconCardView::onSearchTextChanged);

    applyQss();
    connect(&QConfig::instance(), &QConfig::themeChanged, this, [this](Theme) { applyQss(); });

    const int last = static_cast<int>(FluentIconEnum::Pin);
    for (int i = 0; i <= last; ++i) {
        addIcon(static_cast<FluentIconEnum>(i));
    }

    if (!icons_.isEmpty()) {
        setSelectedIcon(icons_.first());
    }
}

void IconCardView::addIcon(qfw::FluentIconEnum icon) {
    auto* card = new IconCard(icon, this);
    connect(card, &IconCard::clicked, this, &IconCardView::setSelectedIcon);

    trie_->insert(iconValue(icon), cards_.size());
    cards_.append(card);
    icons_.append(icon);
    flowLayout_->addWidget(card);
}

void IconCardView::setSelectedIcon(qfw::FluentIconEnum icon) {
    const int index = icons_.indexOf(icon);
    if (index < 0) {
        return;
    }

    if (currentIndex_ >= 0 && currentIndex_ < cards_.size()) {
        cards_[currentIndex_]->setSelected(false, false);
    }

    currentIndex_ = index;
    cards_[index]->setSelected(true, false);
    infoPanel_->setIcon(icon);
}

void IconCardView::applyQss() {
    view_->setObjectName(QStringLiteral("iconView"));
    scrollWidget_->setObjectName(QStringLiteral("scrollWidget"));

    applyGalleryStyleSheet(this, GalleryStyleSheet::IconInterface);
    applyGalleryStyleSheet(scrollWidget_, GalleryStyleSheet::IconInterface);

    if (currentIndex_ >= 0 && currentIndex_ < cards_.size()) {
        cards_[currentIndex_]->setSelected(true, true);
    }
}

void IconCardView::onSearchTextChanged(const QString& text) {
    const QString keyWord = text.trimmed().toLower();
    if (keyWord.isEmpty()) {
        showAllIcons();
        return;
    }

    const auto items = trie_->items(keyWord);
    QSet<int> indexes;
    indexes.reserve(items.size());
    for (const auto& it : items) {
        indexes.insert(it.second.toInt());
    }

    flowLayout_->removeAllWidgets();
    for (int i = 0; i < cards_.size(); ++i) {
        const bool visible = indexes.contains(i);
        cards_[i]->setVisible(visible);
        if (visible) {
            flowLayout_->addWidget(cards_[i]);
        }
    }
}

void IconCardView::showAllIcons() {
    flowLayout_->removeAllWidgets();
    for (auto* card : cards_) {
        card->show();
        flowLayout_->addWidget(card);
    }
}

IconInterface::IconInterface(QWidget* parent)
    : GalleryInterface(Translator().icons(), QStringLiteral("qtfluentwidgets.common.icon"), parent) {
    setObjectName(QStringLiteral("iconInterface"));
    setProperty("qssClass", "IconInterface");

    iconView_ = new IconCardView(this);
    if (auto* layout = contentLayout()) {
        layout->addWidget(iconView_);
    }
}

}  // namespace qfw
