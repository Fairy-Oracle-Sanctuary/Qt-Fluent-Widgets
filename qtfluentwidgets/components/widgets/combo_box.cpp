#include "components/widgets/combo_box.h"

#include <QApplication>
#include <QCursor>
#include <QEvent>
#include <QMouseEvent>
#include <QObject>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>

#include "common/style_sheet.h"

namespace qfw {

// ============================================================================
// ComboItem
// ============================================================================

ComboItem::ComboItem(const QString& text, const QVariant& icon, const QVariant& userData,
                     bool isEnabled)
    : text(text), userData(userData), isEnabled(isEnabled), icon_(icon) {}

QIcon ComboItem::icon() const {
    if (icon_.canConvert<QIcon>()) {
        return icon_.value<QIcon>();
    } else if (icon_.canConvert<const FluentIconBase*>()) {
        const auto* fluentIcon = icon_.value<const FluentIconBase*>();
        return fluentIcon ? fluentIcon->icon() : QIcon();
    }
    return QIcon();
}

void ComboItem::setIcon(const QVariant& icon) { icon_ = icon; }

// ============================================================================
// ComboBoxBase
// ============================================================================

void ComboBoxBase::setUpUi(QWidget* widget) {
    isHover = false;
    isPressed = false;
    items.clear();
    currentIndex_ = -1;
    maxVisibleItems_ = -1;
    dropMenu = nullptr;

    qfw::setStyleSheet(widget, FluentStyleSheet::ComboBox);
    widget->installEventFilter(widget);
}

void ComboBoxBase::addItem(const QString& text, const QVariant& icon, const QVariant& userData) {
    items.append(ComboItem(text, icon, userData));
    if (items.size() == 1 && placeholderText_.isEmpty()) {
        setCurrentIndex(0);
    }
}

void ComboBoxBase::addItems(const QStringList& texts) {
    for (const auto& text : texts) {
        addItem(text);
    }
}

void ComboBoxBase::removeItem(int index) {
    if (index < 0 || index >= items.size()) return;

    items.removeAt(index);

    if (index < currentIndex_) {
        setCurrentIndex(currentIndex_ - 1);
    } else if (index == currentIndex_) {
        if (index > 0) {
            setCurrentIndex(currentIndex_ - 1);
        } else if (!items.isEmpty()) {
            setText(items[0].text);
            currentIndex_ = 0;
        } else {
            currentIndex_ = -1;
            setText("");
        }
    }
}

int ComboBoxBase::currentIndex() const { return currentIndex_; }

void ComboBoxBase::setCurrentIndex(int index) {
    if (index < 0 || index >= items.size() || index == currentIndex_) return;

    currentIndex_ = index;
    setText(items[index].text);
}

QString ComboBoxBase::currentText() const {
    if (currentIndex_ < 0 || currentIndex_ >= items.size()) return "";
    return items[currentIndex_].text;
}

QVariant ComboBoxBase::currentData() const {
    if (currentIndex_ < 0 || currentIndex_ >= items.size()) return QVariant();
    return items[currentIndex_].userData;
}

void ComboBoxBase::setCurrentText(const QString& text) {
    int index = findText(text);
    if (index >= 0) {
        setCurrentIndex(index);
    }
}

void ComboBoxBase::setItemText(int index, const QString& text) {
    if (index >= 0 && index < items.size()) {
        items[index].text = text;
        if (currentIndex_ == index) {
            setText(text);
        }
    }
}

QVariant ComboBoxBase::itemData(int index) const {
    if (index >= 0 && index < items.size()) return items[index].userData;
    return QVariant();
}

QString ComboBoxBase::itemText(int index) const {
    if (index >= 0 && index < items.size()) return items[index].text;
    return "";
}

QIcon ComboBoxBase::itemIcon(int index) const {
    if (index >= 0 && index < items.size()) return items[index].icon();
    return QIcon();
}

void ComboBoxBase::setItemData(int index, const QVariant& value) {
    if (index >= 0 && index < items.size()) items[index].userData = value;
}

void ComboBoxBase::setItemIcon(int index, const QVariant& icon) {
    if (index >= 0 && index < items.size()) items[index].setIcon(icon);
}

void ComboBoxBase::setItemEnabled(int index, bool isEnabled) {
    if (index >= 0 && index < items.size()) items[index].isEnabled = isEnabled;
}

int ComboBoxBase::findData(const QVariant& data) const {
    for (int i = 0; i < items.size(); ++i) {
        if (items[i].userData == data) return i;
    }
    return -1;
}

int ComboBoxBase::findText(const QString& text) const {
    for (int i = 0; i < items.size(); ++i) {
        if (items[i].text == text) return i;
    }
    return -1;
}

void ComboBoxBase::clear() {
    if (currentIndex_ >= 0) {
        setText("");
    }
    items.clear();
    currentIndex_ = -1;
}

int ComboBoxBase::count() const { return items.size(); }

void ComboBoxBase::insertItem(int index, const QString& text, const QVariant& icon,
                              const QVariant& userData) {
    items.insert(qBound(0, index, items.size()), ComboItem(text, icon, userData));
    if (index <= currentIndex_) {
        currentIndex_++;
    }
}

void ComboBoxBase::insertItems(int index, const QStringList& texts) {
    int pos = qBound(0, index, items.size());
    for (const auto& text : texts) {
        items.insert(pos++, ComboItem(text));
    }
    if (index <= currentIndex_) {
        currentIndex_ += texts.size();
    }
}

void ComboBoxBase::setMaxVisibleItems(int num) { maxVisibleItems_ = num; }

int ComboBoxBase::maxVisibleItems() const { return maxVisibleItems_; }

void ComboBoxBase::closeComboMenu() {
    if (!dropMenu) {
        return;
    }

    // menu could already be in deletion
    dropMenu->close();
    dropMenu.clear();
}

void ComboBoxBase::onDropMenuClosed() {
    if (!ownerWidget_) {
        dropMenu.clear();
        return;
    }

    QPoint pos = ownerWidget_->mapFromGlobal(QCursor::pos());
    if (!ownerWidget_->rect().contains(pos)) {
        dropMenu.clear();
    }
}

ComboBoxMenu* ComboBoxBase::createComboMenu() {
    return new ComboBoxMenu(dynamic_cast<QWidget*>(this));
}

void ComboBoxBase::showComboMenu(QWidget* widget) {
    if (items.isEmpty()) return;

    ownerWidget_ = widget;

    ComboBoxMenu* menu = createComboMenu();
    for (const auto& item : items) {
        QAction* action = new QAction(item.icon(), item.text, menu);
        action->setEnabled(item.isEnabled);
        menu->addAction(action);
    }

    QObject::connect(menu->view(), &QListWidget::itemClicked, [this, menu](QListWidgetItem* item) {
        onItemClicked(findText(item->text().trimmed()));
    });

    if (menu->view()->width() < widget->width()) {
        menu->view()->setMinimumWidth(widget->width());
        menu->adjustSize();
    }

    menu->setMaxVisibleItems(maxVisibleItems());
    menu->setAttribute(Qt::WA_DeleteOnClose);
    QObject::connect(menu, &RoundMenu::closedSignal, [this]() { onDropMenuClosed(); });
    QObject::connect(menu, &QObject::destroyed, [this]() { dropMenu.clear(); });
    dropMenu = menu;

    if (currentIndex_ >= 0 && currentIndex_ < menu->view()->count()) {
        if (auto* it = menu->view()->item(currentIndex_)) {
            menu->view()->setCurrentItem(it);
            it->setSelected(true);
        }
    }

    int x = -menu->width() / 2 + menu->layout()->contentsMargins().left() + widget->width() / 2;
    QPoint pd = widget->mapToGlobal(QPoint(x, widget->height()));
    int hd = menu->view()->heightForAnimation(pd, MenuAnimationType::DropDown);

    QPoint pu = widget->mapToGlobal(QPoint(x, 0));
    int hu = menu->view()->heightForAnimation(pu, MenuAnimationType::PullUp);

    if (hd >= hu) {
        menu->execAt(pd, true, MenuAnimationType::DropDown);
    } else {
        menu->execAt(pu, true, MenuAnimationType::PullUp);
    }

    // Ensure selection is visible and processed by delegate after menu is shown
    if (currentIndex_ >= 0 && menu->view()) {
        if (auto* it = menu->view()->item(currentIndex_)) {
            it->setSelected(true);
            menu->view()->setCurrentItem(it);
            menu->view()->scrollToItem(it);
        }
    }
}

void ComboBoxBase::toggleComboMenu(QWidget* widget) {
    if (dropMenu) {
        closeComboMenu();
    } else {
        showComboMenu(widget);
    }
}

void ComboBoxBase::onItemClicked(int index) {
    if (index < 0 || index >= items.size() || !items[index].isEnabled) return;

    if (index != currentIndex_) {
        setCurrentIndex(index);
    }
}

// ============================================================================
// ComboBox
// ============================================================================

ComboBox::ComboBox(QWidget* parent) : QPushButton(parent) {
    setProperty("qssClass", "ComboBox");
    setProperty("comboBox", true);
    setUpUi(this);
    qfw::setFont(this);
    installEventFilter(this);
}

bool ComboBox::eventFilter(QObject* obj, QEvent* e) {
    if (obj == this) {
        switch (e->type()) {
            case QEvent::MouseButtonPress:
                isPressed = true;
                update();
                break;
            case QEvent::MouseButtonRelease:
                isPressed = false;
                update();
                break;
            case QEvent::Enter:
                isHover = true;
                update();
                break;
            case QEvent::Leave:
                isHover = false;
                update();
                break;
            default:
                break;
        }
    }

    return QPushButton::eventFilter(obj, e);
}

void ComboBox::setPlaceholderText(const QString& text) {
    placeholderText_ = text;

    if (currentIndex() < 0) {
        updateTextState(true);
        setText(text);
    }
}

void ComboBox::setCurrentIndex(int index) {
    if (index == currentIndex()) {
        return;
    }

    const QString oldText = text();

    if (index < 0) {
        currentIndex_ = -1;
        updateTextState(true);
        setText(placeholderText_);
        if (oldText != text()) {
            emit currentTextChanged(text());
        }
        emit currentIndexChanged(-1);
        return;
    }

    if (index >= items.size()) {
        return;
    }

    updateTextState(false);
    currentIndex_ = index;
    setText(items[index].text);

    if (oldText != text()) {
        emit currentTextChanged(text());
    }
    emit currentIndexChanged(index);
}

void ComboBox::setText(const QString& text) {
    QPushButton::setText(text);
    adjustSize();
}

void ComboBox::updateTextState(bool isPlaceholder) {
    if (property("isPlaceholderText").toBool() == isPlaceholder) {
        return;
    }

    setProperty("isPlaceholderText", isPlaceholder);
    setStyle(QApplication::style());
}

void ComboBox::mouseReleaseEvent(QMouseEvent* e) {
    QPushButton::mouseReleaseEvent(e);
    toggleComboMenu(this);
}

void ComboBox::paintEvent(QPaintEvent* e) {
    Q_UNUSED(e);

    QStyleOptionButton opt;
    initStyleOption(&opt);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform |
                           QPainter::TextAntialiasing);
    painter.setFont(font());

