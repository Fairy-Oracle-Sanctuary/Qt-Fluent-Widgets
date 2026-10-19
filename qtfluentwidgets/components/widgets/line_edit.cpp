#include "components/widgets/line_edit.h"

#include <QAbstractItemModel>
#include <QAction>
#include <QCompleter>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QStyle>
#include <QTimer>

#include "common/color.h"
#include "common/config.h"
#include "common/font.h"
#include "common/icon.h"
#include "common/style_sheet.h"
#include "components/widgets/menu.h"
#include "components/widgets/scroll_bar.h"
#include "components/widgets/tool_tip.h"

namespace qfw {

static void drawIconVariant(const QVariant& icon, QPainter* painter, const QRect& rect,
                            const QColor& fill = QColor()) {
    if (!painter) {
        return;
    }

    if (!icon.isValid()) {
        return;
    }

    if (icon.canConvert<QIcon>()) {
        const QIcon qicon = icon.value<QIcon>();
        if (!qicon.isNull()) {
            qicon.paint(painter, rect, Qt::AlignCenter, QIcon::Normal, QIcon::Off);
        }
        return;
    }

    if (icon.canConvert<const FluentIconBase*>()) {
        const auto* fluentIcon = icon.value<const FluentIconBase*>();
        if (!fluentIcon) {
            return;
        }

        if (fill.isValid()) {
            fluentIcon->render(painter, rect, Theme::Auto,
                               QVariantMap{{QStringLiteral("fill"), fill.name()},
                                           {QStringLiteral("stroke"), fill.name()}});
        } else {
            fluentIcon->render(painter, rect, Theme::Auto);
        }
        return;
    }
}

// ============================================================================
// LineEditButton
// ============================================================================

LineEditButton::LineEditButton(const QVariant& icon, QWidget* parent)
    : QToolButton(parent), icon_(icon) {
    isPressed_ = false;
    setFixedSize(31, 23);
    setIconSize(QSize(10, 10));
    setCursor(Qt::PointingHandCursor);
    setObjectName(QStringLiteral("lineEditButton"));

    qfw::setStyleSheet(this, qfw::FluentStyleSheet::LineEdit);
}

void LineEditButton::setAction(QAction* action) {
    action_ = action;
    onActionChanged();

    if (!action_) {
        return;
    }

    connect(this, &QToolButton::clicked, action_, &QAction::trigger);
    connect(action_, &QAction::toggled, this, &QToolButton::setChecked);
    connect(action_, &QAction::changed, this, &LineEditButton::onActionChanged);

    installEventFilter(new ToolTipFilter(this, 700));
}

QAction* LineEditButton::action() const { return action_; }

void LineEditButton::onActionChanged() {
    if (!action_) {
        return;
    }

    setIcon(QVariant(action_->icon()));
    setToolTip(action_->toolTip());
    setEnabled(action_->isEnabled());
    setCheckable(action_->isCheckable());
    setChecked(action_->isChecked());
}

void LineEditButton::setIcon(const QVariant& icon) {
    icon_ = icon;
    update();
}

QVariant LineEditButton::icon() const { return icon_; }

void LineEditButton::mousePressEvent(QMouseEvent* e) {
    isPressed_ = true;
    QToolButton::mousePressEvent(e);
}

void LineEditButton::mouseReleaseEvent(QMouseEvent* e) {
    isPressed_ = false;
    QToolButton::mouseReleaseEvent(e);
}

void LineEditButton::paintEvent(QPaintEvent* e) {
    QToolButton::paintEvent(e);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    const int iw = iconSize().width();
    const int ih = iconSize().height();
    const int w = width();
    const int h = height();

    QRect rect((w - iw) / 2, (h - ih) / 2, iw, ih);

    if (isPressed_) {
        painter.setOpacity(0.7);
    }

    if (qfw::isDarkTheme()) {
        drawIconVariant(icon_, &painter, rect);
    } else {
        drawIconVariant(icon_, &painter, rect, QColor(QStringLiteral("#656565")));
    }
}

// ============================================================================
// CompleterMenu
// ============================================================================

CompleterMenu::CompleterMenu(LineEdit* lineEdit)
    : RoundMenu(QString(), lineEdit), lineEdit_(lineEdit) {
    view()->setViewportMargins(0, 2, 0, 6);
    view()->setObjectName(QStringLiteral("completerListWidget"));
    view()->setItemDelegate(new IndicatorMenuItemDelegate(this));
    view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    installEventFilter(this);
    setItemHeight(33);

    connect(view(), &QListWidget::itemClicked, this, &CompleterMenu::onItemClicked);
}

bool CompleterMenu::setCompletion(QAbstractItemModel* model, int column) {
    QStringList items;
    indexes_.clear();

    for (int i = 0; i < model->rowCount(); ++i) {
        QModelIndex idx = model->index(i, column);
        items.append(model->data(idx).toString());
        indexes_.append(idx);
    }

    if (items_ == items && isVisible()) {
        return false;
    }

    setItems(items);
    return true;
}

void CompleterMenu::setItems(const QStringList& items) {
    view()->clear();
    items_ = items;

    for (const QString& item : items) {
        auto* listItem = new QListWidgetItem(item);
        listItem->setSizeHint(QSize(1, 33));
        view()->addItem(listItem);
    }
}

void CompleterMenu::setMaxVisibleItems(int num) { RoundMenu::setMaxVisibleItems(num); }

void CompleterMenu::popup() {
    if (items_.isEmpty()) {
        close();
        return;
    }

    // Adjust menu size
    if (view()->width() < lineEdit_->width()) {
        view()->setMinimumWidth(lineEdit_->width());
        adjustSize();
    }

    // Determine animation type
    QMargins margins = layout()->contentsMargins();
    int x = -width() / 2 + margins.left() + lineEdit_->width() / 2;
    int y = lineEdit_->height() - margins.top() + 2;
    QPoint pd = lineEdit_->mapToGlobal(QPoint(x, y));
    int hd = view()->heightForAnimation(pd, MenuAnimationType::FadeInDropDown);

    QPoint pu = lineEdit_->mapToGlobal(QPoint(x, 7));
    int hu = view()->heightForAnimation(pu, MenuAnimationType::FadeInPullUp);

    QPoint pos;
    MenuAnimationType aniType;

    if (hd >= hu) {
        pos = pd;
        aniType = MenuAnimationType::FadeInDropDown;
    } else {
        pos = pu;
        aniType = MenuAnimationType::FadeInPullUp;
    }

    view()->adjustSizeForMenu(pos, aniType);

    // Update border style
    view()->setProperty("dropDown", aniType == MenuAnimationType::FadeInDropDown);
    updateDynamicStyle(view());

    adjustSize();
    execAt(pos, true, aniType);

    // Remove focus from menu
    view()->setFocusPolicy(Qt::NoFocus);
    setFocusPolicy(Qt::NoFocus);
    lineEdit_->setFocus();
}

void CompleterMenu::onItemClicked(QListWidgetItem* item) {
    close();
    onCompletionItemSelected(item->text(), view()->row(item)-completionRowOffset());
}

bool CompleterMenu::eventFilter(QObject* obj, QEvent* event) {
    if (event->type() != QEvent::KeyPress) {
        return RoundMenu::eventFilter(obj, event);
    }

    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);

