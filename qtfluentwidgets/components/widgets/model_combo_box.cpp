#include "components/widgets/model_combo_box.h"

#include <QAction>
#include <QApplication>
#include <QCursor>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QStandardItem>
#include <QToolButton>

#include "common/style_sheet.h"
#include "components/widgets/combo_box.h"
#include "components/widgets/line_edit.h"

namespace qfw {

ModelComboBoxBase::ModelComboBoxBase() = default;

void ModelComboBoxBase::setUpUi() {
    isHover_ = false;
    isPressed_ = false;
    currentIndex_ = -1;
    maxVisibleItems_ = -1;
    dropMenu_ = nullptr;
    placeholderText_.clear();

    setModel(new QStandardItemModel(asWidget()));

    qfw::setStyleSheet(asWidget(), FluentStyleSheet::ComboBox);
    asWidget()->installEventFilter(asWidget());
}

void ModelComboBoxBase::setModel(QAbstractItemModel* model) {
    if (model_) {
        model_->disconnect(asWidget());
    }

    model_ = model;
    QObject::connect(model, &QAbstractItemModel::rowsInserted, asWidget(),
                     [this](const QModelIndex& parent, int first, int last) {
                         onModelRowInserted(parent, first, last);
                     });
    QObject::connect(
        model, &QAbstractItemModel::dataChanged, asWidget(),
        [this](const QModelIndex& topLeft, const QModelIndex& bottomRight,
               const QList<int>& roles) { onModelDataChanged(topLeft, bottomRight, roles); });
    QObject::connect(model, &QAbstractItemModel::rowsRemoved, asWidget(),
                     [this](const QModelIndex& parent, int first, int last) {
                         onRowsRemoved(parent, first, last);
                     });
}

void ModelComboBoxBase::onModelRowInserted(const QModelIndex& parentIndex, int first, int last) {
    Q_UNUSED(parentIndex);
    Q_UNUSED(last);
    if (first <= currentIndex_) {
        setCurrentIndex(currentIndex_ + (last - first + 1));
    }
}

void ModelComboBoxBase::onRowsRemoved(const QModelIndex& parentIndex, int first, int last) {
    Q_UNUSED(parentIndex);
    if (last < currentIndex_) {
        setCurrentIndex(currentIndex_ - (last - first + 1));
    } else if (first < currentIndex_ && currentIndex_ <= last) {
        setCurrentIndex(qMax(first - 1, 0));
    }

    if (count() == 0) {
        clear();
    }
}

void ModelComboBoxBase::onModelDataChanged(const QModelIndex& topLeft,
                                           const QModelIndex& bottomRight,
                                           const QList<int>& roles) {
    if (roles.contains(Qt::EditRole)) {
        for (int row = topLeft.row(); row <= bottomRight.row(); ++row) {
            setItemText(row, itemText(row));
        }
    }
    if (roles.contains(Qt::DecorationRole)) {
        for (int row = topLeft.row(); row <= bottomRight.row(); ++row) {
            setItemIcon(row, itemIcon(row));
        }
    }
}

bool ModelComboBoxBase::eventFilter(QObject* obj, QEvent* e) {
    if (obj == asWidget()) {
        if (e->type() == QEvent::MouseButtonPress) {
            isPressed_ = true;
        } else if (e->type() == QEvent::MouseButtonRelease) {
            isPressed_ = false;
        } else if (e->type() == QEvent::Enter) {
            isHover_ = true;
        } else if (e->type() == QEvent::Leave) {
            isHover_ = false;
        }
    }
    return false;
}

QModelIndex ModelComboBoxBase::insertItem(int index, const QString& text, const QVariant& userData,
                                          const QIcon& icon) {
    QMap<int, QVariant> values;
    values[Qt::EditRole] = text;

    if (!icon.isNull()) {
        values[Qt::DecorationRole] = icon;
    }

    if (userData.isValid()) {
        values[Qt::UserRole] = userData;
    }

    QModelIndex modelIndex = insertItemFromValues(index, values);

    if (index <= currentIndex_) {
        setCurrentIndex(currentIndex_ + 1);
    }

    return modelIndex;
}

void ModelComboBoxBase::insertItems(int index, const QStringList& texts) {
    asWidget()->blockSignals(true);

    int row = index;
    for (const QString& text : texts) {
        QMap<int, QVariant> values;
        values[Qt::EditRole] = text;
        insertItemFromValues(index, values);
        row++;
    }

    asWidget()->blockSignals(false);

    if (index <= currentIndex_) {
        setCurrentIndex(currentIndex_ + row - index);
    }
}

QModelIndex ModelComboBoxBase::insertItemFromValues(int row, const QMap<int, QVariant>& values) {
    QModelIndex ret;

    model_->blockSignals(true);

    row = qBound(0, row, model_->rowCount());

    auto* standardModel = qobject_cast<QStandardItemModel*>(model_);
    if (standardModel) {
        auto* item = new QStandardItem();
        for (auto it = values.constBegin(); it != values.constEnd(); ++it) {
            item->setData(it.value(), it.key());
        }
        standardModel->insertRow(row, item);
        ret = item->index();
    } else if (model_->insertRows(row, 1)) {
        ret = model_->index(row, 0);
        if (!values.isEmpty()) {
            model_->setItemData(ret, values);
        }
    }

    model_->blockSignals(false);
    return ret;
}

void ModelComboBoxBase::addItem(const QString& text, const QVariant& userData, const QIcon& icon) {
    insertItem(model_->rowCount(), text, userData, icon);
    if (count() == 1 && placeholderText_.isEmpty()) {
        setCurrentIndex(0);
    }
}

void ModelComboBoxBase::addItems(const QStringList& texts) {
    for (const QString& text : texts) {
        addItem(text);
    }
}

void ModelComboBoxBase::removeItem(int index) {
    if (!isValidIndex(index)) {
        return;
    }

    model_->blockSignals(true);
    model_->removeRow(index);
    model_->blockSignals(false);

    if (index < currentIndex_) {
        setCurrentIndex(currentIndex_ - 1);
    } else if (index == currentIndex_) {
        if (index > 0) {
            setCurrentIndex(currentIndex_ - 1);
        } else {
            setText(itemText(0));
            // Emit signals manually since we're not going through setCurrentIndex
        }
    }

    if (count() == 0) {
        clear();
    }
}

void ModelComboBoxBase::setCurrentIndex(int index) {
    if (!isValidIndex(index) || index == currentIndex_) {
        return;
    }

    QString oldText = currentText();

    currentIndex_ = index;
    setText(itemText(index));

    if (oldText != currentText()) {
        // Emit currentTextChanged signal - needs to be done via widget signals
    }
    // Emit currentIndexChanged signal
}

void ModelComboBoxBase::setCurrentText(const QString& text) {
    if (text == currentText()) {
        return;
    }

    int index = findText(text);
    if (index >= 0) {
        setCurrentIndex(index);
    }
}

QString ModelComboBoxBase::currentText() const { return itemText(currentIndex_); }

QVariant ModelComboBoxBase::currentData() const { return itemData(currentIndex_); }

QVariant ModelComboBoxBase::itemData(int index) const {
    if (!isValidIndex(index)) {
        return QVariant();
    }
    return model_->data(model_->index(index, 0), Qt::UserRole);
}

QString ModelComboBoxBase::itemText(int index) const {
    if (!isValidIndex(index)) {
        return QString();
    }
    QVariant data = model_->data(model_->index(index, 0), Qt::EditRole);
    return data.toString();
}

QIcon ModelComboBoxBase::itemIcon(int index) const {
    if (!isValidIndex(index)) {
        return QIcon();
    }
    QVariant data = model_->data(model_->index(index, 0), Qt::DecorationRole);
    return data.value<QIcon>();
}

void ModelComboBoxBase::setItemData(int index, const QVariant& value, int role) {
    if (isValidIndex(index)) {
        model_->setData(model_->index(index, 0), value, role);
    }
}

void ModelComboBoxBase::setItemText(int index, const QString& text) {
    if (!isValidIndex(index)) {
        return;
    }

    QString oldText = this->text();
    setItemData(index, text, Qt::EditRole);
    if (currentIndex_ == index) {
        setText(text);
        if (oldText != text) {
            // Emit currentTextChanged signal
        }
    }
}

void ModelComboBoxBase::setItemIcon(int index, const QIcon& icon) {
    setItemData(index, icon, Qt::DecorationRole);
}

int ModelComboBoxBase::findData(const QVariant& data, int role, Qt::MatchFlags flags) const {
    QModelIndex mi = model_->index(0, 0);
    QModelIndexList result = model_->match(mi, role, data, -1, flags | Qt::MatchRecursive);
    for (const QModelIndex& i : result) {
        return i.row();
    }
    return -1;
}

int ModelComboBoxBase::findText(const QString& text, Qt::MatchFlags flags) const {
    return findData(text, Qt::EditRole, flags);
}

void ModelComboBoxBase::clear() {
    if (currentIndex_ >= 0) {
        setText(QString());
    }

    model_->blockSignals(true);
    auto* standardModel = qobject_cast<QStandardItemModel*>(model_);
    if (standardModel) {
        standardModel->clear();
    }
    currentIndex_ = -1;
    model_->blockSignals(false);
}

int ModelComboBoxBase::count() const { return model_->rowCount(); }

void ModelComboBoxBase::setMaxVisibleItems(int num) { maxVisibleItems_ = num; }

void ModelComboBoxBase::setPlaceholderText(const QString& text) { placeholderText_ = text; }

bool ModelComboBoxBase::isValidIndex(int index) const { return 0 <= index && index < count(); }

void ModelComboBoxBase::closeComboMenu() {
    if (!dropMenu_) {
        return;
    }

    dropMenu_->close();
    dropMenu_ = nullptr;
}

void ModelComboBoxBase::onDropMenuClosed() { dropMenu_ = nullptr; }

ComboBoxMenu* ModelComboBoxBase::createComboMenu() { return new ComboBoxMenu(asWidget()); }

void ModelComboBoxBase::showComboMenu() {
    if (count() == 0) {
        return;
    }

    ComboBoxMenu* menu = createComboMenu();
    for (int i = 0; i < count(); ++i) {
        QAction* action = new QAction(itemIcon(i), itemText(i), menu);
        QObject::connect(action, &QAction::triggered, asWidget(),
                         [this, i]() { onItemClicked(i); });
        menu->addAction(action);
    }

    if (menu->view()->width() < width()) {
        menu->view()->setMinimumWidth(width());
        menu->adjustSize();
    }

    menu->setMaxVisibleItems(maxVisibleItems_);
    menu->setAttribute(Qt::WA_DeleteOnClose);
    QObject::connect(menu, &ComboBoxMenu::closedSignal, asWidget(),
                     [this]() { onDropMenuClosed(); });
    dropMenu_ = menu;

    // Set the selected item
    if (currentIndex_ >= 0 && currentIndex_ < menu->actions().size()) {
        if (auto* listWidget = qobject_cast<QListWidget*>(menu->view())) {
            if (auto* it = listWidget->item(currentIndex_)) {
                listWidget->setCurrentItem(it);
                it->setSelected(true);
            }
        }
    }

    // Determine the animation type by choosing the maximum height of view
    int x = -menu->width() / 2 + menu->layout()->contentsMargins().left() + width() / 2;
    QPoint pd = mapToGlobal(QPoint(x, height()));
    int hd = menu->view()->heightForAnimation(pd, MenuAnimationType::DropDown);

    QPoint pu = mapToGlobal(QPoint(x, 0));
    int hu = menu->view()->heightForAnimation(pu, MenuAnimationType::PullUp);

    if (hd >= hu) {
        menu->execAt(pd, true, MenuAnimationType::DropDown);
    } else {
        menu->execAt(pu, true, MenuAnimationType::PullUp);
    }

    // Ensure selection is visible and processed by delegate after menu is shown
    if (currentIndex_ >= 0) {
        if (auto* listWidget = qobject_cast<QListWidget*>(menu->view())) {
            if (auto* it = listWidget->item(currentIndex_)) {
                it->setSelected(true);
                listWidget->setCurrentItem(it);
                listWidget->scrollToItem(it);
            }
        }
    }
}

void ModelComboBoxBase::toggleComboMenu() {
    if (dropMenu_) {
        closeComboMenu();
    } else {
        showComboMenu();
    }
}

void ModelComboBoxBase::onItemClicked(int index) {
    if (index != currentIndex_) {
        setCurrentIndex(index);
    }
    // Emit activated signals
}

// ============================================================================
// ModelComboBox
// ============================================================================

ModelComboBox::ModelComboBox(QWidget* parent) : QPushButton(parent) {
    setProperty("qssClass", "ComboBox");
    setProperty("comboBox", true);
    setUpUi();
    qfw::setFont(this);
    installEventFilter(this);
}

void ModelComboBox::setIconVisible(bool visible) {
    if (visible == isIconVisible_ || currentIndex() < 0) {
        return;
    }

    isIconVisible_ = visible;

    if (visible) {
        updateIcon();
    } else {
        setIcon(QIcon());
    }
}

void ModelComboBox::setPlaceholderText(const QString& text) {
    placeholderText_ = text;

    if (currentIndex() < 0) {
        updateTextState(true);
        setText(text);
    }
}

void ModelComboBox::setCurrentIndex(int index) {
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
        updateIcon();
        return;
    }