    // draw button background
    style()->drawControl(QStyle::CE_PushButtonBevel, &opt, &painter, this);

    // draw text manually for consistent font rendering
    if (!opt.text.isEmpty()) {
        QColor textColor = opt.palette.buttonText().color();
        if (!isEnabled()) {
            painter.setOpacity(0.3628);
            textColor = isDarkTheme() ? QColor(255, 255, 255, 92) : QColor(0, 0, 0, 92);
        } else if (isPressed) {
            painter.setOpacity(0.786);
            textColor = isDarkTheme() ? QColor(255, 255, 255, 161) : QColor(0, 0, 0, 161);
        }

        QRect contentRect = style()->subElementRect(QStyle::SE_PushButtonContents, &opt, this);

        painter.setPen(textColor);
        painter.drawText(contentRect, Qt::AlignVCenter | Qt::AlignLeft, opt.text);
    }

    // draw arrow icon
    if (isHover) {
        painter.setOpacity(0.8);
    } else if (isPressed) {
        painter.setOpacity(0.7);
    } else {
        painter.setOpacity(1.0);
    }

    const QRect rect(width() - 22, height() / 2 - 5, 10, 10);
    const FluentIcon arrowDown(FluentIconEnum::ArrowDown);

    if (isDarkTheme()) {
        arrowDown.render(&painter, rect);
    } else {
        QVariantMap attrs;
        attrs.insert(QStringLiteral("fill"), QStringLiteral("#646464"));
        attrs.insert(QStringLiteral("stroke"), QStringLiteral("#646464"));
        arrowDown.render(&painter, rect, Theme::Auto, attrs);
    }
}