    // Redirect input to line edit
    QKeyEvent keyPress(QEvent::KeyPress, keyEvent->key(), keyEvent->modifiers(), keyEvent->text(),
                       keyEvent->isAutoRepeat(), keyEvent->count());
    qApp->sendEvent(lineEdit_, &keyPress);

    // Do not synchronously resend the original event to view(). An unhandled key event from the
    // list propagates back to this popup, re-enters this filter, and overflows the stack while a
    // completer is visible (notably when repeatedly pressing Backspace).
    switch (keyEvent->key()) {
        case Qt::Key_Up:
        case Qt::Key_Down: {
            const int count = view()->count();
            if (count > 0) {
                int row = view()->currentRow();
                row = keyEvent->key() == Qt::Key_Up ? row - 1 : row + 1;
                const int first=completionRowOffset(),last=count-1-first;
                if(last>=first) view()->setCurrentRow(qBound(first,row,last));
            }
            break;
        }
        case Qt::Key_Escape:
            close();
            break;
        case Qt::Key_Enter:
        case Qt::Key_Return:
            if (view()->currentRow() >= 0) {
                if (QListWidgetItem* currentItem = view()->currentItem()) {
                    if(currentItem->flags() & Qt::ItemIsEnabled)
                        onCompletionItemSelected(currentItem->text(), view()->currentRow()-completionRowOffset());
                }
            }
            close();
            break;
        default:
            break;
    }