    if (!isValidIndex(index)) {
        return;
    }

    updateTextState(false);
    currentIndex_ = index;
    setText(itemText(index));

    if (oldText != text()) {
        emit currentTextChanged(text());
    }
    emit currentIndexChanged(index);
    updateIcon();
}

void ModelComboBox::clear() {
    ModelComboBoxBase::clear();
    setCurrentIndex(-1);
}

void ModelComboBox::setItemIcon(int index, const QIcon& icon) {
    ModelComboBoxBase::setItemIcon(index, icon);
    if (index == currentIndex() && isIconVisible()) {
        setIcon(icon);
    }
}

void ModelComboBox::setText(const QString& text) {
    QPushButton::setText(text);
    adjustSize();
}

QString ModelComboBox::text() const { return QPushButton::text(); }

void ModelComboBox::adjustSize() { QPushButton::adjustSize(); }

QWidget* ModelComboBox::asWidget() { return this; }

const QWidget* ModelComboBox::asWidget() const { return this; }

QRect ModelComboBox::rect() const { return QPushButton::rect(); }

QPoint ModelComboBox::mapToGlobal(const QPoint& pos) const { return QPushButton::mapToGlobal(pos); }

QPoint ModelComboBox::mapFromGlobal(const QPoint& pos) const {
    return QPushButton::mapFromGlobal(pos);
}

