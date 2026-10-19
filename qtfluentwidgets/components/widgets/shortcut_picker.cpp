#include "components/widgets/shortcut_picker.h"

#include <QApplication>
#include <QKeyEvent>
#include <QPainter>
#include <QSignalBlocker>
#include <QStyleOptionFocusRect>
#include <algorithm>
#include <stdexcept>

#include "common/font.h"
#include "common/style_sheet.h"
#include "components/dialog_box/message_box_base.h"
#include "components/widgets/label.h"

namespace qfw {
namespace {
constexpr int allowedModifiers = Qt::ControlModifier | Qt::AltModifier | Qt::ShiftModifier |
                                 Qt::MetaModifier | Qt::KeypadModifier;
int combined(const QKeySequence& sequence) {
    if (sequence.isEmpty()) return 0;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return sequence[0].toCombined();
#else
    return sequence[0];
#endif
}
int modifierFlag(int key) {
    switch (key) {
        case Qt::Key_Control:
            return Qt::ControlModifier;
        case Qt::Key_Alt:
            return Qt::AltModifier;
        case Qt::Key_Shift:
            return Qt::ShiftModifier;
        case Qt::Key_Meta:
            return Qt::MetaModifier;
        default:
            return 0;
    }
}
QKeySequence validated(const QKeySequence& sequence) {
    if (sequence.count() > 1)
        throw std::invalid_argument("shortcut picker accepts a single key combination");
    const int chord = combined(sequence);
    const int key = chord & ~int(Qt::KeyboardModifierMask);
    if (!sequence.isEmpty() && (key == 0 || key == Qt::Key_unknown ||
                                (chord & int(Qt::KeyboardModifierMask) & ~allowedModifiers)))
        throw std::invalid_argument("invalid key combination");
    return sequence;
}
QStringList tokens(const QKeySequence& sequence) {
    if (sequence.isEmpty()) return {};
    const int chord = combined(sequence);
    const int key = chord & ~int(Qt::KeyboardModifierMask);
    QStringList result;
    for (const auto& entry : QList<QPair<int, QString>>{{Qt::Key_Control, "Ctrl"},
                                                        {Qt::Key_Alt, "Alt"},
                                                        {Qt::Key_Shift, "Shift"},
                                                        {Qt::Key_Meta, "Meta"}}) {
        if (key == entry.first || chord & modifierFlag(entry.first)) result.append(entry.second);
    }
    if (chord & Qt::KeypadModifier) result.append("Num");
    if (!modifierFlag(key)) result.append(QKeySequence(key).toString(QKeySequence::NativeText));
    return result;
}
class KeyCap : public QWidget {
public:
    KeyCap(const QString& text, bool large, QWidget* parent) : QWidget(parent), text_(text) {
        qfw::setFont(this, large ? 20 : 15);
        const int height = large ? 56 : 36;
        setFixedSize(std::max(height, fontMetrics().horizontalAdvance(text) + 24), height);
        setAccessibleName(text);
        connect(&QConfig::instance(), &QConfig::themeChanged, this, [this] { update(); });
        connect(&QConfig::instance(), &QConfig::themeColorChanged, this, [this] { update(); });
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        if (!isEnabled()) painter.setOpacity(0.4);
        painter.setPen(Qt::NoPen);
        painter.setBrush(themeColor());
        painter.drawRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), 5, 5);
        painter.setFont(font());
        painter.setPen(isDarkTheme() ? Qt::black : Qt::white);
        painter.drawText(rect(), Qt::AlignCenter, text_);
    }

private:
    QString text_;
};
}  // namespace
class KeyCaps : public QWidget {
public:
    KeyCaps(QWidget* parent, bool large = false) : QWidget(parent), large_(large) {
        row_ = new QHBoxLayout(this);
        row_->setContentsMargins(0, 0, 0, 0);
        row_->setSpacing(large ? 8 : 6);
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    }
    void setSequence(const QKeySequence& sequence) {
        while (auto* item = row_->takeAt(0)) {
            delete item->widget();
            delete item;
        }
        for (const auto& text : tokens(sequence)) row_->addWidget(new KeyCap(text, large_, this));
        setAccessibleName(sequence.isEmpty() ? ShortcutPicker::tr("Not set")
                                             : sequence.toString(QKeySequence::NativeText));
        updateGeometry();
    }

private:
    QHBoxLayout* row_;
    bool large_;
};

