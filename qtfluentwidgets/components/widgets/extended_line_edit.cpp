#include "components/widgets/extended_line_edit.h"

#include <QApplication>
#include <QClipboard>
#include <QKeyEvent>
#include <QPainter>
#include <QSignalBlocker>
#include <algorithm>
#include <stdexcept>

#include "common/color.h"
#include "common/style_sheet.h"
#include "components/widgets/button.h"

namespace qfw {
LabelLineEdit::LabelLineEdit(const QString& prefix, const QString& suffix, QWidget* parent)
    : LineEdit(parent) {
    setFixedHeight(40);
    prefix_ = new PushButton(prefix, this);
    suffix_ = new PushButton(suffix, this);
    prefix_->setFocusPolicy(Qt::NoFocus);
    suffix_->setFocusPolicy(Qt::NoFocus);
    prefix_->installEventFilter(this);
    suffix_->installEventFilter(this);
    buttonLayout()->insertWidget(0, prefix_, 0, Qt::AlignLeft);
    buttonLayout()->insertStretch(1, 1);
    buttonLayout()->addWidget(suffix_, 0, Qt::AlignRight);
    updateLabels();
}
QString LabelLineEdit::prefix() const { return prefix_->text(); }
QString LabelLineEdit::suffix() const { return suffix_->text(); }
void LabelLineEdit::setPrefix(const QString& prefix) {
    prefix_->setText(prefix);
    updateLabels();
}
void LabelLineEdit::setSuffix(const QString& suffix) {
    suffix_->setText(suffix);
    updateLabels();
}
PushButton* LabelLineEdit::prefixButton() const { return prefix_; }
PushButton* LabelLineEdit::suffixButton() const { return suffix_; }
void LabelLineEdit::updateLabels() {
    prefix_->setVisible(!prefix().isEmpty());
    suffix_->setVisible(!suffix().isEmpty());
    prefix_->setFixedSize(prefix_->sizeHint());
    suffix_->setFixedSize(suffix_->sizeHint());
    adjustTextMargins();
    updateGeometry();
}
void LabelLineEdit::adjustTextMargins() {
    LineEdit::adjustTextMargins();
    if (!prefix_ || !suffix_) {
        return;
    }
    QMargins margins = textMargins();
    margins.setLeft(margins.left() + (prefix().isEmpty() ? 0 : prefix_->width() + 7));
    margins.setRight(margins.right() + (suffix().isEmpty() ? 0 : suffix_->width() + 7));
    setTextMargins(margins);
}
bool LabelLineEdit::eventFilter(QObject* watched, QEvent* event) {
    if ((watched == prefix_ || watched == suffix_) &&
        (event->type() == QEvent::Resize || event->type() == QEvent::FontChange)) {
        adjustTextMargins();
    }
    return LineEdit::eventFilter(watched, event);
}
void LabelLineEdit::changeEvent(QEvent* event) {
    LineEdit::changeEvent(event);
    if (prefix_ && suffix_ &&
        (event->type() == QEvent::FontChange || event->type() == QEvent::StyleChange)) {
        updateLabels();
    }
}

PinBoxLineEdit::PinBoxLineEdit(QWidget* parent) : LineEdit(parent) {
    setProperty("pinEditor", true);
    qfw::updateDynamicStyle(this);
    setAlignment(Qt::AlignCenter);
    setMaxLength(1);
    updateCellSize();
}
void PinBoxLineEdit::setCellSize(const QSize& size) {
    if (size.width() <= 0 || size.height() <= 0)
        throw std::invalid_argument("PIN cell dimensions must be positive");
    cellSize_ = size;
    updateCellSize();
}
QSize PinBoxLineEdit::cellSize() const { return cellSize_; }
void PinBoxLineEdit::setFixedSize(int width, int height) { setCellSize(QSize(width, height)); }
void PinBoxLineEdit::setFixedSize(const QSize& size) { setCellSize(size); }
void PinBoxLineEdit::setFixedWidth(int width) { setCellSize(QSize(width, cellSize_.height())); }
void PinBoxLineEdit::setFixedHeight(int height) { setCellSize(QSize(cellSize_.width(), height)); }
void PinBoxLineEdit::updateCellSize() {
    const int inset = hasFocus() ? 2 : 0;
    QWidget::setFixedSize(std::max(1, cellSize_.width() - inset),
                          std::max(1, cellSize_.height() - inset));
}
void PinBoxLineEdit::focusInEvent(QFocusEvent* event) {
    LineEdit::focusInEvent(event);
    updateCellSize();
}
void PinBoxLineEdit::focusOutEvent(QFocusEvent* event) {
    LineEdit::focusOutEvent(event);
    updateCellSize();
}
void PinBoxLineEdit::setCustomFocusedBorderColor(const QColor& light, const QColor& dark) {
    lightBorderColor_ = light;
    darkBorderColor_ = dark;
    LineEdit::setCustomFocusedBorderColor(light, dark);
}
QColor PinBoxLineEdit::focusedBorderColor() const {
    if (isError()) return getFluentSystemColor(FluentSystemColor::CriticalForeground);
    return autoFallbackThemeColor(lightBorderColor_, darkBorderColor_);
}
void PinBoxLineEdit::paintEvent(QPaintEvent* event) {
    // Do not add the ordinary LineEdit's bottom-only accent to a PIN focus ring.
    QLineEdit::paintEvent(event);
    if (!hasFocus() || !isEnabled()) return;
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(focusedBorderColor(), 2));
    painter.drawRoundedRect(QRectF(rect()).adjusted(1, 1, -1, -1), 4, 4);
}