int ModelComboBox::height() const { return QPushButton::height(); }

int ModelComboBox::width() const { return QPushButton::width(); }

bool ModelComboBox::eventFilter(QObject* obj, QEvent* e) {
    if (obj == this) {
        switch (e->type()) {
            case QEvent::MouseButtonPress:
                isPressed_ = true;
                update();
                break;
            case QEvent::MouseButtonRelease:
                isPressed_ = false;
                update();
                break;
            case QEvent::Enter:
                isHover_ = true;
                update();
                break;
            case QEvent::Leave:
                isHover_ = false;
                update();
                break;
            default:
                break;
        }
    }
    return QPushButton::eventFilter(obj, e);
}

void ModelComboBox::mouseReleaseEvent(QMouseEvent* event) {
    QPushButton::mouseReleaseEvent(event);
    toggleComboMenu();
}

void ModelComboBox::paintEvent(QPaintEvent* event) {
    QPushButton::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    if (isHover_) {
        painter.setOpacity(0.8);
    } else if (isPressed_) {
        painter.setOpacity(0.7);
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

void ModelComboBox::updateIcon() {
    if (!isIconVisible_) {
        return;
    }

    QIcon icon = itemIcon(currentIndex());
    if (!icon.isNull()) {
        setIcon(icon);
    } else {
        setIcon(QIcon());
    }
}

void ModelComboBox::updateTextState(bool isPlaceholder) {
    if (property("isPlaceholderText").toBool() == isPlaceholder) {
        return;
    }

    setProperty("isPlaceholderText", isPlaceholder);
    setStyle(QApplication::style());
}

void ModelComboBox::onItemClicked(int index) {
    if (!isValidIndex(index)) {
        return;
    }

    if (index != currentIndex()) {
        setCurrentIndex(index);
    }

    emit activated(index);
    emit textActivated(currentText());
}

// ============================================================================
// EditableModelComboBox
// ============================================================================

EditableModelComboBox::EditableModelComboBox(QWidget* parent) : LineEdit(parent) {
    setProperty("qssClass", "LineEdit");
    setUpUi();

    static FluentIcon arrowDown(FluentIconEnum::ArrowDown);
    dropButton_ = new LineEditButton(QVariant(arrowDown.qicon()), this);

    setTextMargins(0, 0, 29, 0);
    dropButton_->setFixedSize(30, 25);

    if (auto* layout = buttonLayout()) {
        layout->addWidget(dropButton_, 0, Qt::AlignRight);
    }

    connect(dropButton_, &QToolButton::clicked, this, [this]() { toggleComboMenu(); });
    connect(this, &LineEdit::textChanged, this, &EditableModelComboBox::onComboTextChanged);
    connect(this, &LineEdit::returnPressed, this, &EditableModelComboBox::onReturnPressed);

    qfw::setStyleSheet(this, FluentStyleSheet::LineEdit);

    // Find and reconnect clear button with custom behavior
    if (auto* clearBtn = findChild<QToolButton*>(QStringLiteral("lineEditButton"))) {
        clearBtn->disconnect();
        connect(clearBtn, &QToolButton::clicked, this,
                &EditableModelComboBox::onClearButtonClicked);
    }
}

QString EditableModelComboBox::currentText() const { return text(); }

void EditableModelComboBox::setCurrentIndex(int index) {
    if (index >= count() || index == currentIndex()) {
        return;
    }

    if (index < 0) {
        currentIndex_ = -1;
        setText("");
        setPlaceholderText(placeholderText_);
    } else {
        currentIndex_ = index;
        setText(itemText(index));
    }
}

void EditableModelComboBox::clear() { ModelComboBoxBase::clear(); }

void EditableModelComboBox::setPlaceholderText(const QString& text) {
    placeholderText_ = text;
    LineEdit::setPlaceholderText(text);
}

void EditableModelComboBox::onReturnPressed() {
    const QString txt = text();
    if (txt.isEmpty()) {
        return;
    }

    int index = findText(txt);
    if (index >= 0 && index != currentIndex()) {
        currentIndex_ = index;
        emit currentIndexChanged(index);
    } else if (index == -1) {
        addItem(txt);
        setCurrentIndex(count() - 1);
    }
}

void EditableModelComboBox::onComboTextChanged(const QString& text) {
    currentIndex_ = -1;
    emit currentTextChanged(text);

    int index = findText(text);
    if (index >= 0) {
        currentIndex_ = index;
        emit currentIndexChanged(index);
    }
}

void EditableModelComboBox::onClearButtonClicked() {
    LineEdit::clear();
    currentIndex_ = -1;
}

void EditableModelComboBox::onDropMenuClosed() { dropMenu_ = nullptr; }

void EditableModelComboBox::setText(const QString& text) { LineEdit::setText(text); }

QString EditableModelComboBox::text() const { return LineEdit::text(); }

void EditableModelComboBox::adjustSize() { LineEdit::adjustSize(); }

QWidget* EditableModelComboBox::asWidget() { return this; }

const QWidget* EditableModelComboBox::asWidget() const { return this; }

QRect EditableModelComboBox::rect() const { return LineEdit::rect(); }

QPoint EditableModelComboBox::mapToGlobal(const QPoint& pos) const {
    return LineEdit::mapToGlobal(pos);
}

QPoint EditableModelComboBox::mapFromGlobal(const QPoint& pos) const {
    return LineEdit::mapFromGlobal(pos);
}

int EditableModelComboBox::height() const { return LineEdit::height(); }

int EditableModelComboBox::width() const { return LineEdit::width(); }

}  // namespace qfw