void ComboBox::onItemClicked(int index) {
    if (index < 0 || index >= items.size() || !items[index].isEnabled) {
        return;
    }

    if (index != currentIndex()) {
        setCurrentIndex(index);
    }

    emit activated(index);
    emit textActivated(currentText());
}

// ============================================================================
// EditableComboBox
// ============================================================================

EditableComboBox::EditableComboBox(QWidget* parent) : LineEdit(parent) {
    // Don't set comboBox property - EditableComboBox uses LineEdit styling only
    // (unlike ComboBox which is a QPushButton)
    isHover = false;
    isPressed = false;
    items.clear();
    currentIndex_ = -1;
    maxVisibleItems_ = -1;
    dropMenu = nullptr;

    // Apply LineEdit style (not ComboBox style - Python version only uses LINE_EDIT)
    qfw::setStyleSheet(this, FluentStyleSheet::LineEdit);
    installEventFilter(this);

    static qfw::FluentIcon arrowDownIcon(qfw::FluentIconEnum::ArrowDown);
    dropButton_ = new LineEditButton(QVariant(arrowDownIcon.qicon()), this);

    setTextMargins(0, 0, 29, 0);
    dropButton_->setFixedSize(30, 25);
    if (auto* layout = buttonLayout()) {
        layout->addWidget(dropButton_, 0, Qt::AlignRight);
    }

    connect(dropButton_, &QToolButton::clicked, this, [this]() { toggleComboMenu(this); });
    connect(this, &QLineEdit::textChanged, this, &EditableComboBox::onComboTextChanged);
    connect(this, &QLineEdit::returnPressed, this, &EditableComboBox::onReturnPressed);

    // override clear button behavior to keep currentIndex_ in sync
    if (auto* clearBtn = findChild<LineEditButton*>(QStringLiteral("lineEditButton"))) {
        disconnect(clearBtn, nullptr, this, nullptr);
    }
    connect(this, &QLineEdit::textChanged, this, [this](const QString& t) { Q_UNUSED(t); });

    installEventFilter(this);
}