class ShortcutEditor : public MessageBoxBase {
public:
    ShortcutEditor(ShortcutPicker* picker, QWidget* owner)
        : MessageBoxBase(owner), picker_(picker), owner_(owner), draft_(picker->keySequence()) {
        setObjectName("shortcutEditor");
        // A picker used as a standalone window needs a separate editor. Inside
        // an application window the mask remains a child overlay, like MaskDialogBase.
        if (owner == picker) setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
        setWindowTitle(picker->dialogTitle());
        setWindowModality(Qt::WindowModal);
        auto* title = new SubtitleLabel(picker->dialogTitle(), widget);
        title->setWordWrap(true);
        auto* description = new BodyLabel(picker->dialogDescription(), widget);
        description->setWordWrap(true);
        caps_ = new KeyCaps(widget, true);
        caps_->setObjectName("shortcutDraft");
        empty_ = new BodyLabel(ShortcutPicker::tr("Press a key combination"), widget);
        empty_->setAlignment(Qt::AlignCenter);
        viewLayout->addWidget(title);
        viewLayout->addWidget(description);
        viewLayout->addStretch();
        viewLayout->addWidget(caps_, 0, Qt::AlignCenter);
        viewLayout->addWidget(empty_, 0, Qt::AlignCenter);
        viewLayout->addStretch();
        yesButton->setText(ShortcutPicker::tr("Save"));
        yesButton->setObjectName("shortcutSaveButton");
        cancelButton->setText(ShortcutPicker::tr("Cancel"));
        // Preserve MessageBoxBase's #cancelButton stylesheet selector.
        auto* reset = new PushButton(ShortcutPicker::tr("Reset"), buttonGroup);
        reset->setObjectName("shortcutResetButton");
        buttonLayout->insertWidget(1, reset, 1, Qt::AlignVCenter);
        for (auto* button : QList<QPushButton*>{yesButton, reset, cancelButton}) {
            button->setAutoDefault(false);
            button->setDefault(false);
        }
        connect(reset, &QPushButton::clicked, this, [this] {
            held_.clear();
            modifiers_ = 0;
            if (picker_) setDraft(picker_->defaultKeySequence());
        });
        fitToOwner();
        setDraft(draft_);
    }
    ~ShortcutEditor() override { stopCapture(); }
    QKeySequence draft() const { return draft_; }

protected:
    void showEvent(QShowEvent* event) override {
        MessageBoxBase::showEvent(event);
        fitToOwner();
        capturing_ = true;
        qApp->installEventFilter(this);
        setFocus(Qt::OtherFocusReason);
    }
    void done(int result) override {
        if (finishing_) return;
        finishing_ = true;
        stopCapture();
        for (auto* animation : findChildren<QPropertyAnimation*>()) animation->stop();
        MessageBoxBase::done(result);
    }
    void hideEvent(QHideEvent* event) override {
        stopCapture();
        MessageBoxBase::hideEvent(event);
    }
    bool eventFilter(QObject* watched, QEvent* event) override {
        if (capturing_) {
            if (event->type() == QEvent::ApplicationDeactivate) {
                held_.clear();
                modifiers_ = 0;
            }
            if (watched == owner_) {
                if (event->type() == QEvent::Resize || event->type() == QEvent::Move)
                    fitToOwner();
                else if (event->type() == QEvent::Hide || event->type() == QEvent::Close) {
                    reject();
                    return false;
                }
            }
            auto* target = qobject_cast<QWidget*>(watched);
            const bool inEditor = target && (target == this || isAncestorOf(target));
            if (inEditor &&
                (event->type() == QEvent::ShortcutOverride || event->type() == QEvent::KeyPress ||
                 event->type() == QEvent::KeyRelease)) {
                event->accept();
                auto* keyEvent = static_cast<QKeyEvent*>(event);
                if (event->type() != QEvent::ShortcutOverride && !keyEvent->isAutoRepeat())
                    record(keyEvent);
                return true;
            }
            if (event->type() == QEvent::Shortcut && isActiveWindow()) {
                event->accept();
                return true;
            }
        }
        return MessageBoxBase::eventFilter(watched, event);
    }

private:
    void stopCapture() {
        capturing_ = false;
        if (qApp) qApp->removeEventFilter(this);
        held_.clear();
        modifiers_ = 0;
    }
    void fitToOwner() {
        if (!owner_ || owner_ == picker_)
            resize(600, 440);
        else {
            resize(owner_->size());
            const QPoint global = owner_->mapToGlobal(QPoint());
            move(isWindow() || !parentWidget() ? global : parentWidget()->mapFromGlobal(global));
        }
        widget->setFixedSize(std::min(560, std::max(1, width() - 24)),
                             std::min(400, std::max(1, height() - 24)));
    }
    void setDraft(const QKeySequence& sequence) {
        draft_ = sequence;
        caps_->setSequence(sequence);
        caps_->setVisible(!sequence.isEmpty());
        empty_->setVisible(sequence.isEmpty());
        setProperty("draftKeySequence", QVariant::fromValue(sequence));
    }
    void record(QKeyEvent* event) {
        int key = event->key();
        int modifiers = int(event->modifiers());
        if (key == Qt::Key_Backtab) {
            key = Qt::Key_Tab;
            modifiers |= Qt::ShiftModifier;
        }
        if (key == 0 || key == Qt::Key_unknown) return;
        const int modifier = modifierFlag(key);
        if (event->type() == QEvent::KeyRelease) {
            if (modifier) modifiers_ &= ~modifier;
            held_.removeAll(key);
            return;
        }
        if (key == Qt::Key_Escape && picker_ && picker_->isEscapeCancelsCapture()) {
            reject();
            return;
        }
        modifiers_ = modifiers & allowedModifiers;
        if (modifier) {
            modifiers_ |= modifier;
            const int main = held_.isEmpty() ? key : held_.last();
            setDraft(QKeySequence(main | (held_.isEmpty() ? modifiers_ & ~modifier : modifiers_)));
        } else {
            held_.removeAll(key);
            held_.append(key);
            setDraft(QKeySequence(key | modifiers_));
        }
    }
    QPointer<ShortcutPicker> picker_;
    QPointer<QWidget> owner_;
    QKeySequence draft_;
    KeyCaps* caps_;
    BodyLabel* empty_;
    QList<int> held_;
    int modifiers_ = 0;
    bool capturing_ = false;
    bool finishing_ = false;
};