    return true;
}

void CompleterMenu::onCompletionItemSelected(const QString& text, int row) {
    lineEdit_->setText(text);
    emit activated(text);

    if (row >= 0 && row < indexes_.size()) {
        emit indexActivated(indexes_[row]);
    }
}

// ============================================================================
// LineEdit
// ============================================================================

LineEdit::LineEdit(QWidget* parent) : QLineEdit(parent) {
    setProperty("transparent", true);
    setProperty("qssClass", "LineEdit");

    qfw::setStyleSheet(this, qfw::FluentStyleSheet::LineEdit);
    qfw::updateDynamicStyle(this);

    setFixedHeight(33);
    setAttribute(Qt::WA_MacShowFocusRect, false);
    qfw::setFont(this);

    hBoxLayout_ = new QHBoxLayout(this);

    static qfw::FluentIcon closeIcon(qfw::FluentIconEnum::Close);
    clearButton_ = new LineEditButton(QVariant(closeIcon.qicon()), this);

    clearButton_->setFixedSize(29, 25);
    clearButton_->hide();

    hBoxLayout_->setSpacing(3);
    hBoxLayout_->setContentsMargins(4, 4, 4, 4);
    hBoxLayout_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    hBoxLayout_->addWidget(clearButton_, 0, Qt::AlignRight);

    connect(clearButton_, &QToolButton::clicked, this, &QLineEdit::clear);
    connect(this, &QLineEdit::textChanged, this, &LineEdit::onTextChanged);
    connect(this, &QLineEdit::textEdited, this, &LineEdit::onTextEdited);
}

QHBoxLayout* LineEdit::buttonLayout() const { return hBoxLayout_; }

LineEditButton* LineEdit::clearButton() const { return clearButton_; }

bool LineEdit::isError() const { return isError_; }

void LineEdit::setError(bool isError) {
    if (isError == isError_) {
        return;
    }

    isError_ = isError;
    update();
}

void LineEdit::setCustomFocusedBorderColor(const QColor& light, const QColor& dark) {
    lightFocusedBorderColor_ = QColor(light);
    darkFocusedBorderColor_ = QColor(dark);
    update();
}

QColor LineEdit::focusedBorderColor() const {
    if (isError()) {
        return qfw::getFluentSystemColor(qfw::FluentSystemColor::CriticalForeground);
    }

    return qfw::autoFallbackThemeColor(lightFocusedBorderColor_, darkFocusedBorderColor_);
}

void LineEdit::setClearButtonEnabled(bool enable) {
    isClearButtonEnabled_ = enable;
    adjustTextMargins();
}

bool LineEdit::isClearButtonEnabled() const { return isClearButtonEnabled_; }

void LineEdit::setCompleter(QCompleter* completer) { completer_ = completer; }

QCompleter* LineEdit::completer() const { return completer_; }

void LineEdit::setCompleterMenu(CompleterMenu* menu) {
    if(completerMenu_==menu) return;
    delete completerMenu_;
    completerMenu_ = menu;
    if(!menu) return;
    connect(menu, &CompleterMenu::activated, this, [this](const QString& text) {
        if (completer_) {
            emit completer_->activated(text);
        }
    });
    connect(menu, &CompleterMenu::indexActivated, this, [this](const QModelIndex& index) {
        if (completer_) {
            emit completer_->activated(index);
        }
    });
}