bool EditableComboBox::eventFilter(QObject* obj, QEvent* e) {
    if (obj == this) {
        switch (e->type()) {
            case QEvent::MouseButtonPress:
                isPressed = true;
                break;
            case QEvent::MouseButtonRelease:
                isPressed = false;
                break;
            case QEvent::Enter:
                isHover = true;
                break;
            case QEvent::Leave:
                isHover = false;
                break;
            default:
                break;
        }
    }

    return LineEdit::eventFilter(obj, e);
}

void EditableComboBox::setPlaceholderText(const QString& text) {
    placeholderText_ = text;
    LineEdit::setPlaceholderText(text);
}

QString EditableComboBox::currentText() const { return text(); }

void EditableComboBox::setCurrentIndex(int index) {
    if (index >= count() || index == currentIndex()) {
        return;
    }

    if (index < 0) {
        currentIndex_ = -1;
        setText(QString());
        LineEdit::setPlaceholderText(placeholderText_);
    } else {
        currentIndex_ = index;
        setText(items[index].text);
    }

    emit currentIndexChanged(currentIndex_);
    emit currentTextChanged(text());
}

void EditableComboBox::setText(const QString& text) { LineEdit::setText(text); }

void EditableComboBox::clear() {
    ComboBoxBase::clear();
    LineEdit::clear();
    currentIndex_ = -1;
}

void EditableComboBox::onComboTextChanged(const QString& text) {
    currentIndex_ = -1;
    emit currentTextChanged(text);

    for (int i = 0; i < items.size(); ++i) {
        if (items[i].text == text) {
            currentIndex_ = i;
            emit currentIndexChanged(i);
            return;
        }
    }
}

void EditableComboBox::onReturnPressed() {
    if (text().isEmpty()) {
        return;
    }

    int index = findText(text());
    if (index >= 0 && index != currentIndex_) {
        currentIndex_ = index;
        emit currentIndexChanged(index);
        emit activated(index);
        emit textActivated(currentText());
    } else if (index == -1) {
        addItem(text());
        setCurrentIndex(count() - 1);
        emit activated(currentIndex_);
        emit textActivated(currentText());
    }
}

void EditableComboBox::onDropMenuClosed() { dropMenu = nullptr; }

void EditableComboBox::onClearButtonClicked() {
    LineEdit::clear();
    currentIndex_ = -1;
}

void EditableComboBox::onItemClicked(int index) {
    if (index < 0 || index >= items.size() || !items[index].isEnabled) {
        return;
    }

    if (index != currentIndex_) {
        currentIndex_ = index;
        setText(items[index].text);
        emit currentIndexChanged(index);
        emit currentTextChanged(text());
    }

    emit activated(index);
    emit textActivated(currentText());
}

// ============================================================================
// ComboBoxMenu
// ============================================================================

ComboBoxMenu::ComboBoxMenu(QWidget* parent) : RoundMenu("", parent) {
    view()->setViewportMargins(0, 2, 0, 6);
    view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    view()->setItemDelegate(new IndicatorMenuItemDelegate(this));
    view()->setObjectName("comboListWidget");
    setItemHeight(33);
}

void ComboBoxMenu::execAt(const QPoint& pos, bool ani, MenuAnimationType aniType) {
    view()->adjustSizeForMenu(pos, aniType);
    adjustSize();
    RoundMenu::execAt(pos, ani, aniType);
}

}  // namespace qfw