ShortcutPicker::ShortcutPicker(QWidget* parent)
    : ShortcutPicker(QStringLiteral("Ctrl+Shift+A"), parent) {}
ShortcutPicker::ShortcutPicker(const QString& sequence, QWidget* parent)
    : ShortcutPicker(QKeySequence::fromString(sequence, QKeySequence::PortableText), parent) {}
ShortcutPicker::ShortcutPicker(const QKeySequence& sequence, QWidget* parent)
    : CardWidget(parent),
      sequence_(validated(sequence)),
      default_(sequence_),
      title_(tr("Activate shortcut")),
      description_(tr("Press a key combination to change this shortcut")) {
    setClickEnabled(true);
    setBorderRadius(8);
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::StrongFocus);
    setFixedHeight(60);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    keyCaps = new KeyCaps(this);
    keyCaps->setAttribute(Qt::WA_TransparentForMouseEvents);
    editButton = new TransparentToolButton(FluentIcon(FluentIconEnum::Edit), this);
    editButton->setFixedSize(24, 36);
    editButton->setIconSize(QSize(16, 16));
    editButton->setAttribute(Qt::WA_TransparentForMouseEvents);
    editButton->setFocusPolicy(Qt::NoFocus);
    editButton->setAccessibleName(tr("Edit shortcut"));
    auto* row = new QHBoxLayout(this);
    row->setContentsMargins(12, 12, 12, 12);
    row->setSpacing(6);
    row->addWidget(keyCaps);
    row->addWidget(editButton);
    static_cast<KeyCaps*>(keyCaps)->setSequence(sequence_);
    updateAccessibleName();
    connect(this, &CardWidget::clicked, this, &ShortcutPicker::showEditor);
    connect(editButton, &QAbstractButton::clicked, this, &ShortcutPicker::showEditor);
}
ShortcutPicker::~ShortcutPicker() {
    if (editor_) {
        disconnect(editor_, nullptr, this, nullptr);
        delete editor_.data();
    }
}
QKeySequence ShortcutPicker::keySequence() const { return sequence_; }
void ShortcutPicker::setKeySequence(const QKeySequence& sequence) {
    validated(sequence);
    if (sequence == sequence_) return;
    sequence_ = sequence;
    static_cast<KeyCaps*>(keyCaps)->setSequence(sequence);
    updateAccessibleName();
    updateGeometry();
    emit keySequenceChanged(sequence);
}
void ShortcutPicker::setKeySequence(const QString& sequence) {
    setKeySequence(QKeySequence::fromString(sequence, QKeySequence::PortableText));
}
QKeySequence ShortcutPicker::defaultKeySequence() const { return default_; }
void ShortcutPicker::setDefaultKeySequence(const QKeySequence& sequence) {
    default_ = validated(sequence);
}
void ShortcutPicker::setDefaultKeySequence(const QString& sequence) {
    setDefaultKeySequence(QKeySequence::fromString(sequence, QKeySequence::PortableText));
}
void ShortcutPicker::reset() { setKeySequence(default_); }
void ShortcutPicker::clear() { setKeySequence(QKeySequence()); }
QString ShortcutPicker::dialogTitle() const { return title_; }
void ShortcutPicker::setDialogTitle(const QString& title) {
    title_ = title;
    customTitle_ = true;
}
QString ShortcutPicker::dialogDescription() const { return description_; }
void ShortcutPicker::setDialogDescription(const QString& description) {
    description_ = description;
    customDescription_ = true;
}
void ShortcutPicker::setEscapeCancelsCapture(bool enabled) { escapeCancels_ = enabled; }
bool ShortcutPicker::isEscapeCancelsCapture() const { return escapeCancels_; }
bool ShortcutPicker::isEditing() const { return !editor_.isNull(); }
void ShortcutPicker::showEditor() {
    if (!isEnabled() || !isVisible()) return;
    if (editor_) {
        editor_->raise();
        editor_->activateWindow();
        return;
    }
    auto* editor = new ShortcutEditor(this, window());
    editor_ = editor;
    editor->setAttribute(Qt::WA_DeleteOnClose);
    connect(editor, &QDialog::finished, this, [this, editor](int result) {
        editor_.clear();
        const QKeySequence selected = editor->draft();
        QPointer<ShortcutPicker> guard(this);
        if (result == QDialog::Accepted) {
            setKeySequence(selected);
            if (!guard) return;
            emit keySequenceSelected(selected);
            if (!guard) return;
            emit editingFinished();
        } else
            emit editingCanceled();
        if (guard && isEnabled() && isVisible()) setFocus(Qt::OtherFocusReason);
    });
    editor->open();
}
void ShortcutPicker::cancelEditing() {
    if (editor_) editor_->reject();
}
QSize ShortcutPicker::sizeHint() const { return layout()->sizeHint(); }
QSize ShortcutPicker::minimumSizeHint() const { return sizeHint(); }
void ShortcutPicker::updateAccessibleName() {
    setAccessibleName(
        tr("Shortcut") + QStringLiteral(": ") +
        (sequence_.isEmpty() ? tr("Not set") : sequence_.toString(QKeySequence::NativeText)));
}
void ShortcutPicker::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && isEnabled() && isClickEnabled()) {
        pressPending_ = true;
        setFocus(Qt::MouseFocusReason);
        CardWidget::mousePressEvent(event);
        event->accept();
    } else
        event->ignore();
}
void ShortcutPicker::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() != Qt::LeftButton) {
        event->ignore();
        return;
    }
    const bool activate =
        pressPending_ && isEnabled() && isClickEnabled() && rect().contains(event->pos());
    pressPending_ = false;
    if (activate)
        CardWidget::mouseReleaseEvent(event);
    else {
        isPressed_ = false;
        setBackgroundColor(isHover_ ? hoverBackgroundColor() : normalBackgroundColor());
    }
    event->accept();
}
void ShortcutPicker::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter ||
        event->key() == Qt::Key_Space) {
        if (!event->isAutoRepeat() && isEnabled() && isClickEnabled()) showEditor();
        event->accept();
    } else
        CardWidget::keyPressEvent(event);
}
void ShortcutPicker::hideEvent(QHideEvent* event) {
    pressPending_ = false;
    cancelEditing();
    CardWidget::hideEvent(event);
}
void ShortcutPicker::changeEvent(QEvent* event) {
    if (event->type() == QEvent::EnabledChange && !isEnabled()) {
        pressPending_ = false;
        cancelEditing();
    }
    if (event->type() == QEvent::LanguageChange) {
        if (!customTitle_) title_ = tr("Activate shortcut");
        if (!customDescription_)
            description_ = tr("Press a key combination to change this shortcut");
        editButton->setAccessibleName(tr("Edit shortcut"));
        updateAccessibleName();
    }
    CardWidget::changeEvent(event);
}
}  // namespace qfw