void LineEdit::showCompleterMenu() {
    if (!completer_ || text().isEmpty()) {
        return;
    }

    // Create menu if not exists
    if (!completerMenu_) {
        setCompleterMenu(new CompleterMenu(this));
    }

    // Add menu items
    completer_->setCompletionPrefix(text());
    bool changed = completerMenu_->setCompletion(completer_->completionModel(),
                                                 completer_->completionColumn());
    completerMenu_->setMaxVisibleItems(completer_->maxVisibleItems());

    // Show menu
    if (changed) {
        completerMenu_->popup();
    }
}

void LineEdit::onTextEdited(const QString& text) {
    if (!completer_) {
        return;
    }

    if (!text.isEmpty()) {
        QTimer::singleShot(50, this, &LineEdit::showCompleterMenu);
    } else if (completerMenu_) {
        completerMenu_->close();
    }
}

void LineEdit::addAction(QAction* action, QLineEdit::ActionPosition position) {
    if (!action) {
        return;
    }

    QWidget::addAction(action);

    auto* button = new LineEditButton(QVariant(action->icon()), this);
    button->setAction(action);
    button->setFixedWidth(29);

    if (position == QLineEdit::LeadingPosition) {
        hBoxLayout_->insertWidget(leftButtons_.size(), button, 0, Qt::AlignLeading);
        if (leftButtons_.isEmpty()) {
            hBoxLayout_->insertStretch(1, 1);
        }
        leftButtons_.append(button);
    } else {
        rightButtons_.append(button);
        hBoxLayout_->addWidget(button, 0, Qt::AlignRight);
    }

    adjustTextMargins();
}

void LineEdit::addActions(const QList<QAction*>& actions, QLineEdit::ActionPosition position) {
    for (auto* action : actions) {
        addAction(action, position);
    }
}

void LineEdit::adjustTextMargins() {
    const int left = leftButtons_.size() * 30;
    const int right = rightButtons_.size() * 30 + (isClearButtonEnabled() ? 28 : 0);

    const QMargins m = textMargins();
    setTextMargins(left, m.top(), right, m.bottom());
}

void LineEdit::focusOutEvent(QFocusEvent* e) {
    editingCursorPosition_ = cursorPosition();
    QLineEdit::focusOutEvent(e);
    if (clearButton_) {
        clearButton_->hide();
    }

    // An unfocused field is a value preview, so show the beginning of a long
    // value instead of the tail that QLineEdit keeps visible for its cursor.
    if (!text().isEmpty()) {
        setCursorPosition(0);
        isShowingTextStart_ = true;
    } else {
        isShowingTextStart_ = false;
    }
}

void LineEdit::focusInEvent(QFocusEvent* e) {
    if (isShowingTextStart_) {
        const int textLength = static_cast<int>(text().size());
        setCursorPosition(qBound(0, editingCursorPosition_, textLength));
        isShowingTextStart_ = false;
    }

    QLineEdit::focusInEvent(e);

    if (!clearButton_) {
        return;
    }

    if (isClearButtonEnabled()) {
        clearButton_->setVisible(!text().isEmpty());
    }
}

void LineEdit::onTextChanged(const QString& text) {
    if (!hasFocus()) {
        // QLineEdit::setText() positions the cursor at the end.  Preserve that
        // as the editing position but use position zero for the idle preview.
        editingCursorPosition_ = static_cast<int>(text.size());
        setCursorPosition(0);
        isShowingTextStart_ = !text.isEmpty();
    } else {
        isShowingTextStart_ = false;
    }

    if (!clearButton_) {
        return;
    }

    if (isClearButtonEnabled()) {
        clearButton_->setVisible(!text.isEmpty() && hasFocus());
    }
}

void LineEdit::paintEvent(QPaintEvent* e) {
    QLineEdit::paintEvent(e);

    if (!hasFocus()) {
        return;
    }

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    const QMargins m = contentsMargins();

    QPainterPath path;
    const int w = width() - m.left() - m.right();
    const int h = height();

    path.addRoundedRect(QRectF(m.left(), h - 10, w, 10), 5, 5);

    QPainterPath rectPath;
    rectPath.addRect(m.left(), h - 10, w, 8);

    path = path.subtracted(rectPath);

    painter.fillPath(path, focusedBorderColor());
}

