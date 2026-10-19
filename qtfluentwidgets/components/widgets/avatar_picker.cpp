#include "components/widgets/avatar_picker.h"

#include <QFileDialog>
#include <QHideEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>

#include "common/icon.h"

namespace qfw {
AvatarPicker::AvatarPicker(QWidget* parent) : AvatarWidget(parent) {
    // C++ base constructors cannot dispatch Python's virtual _postInit().
    AvatarWidget::postInit();
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::StrongFocus);
    setAccessibleName(tr("Choose avatar"));
}
AvatarPicker::AvatarPicker(const QImage& image, QWidget* parent) : AvatarPicker(parent) {
    setImage(image);
}
AvatarPicker::AvatarPicker(const QPixmap& image, QWidget* parent) : AvatarPicker(parent) {
    setImage(image);
}
AvatarPicker::AvatarPicker(const QString& path, QWidget* parent) : AvatarPicker(parent) {
    setImage(path);
}
AvatarPicker::~AvatarPicker() { delete editor_.data(); }
ImageCropper* AvatarPicker::cropEditor() const { return editor_; }
QFileDialog* AvatarPicker::browseDialog() const { return dialog_; }
void AvatarPicker::chooseImage() {
    if (!isVisible() || !isEnabled()) return;
    if (editor_) {
        editor_->raise();
        return;
    }
    if (dialog_) {
        dialog_->raise();
        dialog_->activateWindow();
        return;
    }
    auto* dialog = new QFileDialog(this, tr("Choose avatar"));
    dialog_ = dialog;
    dialog->setFileMode(QFileDialog::ExistingFile);
    dialog->setAcceptMode(QFileDialog::AcceptOpen);
    dialog->setNameFilter(tr("Images (*.png *.jpg *.jpeg *.bmp *.webp)"));
    dialog->setWindowModality(Qt::WindowModal);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(dialog, &QDialog::finished, this, [this, dialog](int result) {
        const auto files = dialog->selectedFiles();
        if (dialog_ == dialog) dialog_.clear();
        if (result == QDialog::Accepted && isEnabled() && isVisible() && !files.isEmpty()) {
            const QImage image(files.first());
            if (!image.isNull()) cropImage(image);
        }
    });
    dialog->open();
}
ImageCropper* AvatarPicker::cropImage(const QImage& image) {
    if (!isEnabled() || image.isNull()) return nullptr;
    QPointer<AvatarPicker> guard(this);
    cancelSelection();
    if (!guard) return nullptr;
    auto* editor = new ImageCropper(image, window());
    editor_ = editor;
    editor->setCropShape(CropShape::CIRCLE);
    connect(editor, &ImageCropper::imageCropped, this, [this](const QImage& result) {
        if (!isEnabled()) return;
        setImage(result);
        emit imageChanged(result);
    });
    connect(editor, &QDialog::finished, this, [this, editor] {
        if (editor_ == editor) editor_.clear();
    });
    connect(editor, &QDialog::finished, editor, &QObject::deleteLater);
    connect(this, &QObject::destroyed, editor, &QObject::deleteLater);
    editor->show();  // Parent-scoped, asynchronous overlay; no nested event loop.
    return editor;
}
ImageCropper* AvatarPicker::cropImage(const QPixmap& image) { return cropImage(image.toImage()); }
ImageCropper* AvatarPicker::cropImage(const QString& path) { return cropImage(QImage(path)); }
void AvatarPicker::cancelSelection() {
    QPointer<AvatarPicker> guard(this);
    if (dialog_) dialog_->reject();
    if (!guard) return;
    if (editor_) editor_->reject();
}
void AvatarPicker::enterEvent(enterEvent_QEnterEvent* event) {
    hovered_ = true;
    update();
    AvatarWidget::enterEvent(event);
}
void AvatarPicker::leaveEvent(QEvent* event) {
    hovered_ = false;
    update();
    AvatarWidget::leaveEvent(event);
}
void AvatarPicker::mousePressEvent(QMouseEvent* event) {
    pressed_ = isEnabled() && event->button() == Qt::LeftButton;
    AvatarWidget::mousePressEvent(event);
}
void AvatarPicker::mouseReleaseEvent(QMouseEvent* event) {
    const bool activate =
        pressed_ && event->button() == Qt::LeftButton && rect().contains(event->pos());
    pressed_ = false;
    event->accept();
    if (activate && isEnabled()) {
        QPointer<AvatarPicker> guard(this);
        emit clicked();
        if (guard) chooseImage();
    }
}
void AvatarPicker::keyPressEvent(QKeyEvent* event) {
    if (isEnabled() && !event->isAutoRepeat() &&
        (event->key() == Qt::Key_Space || event->key() == Qt::Key_Return ||
         event->key() == Qt::Key_Enter)) {
        event->accept();
        chooseImage();
    } else
        AvatarWidget::keyPressEvent(event);
}
void AvatarPicker::hideEvent(QHideEvent* event) {
    hovered_ = pressed_ = false;
    AvatarWidget::hideEvent(event);
    cancelSelection();
}
void AvatarPicker::changeEvent(QEvent* event) {
    AvatarWidget::changeEvent(event);
    if (event->type() == QEvent::EnabledChange && !isEnabled()) {
        hovered_ = pressed_ = false;
        update();
        cancelSelection();
    }
}
void AvatarPicker::paintEvent(QPaintEvent* event) {
    AvatarWidget::paintEvent(event);
    if (!hovered_ || !isEnabled()) return;
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 155));
    painter.drawEllipse(QRectF(rect()));
    const int size = qMin(28, qRound(width() * .3));
    FluentIcon(FluentIconEnum::Camera)
        .render(&painter, QRect((width() - size) / 2, (height() - size) / 2, size, size),
                Theme::Dark);
}
}  // namespace qfw
