#include "components/widgets/extended_combo_box.h"

#include <QApplication>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QHideEvent>
#include <QKeyEvent>
#include <QListWidget>
#include <QMouseEvent>
#include <QPainter>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QScreen>
#include <QScrollArea>
#include <QScrollBar>
#include <QSignalBlocker>
#include <QStyleOptionButton>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <algorithm>
#include <stdexcept>

#include "common/style_sheet.h"
#include "components/widgets/chip.h"

namespace qfw {
namespace {
QIcon ownedComboIcon(const QVariant& value) {
    if (value.canConvert<QIcon>()) return value.value<QIcon>();
    if (value.canConvert<const FluentIconBase*>()) {
        const auto* icon = value.value<const FluentIconBase*>();
        return icon ? icon->icon() : QIcon();
    }
    return value.canConvert<QString>() ? QIcon(value.toString()) : QIcon();
}
void drawComboButton(QPushButton* button, const QStyleOptionButton& option, bool placeholder) {
    QPainter p(button);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    button->style()->drawControl(QStyle::CE_PushButtonBevel, &option, &p, button);
    const int shade = isDarkTheme() ? 255 : 0;
    const int alpha = !button->isEnabled() ? 92 : button->isDown() ? 161 : placeholder ? 155 : 255;
    p.setPen(QColor(shade, shade, shade, alpha));
    p.setFont(button->font());
    const auto text = QStyle::visualRect(button->layoutDirection(), button->rect(),
                                         button->rect().adjusted(11, 0, -32, 0));
    p.drawText(
        text,
        Qt::AlignVCenter |
            (button->layoutDirection() == Qt::RightToLeft ? Qt::AlignRight : Qt::AlignLeft),
        QFontMetrics(button->font()).elidedText(button->text(), Qt::ElideRight, text.width()));
    p.setOpacity(!button->isEnabled()   ? 0.36
                 : button->isDown()     ? 0.7
                 : button->underMouse() ? 0.8
                                        : 1);
    const auto arrow =
        QStyle::visualRect(button->layoutDirection(), button->rect(),
                           QRect(button->width() - 22, button->height() / 2 - 5, 10, 10));
    FluentIcon(FluentIconEnum::ArrowDown).render(&p, arrow);
}
class FontPreviewDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override {
        painter->save();
        painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
        const bool enabled = index.flags() & Qt::ItemIsEnabled;
        const bool selected = option.state & QStyle::State_Selected;
        const int shade = isDarkTheme() ? 255 : 0;
        if (selected || ((option.state & QStyle::State_MouseOver) && enabled)) {
            painter->setPen(Qt::NoPen);
            painter->setBrush(QColor(shade, shade, shade, selected ? 9 : 10));
            painter->drawRoundedRect(QRectF(option.rect), 5, 5);
        }
        if (selected) {
            painter->setPen(Qt::NoPen);
            painter->setBrush(themeColor());
            const auto marker =
                QStyle::visualRect(option.direction, option.rect,
                                   QRect(option.rect.x() + 6, option.rect.y() + 9, 3, 15));
            painter->drawRoundedRect(QRectF(marker), 1.5, 1.5);
        }
        QFont preview(index.data().toString().trimmed());
        preview.setPixelSize(14);
        painter->setFont(preview);
        QColor color(shade, shade, shade);
        if (!enabled) color.setAlpha(80);
        painter->setPen(color);
        const QRect text =
            QStyle::visualRect(option.direction, option.rect, option.rect.adjusted(15, 0, -10, 0));
        painter->drawText(text,
                          Qt::AlignVCenter | (option.direction == Qt::RightToLeft ? Qt::AlignRight
                                                                                  : Qt::AlignLeft),
                          QFontMetrics(preview).elidedText(index.data().toString().trimmed(),
                                                           Qt::ElideRight, text.width()));
        painter->restore();
    }
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override {
        QFont preview(index.data().toString().trimmed());
        preview.setPixelSize(14);
        return QSize(
            std::max(100, QFontMetrics(preview).horizontalAdvance(index.data().toString()) + 30),
            33);
    }
};
class MultiComboDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    QSize sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const override {
        return QSize(100, 36);
    }
    // The owner handles mouse and keyboard toggles once; do not let Qt mutate
    // CheckStateRole independently of the selected-index set.
    bool editorEvent(QEvent*, QAbstractItemModel*, const QStyleOptionViewItem&,
                     const QModelIndex&) override {
        return false;
    }
    void paint(QPainter* p, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override {
        p->save();
        p->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
        const bool dark = isDarkTheme();
        const int shade = dark ? 255 : 0;
        const bool enabled = index.flags() & Qt::ItemIsEnabled;
        const bool checked = index.data(Qt::CheckStateRole).toInt() == Qt::Checked;
        const bool hover = enabled && (option.state & QStyle::State_MouseOver);
        if (!enabled) p->setOpacity(0.36);
        const auto background = QRectF(option.rect.adjusted(6, 2, -6, -2));
        p->setPen(Qt::NoPen);
        if (checked) {
            p->setBrush(QColor(shade, shade, shade, 9));
            p->drawRoundedRect(background, 5, 5);
        }
        if (hover) {
            p->setBrush(QColor(shade, shade, shade, 10));
            p->drawRoundedRect(background, 5, 5);
        }
        const QRect box = QStyle::visualRect(
            option.direction, option.rect,
            QRect(option.rect.x() + 14, option.rect.y() + (option.rect.height() - 18) / 2, 18, 18));
        p->setPen(QPen(checked ? themeColor()
                               : QColor(shade, shade, shade,
                                        dark    ? 141
                                        : hover ? 143
                                                : 122),
                       1.5));
        p->setBrush(checked
                        ? themeColor()
                        : QColor(shade, shade, shade, dark ? (hover ? 11 : 0) : (hover ? 13 : 6)));
        p->drawRoundedRect(QRectF(box), 4.5, 4.5);
        if (checked) {
            p->setPen(QPen(dark ? Qt::black : Qt::white, 1.7));
            QPolygonF tick;
            tick << QPointF(box.x() + 4, box.y() + 9) << QPointF(box.x() + 8, box.y() + 13)
                 << QPointF(box.x() + 14, box.y() + 5);
            p->drawPolyline(tick);
        }
        int start = 40;
        const auto icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
        if (!icon.isNull()) {
            icon.paint(
                p,
                QStyle::visualRect(option.direction, option.rect,
                                   QRect(option.rect.x() + start, option.rect.y() + 10, 16, 16)),
                Qt::AlignCenter, enabled ? QIcon::Normal : QIcon::Disabled);
            start += 24;
        }
        const auto text = QStyle::visualRect(option.direction, option.rect,
                                             option.rect.adjusted(start, 0, -8, 0));
        p->setFont(option.font);
        p->setPen(dark ? Qt::white : Qt::black);
        p->drawText(text,
                    Qt::AlignVCenter |
                        (option.direction == Qt::RightToLeft ? Qt::AlignRight : Qt::AlignLeft),
                    QFontMetrics(option.font)
                        .elidedText(index.data().toString(), Qt::ElideRight, text.width()));
        p->restore();
    }
};
}  // namespace