// ============================================================================
// PlainTextEdit
// ============================================================================
EditLayer::EditLayer(QWidget* parent) : QWidget(parent) {
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_NoSystemBackground);
    resize(parent->size());
    parent->installEventFilter(this);
    connect(&QConfig::instance(), &QConfig::themeChangedFinished, this, [this] { update(); });
}

bool EditLayer::eventFilter(QObject* watched, QEvent* event) {
    if (watched == parentWidget()) {
        if (event->type() == QEvent::Resize) {
            resize(parentWidget()->size());
            raise();
        }
        if (event->type() == QEvent::FocusIn || event->type() == QEvent::FocusOut ||
            event->type() == QEvent::EnabledChange)
            update();
    }
    return QWidget::eventFilter(watched, event);
}

void EditLayer::paintEvent(QPaintEvent*) {
    if (!parentWidget()->hasFocus() || !parentWidget()->isEnabled()) return;
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QPainterPath bottom, cut;
    bottom.addRoundedRect(QRectF(0, height() - 10, width(), 10), 5, 5);
    cut.addRect(0, height() - 10, width(), 7.5);
    painter.fillPath(bottom.subtracted(cut), qfw::themeColor());
}

TextEdit::TextEdit(QWidget* parent) : QTextEdit(parent) {
    setProperty("qssClass", "TextEdit");
    qfw::setStyleSheet(this, qfw::FluentStyleSheet::LineEdit);
    qfw::updateDynamicStyle(this);
    qfw::setFont(this);
    scrollDelegate_ = new SmoothScrollDelegate(this);
    new EditLayer(this);
}

namespace {
class TransientTextEditMenu : public TextEditMenu {
public:
    explicit TransientTextEditMenu(QTextEdit* editor) : TextEditMenu(editor) {}
protected:
    void hideEvent(QHideEvent* event) override {
        TextEditMenu::hideEvent(event);
        deleteLater();
    }
};
}

void TextEdit::contextMenuEvent(QContextMenuEvent* event) {
    if (!event) return;
    auto* menu = new TransientTextEditMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->execAt(event->globalPos(), true, MenuAnimationType::DropDown);
}

PlainTextEdit::PlainTextEdit(QWidget* parent) : QPlainTextEdit(parent) {
    new EditLayer(this);
    setProperty("qssClass", "PlainTextEdit");
    qfw::setStyleSheet(this, qfw::FluentStyleSheet::LineEdit);
    qfw::updateDynamicStyle(this);
    qfw::setFont(this);

    scrollDelegate_ = new SmoothScrollDelegate(this);
}

void PlainTextEdit::contextMenuEvent(QContextMenuEvent* e) {
    if (!e) {
        return;
    }

    auto* menu = new qfw::PlainTextEditMenu(this);
    qfw::setStyleSheet(menu, qfw::FluentStyleSheet::Menu);
    menu->execAt(e->globalPos(), true, qfw::MenuAnimationType::DropDown);
}

// ============================================================================
// TextBrowser
// ============================================================================
TextBrowser::TextBrowser(QWidget* parent) : QTextBrowser(parent) {
    new EditLayer(this);
    setProperty("qssClass", "TextBrowser");
    qfw::setStyleSheet(this, qfw::FluentStyleSheet::LineEdit);
    qfw::updateDynamicStyle(this);
    qfw::setFont(this);

    scrollDelegate_ = new SmoothScrollDelegate(this);
}

void TextBrowser::contextMenuEvent(QContextMenuEvent* e) {
    if (!e) {
        return;
    }

    auto* menu = new qfw::TextEditMenu(this);
    qfw::setStyleSheet(menu, qfw::FluentStyleSheet::Menu);
    menu->execAt(e->globalPos(), true, qfw::MenuAnimationType::DropDown);
}