PinBox::PinBox(QWidget* parent) : QWidget(parent) {
    hBoxLayout = new QHBoxLayout(this);
    hBoxLayout->setContentsMargins(0, 0, 0, 0);
    hBoxLayout->setSpacing(10);
    hBoxLayout->setAlignment(Qt::AlignLeft);
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    setAccessibleName(tr("PIN"));
    setPinBoxCount(4);
}
int PinBox::count() const { return editors_.size(); }
PinBoxLineEdit* PinBox::editor(int index) const {
    if (index < 0 || index >= count()) {
        throw std::out_of_range("Invalid PIN index");
    }
    return editors_.at(index);
}
void PinBox::setPinBoxCount(int count) {
    if (count <= 0 || count > 128) {
        throw std::invalid_argument("PIN count must be between 1 and 128");
    }
    if (count == this->count()) {
        return;
    }
    changing_ = true;
    for (auto* edit : editors_) {
        disconnect(edit, nullptr, this, nullptr);
        edit->removeEventFilter(this);
        hBoxLayout->removeWidget(edit);
        edit->hide();
        edit->deleteLater();
    }
    editors_.clear();
    for (int i = 0; i < count; ++i) {
        auto* edit = new PinBoxLineEdit(this);
        edit->setCellSize(QSize(boxWidth_, boxHeight_));
        edit->setEchoMode(echoMode_);
        edit->setValidator(validator_);
        edit->setAccessibleName(tr("PIN character %1").arg(i + 1));
        edit->installEventFilter(this);
        editors_.append(edit);
        hBoxLayout->addWidget(edit);
        edit->show();
        connect(edit, &QLineEdit::textChanged, this, [this] { notifyTextChanged(); });
        connect(edit, &QLineEdit::textEdited, this, [this, edit] {
            const int index = editors_.indexOf(edit);
            if (!changing_ && !edit->text().isEmpty() && index >= 0 && index + 1 < this->count()) {
                focusEditor(index + 1);
            }
        });
    }
    for (int i = 1; i < count; ++i) {
        QWidget::setTabOrder(editors_.at(i - 1), editors_.at(i));
    }
    setFocusProxy(editors_.first());
    changing_ = false;
    updateGeometry();
    notifyTextChanged();
}
void PinBox::setPinBoxFixedWidth(int width) {
    if (width <= 0) {
        throw std::invalid_argument("PIN width must be positive");
    }
    boxWidth_ = width;
    for (auto* edit : editors_) {
        edit->setCellSize(QSize(boxWidth_, boxHeight_));
    }
    updateGeometry();
}
void PinBox::setPinBoxFixedHeight(int height) {
    if (height <= 0) {
        throw std::invalid_argument("PIN height must be positive");
    }
    boxHeight_ = height;
    for (auto* edit : editors_) {
        edit->setCellSize(QSize(boxWidth_, boxHeight_));
    }
    updateGeometry();
}
void PinBox::setEchoMode(QLineEdit::EchoMode mode) {
    echoMode_ = mode;
    for (auto* edit : editors_) {
        edit->setEchoMode(mode);
    }
}
QLineEdit::EchoMode PinBox::echoMode() const { return echoMode_; }
void PinBox::setPinBoxValidator(const QValidator* validator) {
    validator_ = validator;
    for (auto* edit : editors_) {
        edit->setValidator(validator);
    }
}
QStringList PinBox::texts() const {
    QStringList result;
    for (auto* edit : editors_) {
        result.append(edit->text());
    }
    return result;
}
QString PinBox::text() const { return texts().join(QString()); }
void PinBox::setText(const QString& text) {
    // Validate the entire assignment before mutating any editor.
    const QString truncated = text.left(count());
    if (validator_) {
        for (int i = 0; i < truncated.size(); ++i) {
            QString character(truncated.at(i));
            int position = 1;
            if (validator_->validate(character, position) == QValidator::Invalid) {
                return;
            }
        }
    }
    changing_ = true;
    for (int i = 0; i < count(); ++i) {
        const QSignalBlocker blocker(editors_.at(i));
        editors_.at(i)->setText(i < truncated.size() ? QString(truncated.at(i)) : QString());
    }
    changing_ = false;
    notifyTextChanged();
}
void PinBox::clear() { setText(QString()); }
void PinBox::notifyTextChanged() {
    if (changing_) {
        return;
    }
    const auto characters = texts();
    if (characters == lastText_) {
        return;
    }
    lastText_ = characters;
    emit textChanged(characters);
}
void PinBox::focusEditor(int index) {
    if (index >= 0 && index < count()) {
        editors_.at(index)->setFocus(Qt::OtherFocusReason);
        editors_.at(index)->selectAll();
    }
}
bool PinBox::pasteAt(int index, const QString& text) {
    if (index < 0 || text.isEmpty() || index >= count()) {
        return false;
    }
    const QString truncated = text.left(count() - index);
    for (int i = 0; i < truncated.size(); ++i) {
        if (editors_.at(index + i)->isReadOnly()) {
            return false;
        }
        if (validator_) {
            QString character(truncated.at(i));
            int position = 1;
            if (validator_->validate(character, position) == QValidator::Invalid) {
                return false;
            }
        }
    }
    changing_ = true;
    for (int i = 0; i < truncated.size(); ++i) {
        const QSignalBlocker blocker(editors_.at(index + i));
        editors_.at(index + i)->setText(QString(truncated.at(i)));
    }
    changing_ = false;
    notifyTextChanged();
    focusEditor(std::min(count() - 1, index + int(truncated.size())));
    return true;
}
bool PinBox::eventFilter(QObject* watched, QEvent* event) {
    auto* edit = qobject_cast<PinBoxLineEdit*>(watched);
    const int index = editors_.indexOf(edit);
    if (index >= 0 && event->type() == QEvent::KeyPress && isEnabled()) {
        auto* key = static_cast<QKeyEvent*>(event);
        if (key->matches(QKeySequence::Paste) && !edit->isReadOnly()) {
            pasteAt(index, QApplication::clipboard()->text());
            return true;
        }
        if (key->key() == Qt::Key_Backspace && !edit->isReadOnly()) {
            const bool empty = edit->text().isEmpty();
            edit->clear();
            if (index > 0) {
                if (empty) {
                    editors_.at(index - 1)->clear();
                }
                focusEditor(index - 1);
            }
            return true;
        }
        if (key->key() == Qt::Key_Left && index > 0) {
            focusEditor(index - 1);
            return true;
        }
        if (key->key() == Qt::Key_Right && index + 1 < count()) {
            focusEditor(index + 1);
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}
}  // namespace qfw