FontComboBox::FontComboBox(QWidget* parent) : ComboBox(parent) {
    setMaxVisibleItems(10);
    connect(this, &ComboBox::currentTextChanged, this, [this] {
        const auto family = currentText();
        if (lastFamily_ == family) return;
        lastFamily_ = family;
        emit currentFontChanged(currentFont());
    });
    populateFonts();
}
FontComboBox::~FontComboBox() {
    // Legacy menus may already be awaiting DeferredDelete after closing.
    // Destroy them while the mixin's state is still alive.
    const auto menus = findChildren<ComboBoxMenu*>();
    for (auto* menu : menus) delete menu;
}
int FontComboBox::fontFilters() const { return filters_; }
void FontComboBox::setFontFilters(int filters) {
    if (filters < 0 || (filters & ~15)) throw std::invalid_argument("Invalid font filters");
    if (filters_ == filters) return;
    filters_ = filters;
    populateFonts();
}
QFontDatabase::WritingSystem FontComboBox::writingSystem() const { return writingSystem_; }
void FontComboBox::setWritingSystem(QFontDatabase::WritingSystem system) {
    if (system < QFontDatabase::Any || system >= QFontDatabase::WritingSystemsCount)
        throw std::invalid_argument("Invalid writing system");
    if (writingSystem_ == system) return;
    writingSystem_ = system;
    populateFonts();
}
QFont FontComboBox::currentFont() const {
    return currentText().isEmpty() ? QFont() : QFont(currentText());
}
void FontComboBox::setCurrentFont(const QFont& font) {
    const int index = findText(font.family());
    if (index >= 0) setCurrentIndex(index);
}
void FontComboBox::setCurrentIndex(int index) {
    if (index < 0) index = -1;
    if (index >= count() || index == currentIndex_) return;
    const auto previous = currentText();
    currentIndex_ = index;
    setProperty("isPlaceholderText", index < 0);
    QPushButton::setText(index < 0 ? placeholderText_ : items[index].text);
    updateGeometry();
    update();
    const auto text = currentText();
    const auto revision = ++revision_;
    QPointer<FontComboBox> guard(this);
    emit currentIndexChanged(index);
    if (!guard || revision_ != revision) return;
    if (previous != text) emit currentTextChanged(text);
}
void FontComboBox::populateFonts() {
    hidePopup();
    const int previousIndex = currentIndex();
    const auto previousText = currentText();
    QFontDatabase database;
    const auto families = database.families(writingSystem_);
    {
        QSignalBlocker blocker(this);
        items.clear();
        currentIndex_ = -1;
        for (const auto& family : families) {
            const bool scalable = database.isSmoothlyScalable(family);
            const bool fixed = database.isFixedPitch(family);
            if (((filters_ & ScalableFonts) && !scalable) ||
                ((filters_ & NonScalableFonts) && scalable) ||
                ((filters_ & MonospacedFonts) && !fixed) ||
                ((filters_ & ProportionalFonts) && fixed))
                continue;
            items.append(ComboItem(family));
        }
        // Keep the chosen family if it survives a filter change.
        int index = findText(previousText);
        if (index < 0 && !items.isEmpty()) index = 0;
        if (index >= 0)
            setCurrentIndex(index);
        else {
            setProperty("isPlaceholderText", true);
            QPushButton::setText(placeholderText_);
            updateGeometry();
        }
    }
    const auto text = currentText();
    const int index = currentIndex();
    const auto revision = ++revision_;
    QPointer<FontComboBox> guard(this);
    if (previousIndex != index) emit currentIndexChanged(index);
    if (!guard || revision_ != revision) return;
    if (previousText != text) emit currentTextChanged(text);
}
void FontComboBox::clear() {
    hidePopup();
    const bool hadSelection = currentIndex_ >= 0;
    const auto previousText = currentText();
    items.clear();
    currentIndex_ = -1;
    QPushButton::setText(placeholderText_);
    setProperty("isPlaceholderText", true);
    updateGeometry();
    update();
    const auto revision = ++revision_;
    QPointer<FontComboBox> guard(this);
    if (hadSelection) emit currentIndexChanged(-1);
    if (!guard || revision_ != revision) return;
    if (!previousText.isEmpty()) emit currentTextChanged(QString());
}
ComboBoxMenu* FontComboBox::createComboMenu() {
    auto* menu = new ComboBoxMenu(this);
    menu->view()->setItemDelegate(new FontPreviewDelegate(menu));
    menu->view()->setLayoutDirection(layoutDirection());
    menu->view()->installEventFilter(this);
    return menu;
}
bool FontComboBox::eventFilter(QObject* object, QEvent* event) {
    if (object != this && qobject_cast<QListWidget*>(object) && event->type() == QEvent::KeyPress) {
        auto* key = static_cast<QKeyEvent*>(event);
        if (key->key() == Qt::Key_Escape) {
            hidePopup();
            return true;
        }
        if (key->key() == Qt::Key_Space || key->key() == Qt::Key_Return ||
            key->key() == Qt::Key_Enter) {
            auto* list = static_cast<QListWidget*>(object);
            const int index = list->currentRow();
            onItemClicked(index);
            return true;
        }
    }
    return ComboBox::eventFilter(object, event);
}
void FontComboBox::onItemClicked(int index) {
    if (index < 0 || index >= count() || !items[index].isEnabled) return;
    QPointer<FontComboBox> guard(this);
    setCurrentIndex(index);
    if (!guard || currentIndex() != index) return;
    const auto text = currentText();
    hidePopup();
    const auto revision = revision_;
    emit activated(index);
    if (guard && revision_ == revision) emit textActivated(text);
}
void FontComboBox::paintEvent(QPaintEvent*) {
    QStyleOptionButton option;
    initStyleOption(&option);
    drawComboButton(this, option, currentIndex() < 0);
}
void FontComboBox::showPopup() {
    if (isEnabled() && isVisible() && !isPopupVisible()) {
        if (dropMenu) hidePopup();
        showComboMenu(this);
    }
}
void FontComboBox::hidePopup() { closeComboMenu(); }
bool FontComboBox::isPopupVisible() const { return dropMenu && dropMenu->isVisible(); }
QListWidget* FontComboBox::popupView() const {
    return dropMenu ? dropMenu->view().data() : nullptr;
}
void FontComboBox::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Space || event->key() == Qt::Key_Return ||
        event->key() == Qt::Key_Enter || event->key() == Qt::Key_Down ||
        event->key() == Qt::Key_F4) {
        showPopup();
        if (popupView()) popupView()->setFocus(Qt::PopupFocusReason);
        event->accept();
        return;
    }
    if (event->key() == Qt::Key_Escape) {
        hidePopup();
        event->accept();
        return;
    }
    ComboBox::keyPressEvent(event);
}
void FontComboBox::mouseReleaseEvent(QMouseEvent* event) {
    // Unlike the legacy ComboBox, a right-button release must not open a menu.
    const bool activate =
        isDown() && event->button() == Qt::LeftButton && rect().contains(event->pos());
    QPointer<FontComboBox> guard(this);
    QPushButton::mouseReleaseEvent(event);
    if (!guard) return;
    if (activate) {
        if (isPopupVisible())
            hidePopup();
        else
            showPopup();
    }
}
void FontComboBox::hideEvent(QHideEvent* event) {
    hidePopup();
    ComboBox::hideEvent(event);
}
void FontComboBox::resizeEvent(QResizeEvent* event) {
    hidePopup();
    ComboBox::resizeEvent(event);
}
void FontComboBox::moveEvent(QMoveEvent* event) {
    hidePopup();
    ComboBox::moveEvent(event);
}
void FontComboBox::changeEvent(QEvent* event) {
    if ((event->type() == QEvent::EnabledChange && !isEnabled()) ||
        event->type() == QEvent::LayoutDirectionChange)
        hidePopup();
    ComboBox::changeEvent(event);
}