// ============================================================================
// PasswordLineEdit
// ============================================================================
PasswordLineEdit::PasswordLineEdit(QWidget* parent) : LineEdit(parent) {
    setEchoMode(QLineEdit::Password);
    setContextMenuPolicy(Qt::NoContextMenu);

    static qfw::FluentIcon viewIcon(qfw::FluentIconEnum::View);
    viewButton_ = new LineEditButton(QVariant(viewIcon.qicon()), this);
    viewButton_->setIconSize(QSize(13, 13));
    viewButton_->setFixedSize(29, 25);

    if (buttonLayout()) {
        buttonLayout()->addWidget(viewButton_, 0, Qt::AlignRight);
    }

    setClearButtonEnabled(false);
    setPasswordVisible(false);

    viewButton_->installEventFilter(this);
}

void PasswordLineEdit::setPasswordVisible(bool visible) {
    if (isPasswordVisible() == visible) {
        return;
    }

    setEchoMode(visible ? QLineEdit::Normal : QLineEdit::Password);
    updateViewButtonIcon();
}

bool PasswordLineEdit::isPasswordVisible() const { return echoMode() == QLineEdit::Normal; }

void PasswordLineEdit::setViewPasswordButtonVisible(bool visible) {
    if (!viewButton_) return;
    viewButton_->setVisible(visible);
    if (!visible) setPasswordVisible(false);
    setClearButtonEnabled(isClearButtonEnabled());
}
void PasswordLineEdit::setClearButtonEnabled(bool enabled) {
    LineEdit::setClearButtonEnabled(enabled);
    setTextMargins(0, 0, 28 * int(enabled) + (viewButton_ && !viewButton_->isHidden() ? 30 : 0), 0);
}
QVariant PasswordLineEdit::inputMethodQuery(Qt::InputMethodQuery query) const {
    return query == Qt::ImEnabled ? QVariant(false) : LineEdit::inputMethodQuery(query);
}
bool PasswordLineEdit::eventFilter(QObject* object, QEvent* event) {
    if (object == viewButton_) {
        if (isEnabled() && event->type() == QEvent::MouseButtonPress)
            setPasswordVisible(true);
        else if (event->type() == QEvent::MouseButtonRelease || event->type() == QEvent::Hide ||
                 event->type() == QEvent::UngrabMouse || event->type() == QEvent::FocusOut)
            setPasswordVisible(false);
    }
    return LineEdit::eventFilter(object, event);
}
void PasswordLineEdit::changeEvent(QEvent* event) {
    if (event->type() == QEvent::EnabledChange && !isEnabled()) setPasswordVisible(false);
    LineEdit::changeEvent(event);
}
void PasswordLineEdit::hideEvent(QHideEvent* event) {
    setPasswordVisible(false);
    LineEdit::hideEvent(event);
}

void PasswordLineEdit::updateViewButtonIcon() {
    if (!viewButton_) {
        return;
    }

    static qfw::FluentIcon viewIcon(qfw::FluentIconEnum::View);
    viewButton_->setIcon(QVariant(viewIcon.qicon()));
}

// ============================================================================
// SearchLineEdit
// ============================================================================
SearchLineEdit::SearchLineEdit(QWidget* parent) : LineEdit(parent) {
    static qfw::FluentIcon searchIcon(qfw::FluentIconEnum::Search);
    searchButton_ = new LineEditButton(QVariant(searchIcon.qicon()), this);

    if (buttonLayout()) {
        buttonLayout()->addWidget(searchButton_, 0, Qt::AlignRight);
    }

    setClearButtonEnabled(true);
    setTextMargins(0, 0, 59, 0);

    connect(searchButton_, &QToolButton::clicked, this, &SearchLineEdit::search);
    connect(clearButton(), &QToolButton::clicked, this, &SearchLineEdit::clearSignal);
}

void SearchLineEdit::search() {
    const QString text = this->text().trimmed();
    if (!text.isEmpty()) {
        emit searchSignal(text);
    } else {
        emit clearSignal();
    }
}

void SearchLineEdit::setClearButtonEnabled(bool enable) {
    isClearButtonEnabled_ = enable;
    setTextMargins(0, 0, 28 * enable + 30, 0);
}

}  // namespace qfw