MultiSelectionComboBox::MultiSelectionComboBox(QWidget* parent) : QPushButton(parent) {
    setProperty("qssClass", "ComboBox");
    setProperty("comboBox", true);
    setProperty("isPlaceholderText", true);
    qfw::setStyleSheet(this, FluentStyleSheet::ComboBox);
    qfw::setFont(this);
    setFocusPolicy(Qt::StrongFocus);
    connect(this, &QAbstractButton::clicked, this, &MultiSelectionComboBox::togglePopup);
    connect(&QConfig::instance(), &QConfig::themeChanged, this, [this] { refreshTheme(); });
    connect(&QConfig::instance(), &QConfig::themeColorChanged, this, [this] { refreshTheme(); });
}
MultiSelectionComboBox::~MultiSelectionComboBox() { delete popup_; }
bool MultiSelectionComboBox::validIndex(int index) const {
    return index >= 0 && index < items_.size();
}
int MultiSelectionComboBox::count() const { return items_.size(); }
QString MultiSelectionComboBox::itemText(int index) const {
    return validIndex(index) ? items_[index].text : QString();
}
QVariant MultiSelectionComboBox::itemData(int index) const {
    return validIndex(index) ? items_[index].userData : QVariant();
}
QIcon MultiSelectionComboBox::itemIcon(int index) const {
    return validIndex(index) ? items_[index].icon() : QIcon();
}
bool MultiSelectionComboBox::isItemEnabled(int index) const {
    return validIndex(index) && items_[index].isEnabled;
}
void MultiSelectionComboBox::addItem(const QString& text, const QVariant& icon,
                                     const QVariant& data) {
    insertItem(count(), text, icon, data);
}
void MultiSelectionComboBox::addItems(const QStringList& texts) {
    for (const auto& text : texts) addItem(text);
}
void MultiSelectionComboBox::insertItem(int index, const QString& text, const QVariant& icon,
                                        const QVariant& data) {
    // Store QIcon values, not pointers into short-lived FluentIcon objects.
    const QIcon ownedIcon = ownedComboIcon(icon);
    index = std::clamp(index, 0, count());
    const auto previous = selected_;
    items_.insert(index, ComboItem(text, ownedIcon, data));
    identities_.insert(index, ++nextIdentity_);
    QSet<int> shifted;
    for (int selected : selected_) shifted.insert(selected >= index ? selected + 1 : selected);
    selected_ = shifted;
    rebuildPopup();
    publish(previous);
}
void MultiSelectionComboBox::removeItem(int index) {
    if (!validIndex(index)) return;
    const auto previous = selected_;
    items_.removeAt(index);
    identities_.removeAt(index);
    QSet<int> shifted;
    for (int selected : selected_)
        if (selected != index) shifted.insert(selected > index ? selected - 1 : selected);
    selected_ = shifted;
    rebuildPopup();
    publish(previous);
}
void MultiSelectionComboBox::clear() {
    const auto previous = selected_;
    items_.clear();
    identities_.clear();
    selected_.clear();
    hidePopup();
    rebuildPopup();
    publish(previous);
}
void MultiSelectionComboBox::setItemText(int index, const QString& text) {
    if (!validIndex(index) || items_[index].text == text) return;
    items_[index].text = text;
    rebuildPopup();
    publish(selected_);
}
void MultiSelectionComboBox::setItemData(int index, const QVariant& data) {
    if (validIndex(index)) items_[index].userData = data;
}
void MultiSelectionComboBox::setItemIcon(int index, const QVariant& icon) {
    if (!validIndex(index)) return;
    items_[index].setIcon(ownedComboIcon(icon));
    rebuildPopup();
}
void MultiSelectionComboBox::setItemEnabled(int index, bool enabled) {
    if (!validIndex(index) || items_[index].isEnabled == enabled) return;
    items_[index].isEnabled = enabled;
    rebuildPopup();
}
QSet<int> MultiSelectionComboBox::selectedIndices() const { return selected_; }
QList<ComboItem> MultiSelectionComboBox::selectedItems() const {
    QList<ComboItem> result;
    for (int i = 0; i < count(); ++i)
        if (selected_.contains(i)) result.append(items_[i]);
    return result;
}
QStringList MultiSelectionComboBox::selectedTexts() const {
    QStringList result;
    for (const auto& item : selectedItems()) result.append(item.text);
    return result;
}
QList<QVariant> MultiSelectionComboBox::selectedData() const {
    QList<QVariant> result;
    for (const auto& item : selectedItems()) result.append(item.userData);
    return result;
}
void MultiSelectionComboBox::setSelectedIndices(const QSet<int>& indexes) {
    QSet<int> valid;
    for (int index : indexes)
        if (validIndex(index)) valid.insert(index);
    if (valid == selected_) return;
    const auto previous = selected_;
    selected_ = valid;
    publish(previous);
}
void MultiSelectionComboBox::clearSelection() { setSelectedIndices({}); }
void MultiSelectionComboBox::publish(const QSet<int>& previous) {
    const auto oldTexts = lastTexts_;
    lastTexts_ = selectedTexts();
    const auto texts = lastTexts_;
    const auto selection = selected_;
    const auto revision = ++revision_;
    refreshDisplay();
    if (view_) {
        QSignalBlocker blocker(view_);
        for (int i = 0; i < view_->count(); ++i)
            view_->item(i)->setCheckState(selected_.contains(i) ? Qt::Checked : Qt::Unchecked);
        view_->viewport()->update();
    }
    QPointer<MultiSelectionComboBox> guard(this);
    if (previous != selection) emit selectionChanged(selection);
    if (!guard || revision_ != revision) return;
    if (oldTexts != texts) emit selectedTextChanged(texts);
}
void MultiSelectionComboBox::setPlaceholderText(const QString& text) {
    placeholder_ = text;
    refreshDisplay();
}
QString MultiSelectionComboBox::placeholderText() const { return placeholder_; }
void MultiSelectionComboBox::setChipsMode(bool enabled) {
    if (chipsMode_ == enabled) return;
    chipsMode_ = enabled;
    refreshDisplay();
}
bool MultiSelectionComboBox::isChipsMode() const { return chipsMode_; }
void MultiSelectionComboBox::setMaxVisibleItems(int count) {
    maxVisible_ = count < 0 ? -1 : std::clamp(count, 1, 200);
    hidePopup();
}
int MultiSelectionComboBox::maxVisibleItems() const { return maxVisible_; }
void MultiSelectionComboBox::refreshDisplay() {
    const auto texts = selectedTexts();
    QPushButton::setText(texts.isEmpty() ? placeholder_
                         : chipsMode_    ? QString()
                                         : texts.join(", "));
    setProperty("isPlaceholderText", texts.isEmpty());
    setAccessibleDescription(texts.join(", "));
    if (chipsMode_ && !chips_) {
        chips_ = new QScrollArea(this);
        chips_->setObjectName("comboSelectionChips");
        chips_->setFrameShape(QFrame::NoFrame);
        chips_->setWidgetResizable(true);
        chips_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        chips_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        chips_->setFocusPolicy(Qt::NoFocus);
        chips_->setAlignment(Qt::AlignVCenter | Qt::AlignLeading);
        chips_->setStyleSheet("background: transparent; border: none;");
        chipsContainer_ = new QWidget;
        chipsContainer_->setAutoFillBackground(false);
        chipsLayout_ = new QHBoxLayout(chipsContainer_);
        chipsLayout_->setSizeConstraint(QLayout::SetMinAndMaxSize);
        chipsLayout_->setContentsMargins(0, 0, 0, 0);
        chipsLayout_->setSpacing(4);
        chips_->setWidget(chipsContainer_);
        chips_->viewport()->installEventFilter(this);
        chipsContainer_->installEventFilter(this);
    }
    if (chips_) {
        // Identity keys keep queued close signals tied to an option even after
        // inserting/removing another row or using duplicate display names.
        while (chipsLayout_->count()) {
            auto* item = chipsLayout_->takeAt(0);
            if (auto* widget = item->widget()) {
                widget->hide();
                widget->deleteLater();
            }
            delete item;
        }
        if (chipsMode_ && !texts.isEmpty()) {
            for (int i = 0; i < count(); ++i) {
                if (!selected_.contains(i)) continue;
                auto* chip = new Chip(items_[i].text, chipsContainer_);
                chip->setObjectName("comboSelectionChip");
                chip->setCheckable(false);
                chip->setFixedHeight(26);
                chip->setFixedWidth(std::min(240, chip->sizeHint().width()));
                const auto identity = identities_[i];
                chip->setProperty("optionIdentity", QVariant::fromValue(identity));
                connect(chip, &Chip::closed, this, [this, identity] {
                    const int index = identities_.indexOf(identity);
                    if (index < 0) return;
                    auto selection = selected_;
                    selection.remove(index);
                    setSelectedIndices(selection);
                });
                connect(chip, &QAbstractButton::clicked, this, &MultiSelectionComboBox::showPopup);
                chipsLayout_->addWidget(chip);
            }
            chipsLayout_->addStretch();
            chips_->show();
            positionChips();
        } else
            chips_->hide();
    }
    updateGeometry();
    update();
}
void MultiSelectionComboBox::positionChips() {
    if (!chips_) return;
    chips_->setLayoutDirection(layoutDirection());
    chipsContainer_->setLayoutDirection(layoutDirection());
    chips_->setGeometry(QStyle::visualRect(layoutDirection(), rect(),
                                           QRect(10, 0, std::max(0, width() - 46), height())));
}
void MultiSelectionComboBox::toggleIndex(int index) {
    if (!isItemEnabled(index)) return;
    auto selection = selected_;
    if (selection.contains(index))
        selection.remove(index);
    else
        selection.insert(index);
    setSelectedIndices(selection);
}
void MultiSelectionComboBox::rebuildPopup() {
    if (!view_) return;
    const int current = view_->currentRow();
    const int scroll = view_->verticalScrollBar()->value();
    QSignalBlocker blocker(view_);
    view_->clear();
    for (int i = 0; i < count(); ++i) {
        auto* item = new QListWidgetItem(items_[i].icon(), items_[i].text, view_);
        item->setSizeHint(QSize(100, 36));
        item->setFlags(items_[i].isEnabled ? Qt::ItemIsEnabled | Qt::ItemIsSelectable
                                           : Qt::NoItemFlags);
        item->setCheckState(selected_.contains(i) ? Qt::Checked : Qt::Unchecked);
    }
    if (count()) view_->setCurrentRow(std::clamp(current, 0, count() - 1));
    view_->verticalScrollBar()->setValue(scroll);
    // Close on structural/data mutation; next open recomputes height and screen bounds.
    hidePopup();
}
void MultiSelectionComboBox::refreshTheme() {
    if (panel_) {
        panel_->setStyleSheet(
            QStringLiteral(
                "QFrame#multiComboPanel {background:%1; border:1px solid %2; border-radius:8px;}"
                "QListWidget#multiComboView {background:transparent; border:none; outline:none;}")
                .arg(isDarkTheme() ? "#2b2b2b" : "#fafafa", isDarkTheme() ? "#454545" : "#dddddd"));
        view_->viewport()->update();
    }
    update();
}
void MultiSelectionComboBox::showPopup() {
    if (!isVisible() || !isEnabled() || items_.isEmpty() || isPopupVisible()) return;
    if (!popup_) {
        popup_ = new QFrame(this, Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
        popup_->setObjectName("multiComboPopup");
        popup_->setAttribute(Qt::WA_TranslucentBackground);
        popup_->installEventFilter(this);
        auto* outer = new QVBoxLayout(popup_);
        outer->setContentsMargins(12, 12, 12, 12);
        panel_ = new QFrame(popup_);
        panel_->setObjectName("multiComboPanel");
        auto* shadow = new QGraphicsDropShadowEffect(panel_);
        shadow->setBlurRadius(20);
        shadow->setOffset(0, 3);
        shadow->setColor(QColor(0, 0, 0, 65));
        panel_->setGraphicsEffect(shadow);
        outer->addWidget(panel_);
        auto* layout = new QVBoxLayout(panel_);
        layout->setContentsMargins(0, 5, 0, 5);
        view_ = new QListWidget(panel_);
        view_->setObjectName("multiComboView");
        qfw::setFont(view_, 14);
        view_->setItemDelegate(new MultiComboDelegate(view_));
        view_->setSelectionMode(QAbstractItemView::NoSelection);
        view_->setEditTriggers(QAbstractItemView::NoEditTriggers);
        view_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        view_->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        view_->setMouseTracking(true);
        auto* scrolling = new SmoothScrollDelegate(view_, true);
        scrolling->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        scrolling->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        view_->viewport()->setAutoFillBackground(false);
        view_->installEventFilter(this);
        layout->addWidget(view_);
        connect(view_, &QListWidget::itemClicked, this,
                [this](QListWidgetItem* item) { toggleIndex(view_->row(item)); });
        animation_ = new QParallelAnimationGroup(popup_);
        for (const char* property : {"pos", "windowOpacity"}) {
            auto* animation = new QPropertyAnimation(popup_, property, animation_);
            animation->setDuration(160);
            animation->setEasingCurve(QEasingCurve::OutQuad);
            animation_->addAnimation(animation);
        }
    }
    rebuildPopup();
    refreshTheme();
    view_->setLayoutDirection(layoutDirection());
    if (!selected_.isEmpty()) {
        int first = count();
        for (int index : selected_) first = std::min(index, first);
        view_->setCurrentRow(first);
    }
    QScreen* screen = QApplication::screenAt(mapToGlobal(QPoint(0, 0)));
    if (!screen) screen = QApplication::primaryScreen();
    if (!screen) return;
    const QRect available = screen->availableGeometry().adjusted(4, 4, -4, -4);
    const QPoint below = mapToGlobal(QPoint(0, height()));
    const QPoint above = mapToGlobal(QPoint(0, 0));
    const int rows = std::min(count(), maxVisible_ < 0 ? 12 : maxVisible_);
    const int wanted = rows * 36 + 36;
    const int belowSpace = std::max(0, available.bottom() - below.y() + 1);
    const int aboveSpace = std::max(0, above.y() - available.top());
    const bool upwards = wanted > belowSpace && aboveSpace > belowSpace;
    const int h = std::min(wanted, std::max(1, upwards ? aboveSpace : belowSpace));
    const int w = std::min(std::max(width() + 24, 200), available.width());
    const int anchorX =
        layoutDirection() == Qt::RightToLeft ? below.x() + width() + 12 - w : below.x() - 12;
    const int x = std::clamp(anchorX, available.left(), available.right() - w + 1);
    const int y = std::clamp(upwards ? above.y() - h : below.y(), available.top(),
                             available.bottom() - h + 1);
    const QPoint end(x, y), start = end + QPoint(0, upwards ? 8 : -8);
    animation_->stop();
    popup_->setGeometry(QRect(start, QSize(w, h)));
    auto* slide = static_cast<QPropertyAnimation*>(animation_->animationAt(0));
    auto* opacity = static_cast<QPropertyAnimation*>(animation_->animationAt(1));
    slide->setStartValue(start);
    slide->setEndValue(end);
    opacity->setStartValue(0.0);
    opacity->setEndValue(1.0);
    popup_->setWindowOpacity(0);
    popup_->show();
    popup_->activateWindow();
    animation_->start();
    view_->setFocus(Qt::PopupFocusReason);
    if (view_->currentItem()) view_->scrollToItem(view_->currentItem());
}
void MultiSelectionComboBox::hidePopup() {
    if (animation_) animation_->stop();
    if (popup_ && popup_->isVisible()) {
        popup_->hide();
        if (isVisible() && isEnabled()) setFocus(Qt::PopupFocusReason);
    }
}
void MultiSelectionComboBox::togglePopup() {
    if (isPopupVisible())
        hidePopup();
    else
        showPopup();
}
bool MultiSelectionComboBox::isPopupVisible() const { return popup_ && popup_->isVisible(); }
QListWidget* MultiSelectionComboBox::popupView() const { return view_; }
QSize MultiSelectionComboBox::sizeHint() const {
    return QSize(std::clamp(QFontMetrics(font()).horizontalAdvance(text()) + 44, 120, 480), 32);
}
QSize MultiSelectionComboBox::minimumSizeHint() const { return QSize(60, 32); }
void MultiSelectionComboBox::paintEvent(QPaintEvent*) {
    QStyleOptionButton option;
    initStyleOption(&option);
    drawComboButton(this, option, selected_.isEmpty());
}
void MultiSelectionComboBox::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter ||
        event->key() == Qt::Key_Space || event->key() == Qt::Key_Down ||
        event->key() == Qt::Key_F4) {
        showPopup();
        event->accept();
        return;
    }
    if (event->key() == Qt::Key_Escape) {
        hidePopup();
        event->accept();
        return;
    }
    QPushButton::keyPressEvent(event);
}
bool MultiSelectionComboBox::eventFilter(QObject* object, QEvent* event) {
    if (object == popup_ && event->type() == QEvent::Hide && animation_) animation_->stop();
    if (object == view_ && event->type() == QEvent::KeyPress) {
        auto* key = static_cast<QKeyEvent*>(event);
        if (key->key() == Qt::Key_Escape) {
            hidePopup();
            return true;
        }
        if (key->key() == Qt::Key_Space || key->key() == Qt::Key_Enter ||
            key->key() == Qt::Key_Return) {
            toggleIndex(view_->currentRow());
            return true;
        }
    }
    if (chips_ && (object == chips_->viewport() || object == chipsContainer_)) {
        if (event->type() == QEvent::Wheel) {
            auto* wheel = static_cast<QWheelEvent*>(event);
            const auto pixels = wheel->pixelDelta();
            const auto angle = wheel->angleDelta();
            const int delta = !pixels.isNull() ? (pixels.x() ? pixels.x() : pixels.y())
                                               : (angle.x() ? angle.x() : angle.y()) / 2;
            chips_->horizontalScrollBar()->setValue(chips_->horizontalScrollBar()->value() - delta);
            return true;
        }
        if (event->type() == QEvent::Hide) chipsPressed_ = false;
        if (event->type() == QEvent::MouseButtonPress) {
            chipsPressed_ = static_cast<QMouseEvent*>(event)->button() == Qt::LeftButton;
            return true;
        }
        if (event->type() == QEvent::MouseButtonRelease) {
            auto* mouse = static_cast<QMouseEvent*>(event);
            const bool activate = chipsPressed_ && mouse->button() == Qt::LeftButton &&
                                  static_cast<QWidget*>(object)->rect().contains(mouse->pos());
            chipsPressed_ = false;
            if (activate) togglePopup();
            return true;
        }
    }
    return QPushButton::eventFilter(object, event);
}
void MultiSelectionComboBox::hideEvent(QHideEvent* event) {
    hidePopup();
    QPushButton::hideEvent(event);
}
void MultiSelectionComboBox::resizeEvent(QResizeEvent* event) {
    hidePopup();
    QPushButton::resizeEvent(event);
    positionChips();
}
void MultiSelectionComboBox::moveEvent(QMoveEvent* event) {
    hidePopup();
    QPushButton::moveEvent(event);
}
void MultiSelectionComboBox::changeEvent(QEvent* event) {
    if (event->type() == QEvent::EnabledChange && !isEnabled()) hidePopup();
    if (event->type() == QEvent::LayoutDirectionChange) {
        hidePopup();
        positionChips();
    }
    if (event->type() == QEvent::FontChange) {
        updateGeometry();
        update();
    }
    QPushButton::changeEvent(event);
}
}  // namespace qfw
