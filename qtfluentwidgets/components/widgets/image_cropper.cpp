#include "components/widgets/image_cropper.h"

#include <QApplication>
#include <QHideEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QShowEvent>
#include <QSvgRenderer>
#include <QTransform>
#include <cmath>
#include <stdexcept>

#include "common/icon.h"
#include "common/style_sheet.h"

namespace qfw {
namespace {
const QColor cropMask(0, 0, 0, 170);
class CropWindowMask : public QWidget {
public:
    explicit CropWindowMask(QWidget* parent) : QWidget(parent) {}

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);
        painter.fillRect(rect(), cropMask);
    }
};
// The five compact tools use a permanently dark toolbar, independent of page theme.
class CropToolButton : public QPushButton {
public:
    CropToolButton(int symbol, const QString& label, QWidget* parent)
        : QPushButton(parent), symbol_(symbol) {
        setFixedSize(46, 40);
        setAccessibleName(label);
        setToolTip(label);
        setCursor(Qt::PointingHandCursor);
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        if (underMouse() && isEnabled()) {
            painter.save();
            painter.translate(2, 2);
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(255, 255, 255, isDown() ? 12 : 18));
            if (symbol_ == 0 || symbol_ == 4) {
                const qreal w = width() - 6, h = height() - 4;
                if (symbol_ == 4) painter.translate(2, 0);
                qreal left = symbol_ == 0 ? 16 : 4, right = symbol_ == 0 ? 4 : 16;
                if (isRightToLeft()) {
                    painter.translate(symbol_ == 0 ? 2 : -2, 0);
                    qSwap(left, right);
                }
                QPainterPath path;
                path.moveTo(left, 0);
                path.lineTo(w - right, 0);
                path.quadTo(w, 0, w, right);
                path.lineTo(w, h - right);
                path.quadTo(w, h, w - right, h);
                path.lineTo(left, h);
                path.quadTo(0, h, 0, h - left);
                path.lineTo(0, left);
                path.quadTo(0, 0, left, 0);
                painter.drawPath(path);
            } else {
                painter.drawRoundedRect(QRectF(0, 0, width() - 4, height() - 4), 6, 6);
            }
            painter.restore();
        }
        painter.setPen(QPen(QColor(255, 255, 255, isEnabled() ? 255 : 80), 1.5));
        painter.setBrush(Qt::NoBrush);
        painter.translate(width() / 2.0, height() / 2.0);
        if (symbol_ == 1) {
            if (!isEnabled()) painter.setOpacity(80.0 / 255);
            // Match Python's direct SVG rendering, including fractional alpha and scaling.
            QSvgRenderer renderer(FluentIcon(FluentIconEnum::Rotate).path(Theme::Dark));
            renderer.render(&painter, QRectF(-7, -7, 14, 14));
            return;
        }
        painter.scale(0.72, 0.72);
        if (symbol_ == 0) {
            painter.drawLine(-7, -7, 7, 7);
            painter.drawLine(-7, 7, 7, -7);
        } else if (symbol_ == 4) {
            painter.drawLine(-9, 0, -3, 6);
            painter.drawLine(-3, 6, 10, -7);
        } else if (symbol_ == 2) {
            painter.drawLine(0, -9, 0, 9);
            painter.drawLine(3, -8, 10, 8);
            painter.drawLine(10, 8, 3, 8);
            painter.drawLine(3, 8, 3, -8);
            QPainterPath path;
            path.moveTo(-3, -8);
            path.lineTo(-10, 8);
            path.lineTo(-3, 8);
            path.closeSubpath();
            painter.fillPath(path, Qt::white);
        } else {
            auto pen = painter.pen();
            pen.setStyle(Qt::DotLine);
            painter.setPen(pen);
            painter.drawRect(QRectF(-9, -9, 18, 18));
            pen.setStyle(Qt::SolidLine);
            painter.setPen(pen);
            painter.drawRect(QRectF(-5, -7, 10, 14));
        }
    }

private:
    int symbol_;
};
class CropToolBar : public QWidget {
public:
    explicit CropToolBar(QWidget* parent) : QWidget(parent) {}

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(36, 36, 36));
        painter.drawRoundedRect(rect(), 18, 18);
        painter.setPen(QPen(QColor(255, 255, 255, 65), 1));
        for (int index : {1, 4}) {
            const int x = 4 + index * 52 - 3;
            painter.drawLine(isRightToLeft() ? width() - x : x, 11,
                             isRightToLeft() ? width() - x : x, height() - 11);
        }
    }
};
}  // namespace

class ImageCropCanvas : public QWidget {
public:
    explicit ImageCropCanvas(ImageCropper* editor) : QWidget(editor), editor_(editor) {
        setMinimumSize(200, 160);
        setMouseTracking(true);
    }
    void resetDrag() { dragging_ = false; }

protected:
    void mousePressEvent(QMouseEvent* event) override {
        if (!isEnabled() || event->button() != Qt::LeftButton || editor_->image_.isNull())
            return QWidget::mousePressEvent(event);
        const auto rect = editor_->displaySelection();
        const QList<QPointF> corners = {rect.topLeft(), rect.topRight(), rect.bottomRight(),
                                        rect.bottomLeft()};
        handle_ = -1;
        for (int i = 0; i < corners.size(); ++i) {
            if ((corners[i] - event->localPos()).manhattanLength() <= 16) {
                handle_ = i;
                break;
            }
        }
        dragging_ = handle_ >= 0 || rect.contains(event->localPos());
        if (dragging_) {
            origin_ = event->localPos();
            initial_ = editor_->selection_;
            event->accept();
        }
    }
    void mouseMoveEvent(QMouseEvent* event) override {
        if (!dragging_) return QWidget::mouseMoveEvent(event);
        const qreal scale = editor_->imageRect().width() / editor_->image_.width();
        if (scale <= 0) return;
        const QPointF delta = (event->localPos() - origin_) / scale;
        QRectF rect = initial_;
        const QRectF bounds(editor_->image_.rect());
        if (handle_ < 0) {
            rect.translate(delta);
            rect.moveLeft(qBound(qreal(0), rect.x(), bounds.width() - rect.width()));
            rect.moveTop(qBound(qreal(0), rect.y(), bounds.height() - rect.height()));
        } else {
            const qreal minimum = qMin(16 / scale, qMin(initial_.width(), initial_.height()));
            if (handle_ == 0 || handle_ == 3)
                rect.setLeft(
                    qBound(qreal(0), initial_.left() + delta.x(), initial_.right() - minimum));
            else
                rect.setRight(qBound(initial_.left() + minimum, initial_.right() + delta.x(),
                                     bounds.right()));
            if (handle_ == 0 || handle_ == 1)
                rect.setTop(
                    qBound(qreal(0), initial_.top() + delta.y(), initial_.bottom() - minimum));
            else
                rect.setBottom(qBound(initial_.top() + minimum, initial_.bottom() + delta.y(),
                                      bounds.bottom()));
            if (editor_->shape_ == CropShape::CIRCLE) {
                const qreal side = qMin(rect.width(), rect.height());
                rect = QRectF(0, 0, side, side);
                if (handle_ == 0)
                    rect.moveBottomRight(initial_.bottomRight());
                else if (handle_ == 1)
                    rect.moveBottomLeft(initial_.bottomLeft());
                else if (handle_ == 2)
                    rect.moveTopLeft(initial_.topLeft());
                else
                    rect.moveTopRight(initial_.topRight());
            }
        }
        event->accept();
        editor_->setSelectionRect(rect);  // Signal callbacks may delete the entire editor.
    }
    void mouseReleaseEvent(QMouseEvent* event) override {
        dragging_ = false;
        QWidget::mouseReleaseEvent(event);
    }
    void hideEvent(QHideEvent* event) override {
        resetDrag();
        QWidget::hideEvent(event);
    }
    void resizeEvent(QResizeEvent* event) override {
        resetDrag();
        QWidget::resizeEvent(event);
    }
    void changeEvent(QEvent* event) override {
        if (event->type() == QEvent::EnabledChange && !isEnabled()) resetDrag();
        QWidget::changeEvent(event);
    }
    void paintEvent(QPaintEvent*) override {
        if (editor_->image_.isNull()) return;
        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        const auto rect = editor_->imageRect();
        painter.fillRect(rect, QColor(92, 92, 92));
        painter.drawImage(rect, editor_->image_, QRectF(editor_->image_.rect()));
        const auto selection = editor_->displaySelection();
        QPainterPath mask;
        mask.addRect(rect);
        painter.fillPath(mask.subtracted(editor_->cropPath(selection)), cropMask);
        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::white);
        for (const auto& corner : {selection.topLeft(), selection.topRight(),
                                   selection.bottomLeft(), selection.bottomRight()})
            painter.drawEllipse(corner, 8, 8);
    }

private:
    ImageCropper* editor_;
    bool dragging_ = false;
    int handle_ = -1;
    QPointF origin_;
    QRectF initial_;
};

ImageCropper::ImageCropper(QWidget* parent) : QDialog(parent ? parent->window() : nullptr) {
    owner_ = parentWidget();
    setWindowTitle(tr("Crop image"));
    setWindowFlags(owner_ ? Qt::WindowFlags(Qt::FramelessWindowHint)
                          : Qt::WindowFlags(Qt::Dialog | Qt::FramelessWindowHint));
    setAttribute(Qt::WA_TranslucentBackground);
    setFocusPolicy(Qt::StrongFocus);
    if (!owner_) setModal(true);
    setProperty("imageCropper", true);
    const auto transparent =
        QStringLiteral("QDialog[imageCropper=true] { background: transparent; }");
    qfw::setCustomStyleSheet(this, transparent, transparent);
    resize(860, 600);
    windowMask = new CropWindowMask(this);
    windowMask->setAttribute(Qt::WA_TransparentForMouseEvents);
    windowMask->resize(size());
    windowMask->lower();
    canvas_ = new ImageCropCanvas(this);
    toolBar = new CropToolBar(this);
    auto* toolbarLayout = new QHBoxLayout(toolBar);
    toolbarLayout->setContentsMargins(4, 4, 4, 4);
    toolbarLayout->setSpacing(6);
    cancelButton = new CropToolButton(0, tr("Cancel"), toolBar);
    rotateButton = new CropToolButton(1, tr("Rotate clockwise"), toolBar);
    flipButton = new CropToolButton(2, tr("Flip horizontally"), toolBar);
    shapeButton = new CropToolButton(3, tr("Custom crop shape"), toolBar);
    yesButton = new CropToolButton(4, tr("Confirm"), toolBar);
    shapeButton->setEnabled(false);
    for (auto* button : {cancelButton, rotateButton, flipButton, shapeButton, yesButton})
        toolbarLayout->addWidget(button);
    toolBar->setFixedSize(toolbarLayout->sizeHint());
    editorLayout = new QVBoxLayout(this);
    editorLayout->setSizeConstraint(QLayout::SetNoConstraint);
    editorLayout->setContentsMargins(24, 24, 24, 24);
    editorLayout->setSpacing(0);
    editorLayout->addWidget(canvas_, 1);
    toolbarGap_ = new QSpacerItem(0, 80, QSizePolicy::Minimum, QSizePolicy::Fixed);
    editorLayout->addSpacerItem(toolbarGap_);
    editorLayout->addWidget(toolBar, 0, Qt::AlignHCenter);
    connect(cancelButton, &QAbstractButton::clicked, this, &ImageCropper::reject);
    connect(rotateButton, &QAbstractButton::clicked, this, &ImageCropper::rotate);
    connect(flipButton, &QAbstractButton::clicked, this, &ImageCropper::flip);
    connect(yesButton, &QAbstractButton::clicked, this, &ImageCropper::accept);
    setImage(QImage());
}
ImageCropper::ImageCropper(const QImage& image, QWidget* parent) : ImageCropper(parent) {
    setImage(image);
}
ImageCropper::ImageCropper(const QPixmap& image, QWidget* parent)
    : ImageCropper(image.toImage(), parent) {}
ImageCropper::ImageCropper(const QString& path, QWidget* parent)
    : ImageCropper(QImage(path), parent) {}
ImageCropper::~ImageCropper() {
    if (filterInstalled_ && qApp) qApp->removeEventFilter(this);
}
void ImageCropper::setImage(const QImage& image) {
    canvas_->resetDrag();
    image_ = image;
    selection_ = image.isNull() ? QRectF() : QRectF(image.rect());
    if (!image.isNull())
        selection_.adjust(image.width() * .1, image.height() * .1, -image.width() * .1,
                          -image.height() * .1);
    yesButton->setEnabled(!image.isNull());
    rotateButton->setEnabled(!image.isNull());
    flipButton->setEnabled(!image.isNull());
    setCropShape(shape_);
}
void ImageCropper::setImage(const QPixmap& image) { setImage(image.toImage()); }
void ImageCropper::setImage(const QString& path) { setImage(QImage(path)); }
QImage ImageCropper::sourceImage() const { return image_; }
CropShape ImageCropper::cropShape() const { return shape_; }
void ImageCropper::setCropShape(CropShape shape) {
    if (shape != CropShape::RECTANGLE && shape != CropShape::CIRCLE && shape != CropShape::CUSTOM)
        throw std::invalid_argument("Invalid crop shape");
    canvas_->resetDrag();
    shape_ = shape;
    if (shape == CropShape::CIRCLE) {
        const qreal side = qMin(selection_.width(), selection_.height());
        QRectF square(0, 0, side, side);
        square.moveCenter(selection_.center());
        selection_ = square;
    }
    canvas_->update();
}
void ImageCropper::setCropPathFactory(CropPathFactory factory) {
    if (!factory) throw std::invalid_argument("Crop path factory must be callable");
    pathFactory_ = std::move(factory);
    setCropShape(CropShape::CUSTOM);
}
QRectF ImageCropper::selectionRect() const { return selection_; }
void ImageCropper::setSelectionRect(const QRectF& rect) {
    if (!std::isfinite(rect.x()) || !std::isfinite(rect.y()) || !std::isfinite(rect.width()) ||
        !std::isfinite(rect.height()) || rect.width() <= 0 || rect.height() <= 0)
        throw std::invalid_argument("Crop selection must be a finite, positive rectangle");
    QRectF selection = rect.intersected(QRectF(image_.rect()));
    if (selection.isEmpty()) throw std::invalid_argument("Crop selection must overlap the image");
    if (shape_ == CropShape::CIRCLE) {
        const qreal side = qMin(selection.width(), selection.height());
        QRectF square(0, 0, side, side);
        square.moveCenter(selection.center());
        selection = square;
    }
    if (selection_ == selection) return;
    selection_ = selection;
    canvas_->update();
    emit selectionChanged(selection);
}
QRectF ImageCropper::imageRect() const {
    if (image_.isNull()) return {};
    const qreal scale = qMax(qreal(0), qMin(qreal(canvas_->width() - 40) / image_.width(),
                                            qreal(canvas_->height() - 40) / image_.height()));
    const QSizeF size(image_.width() * scale, image_.height() * scale);
    return QRectF((canvas_->width() - size.width()) / 2, (canvas_->height() - size.height()) / 2,
                  size.width(), size.height());
}
QRectF ImageCropper::displaySelection() const {
    const auto rect = imageRect();
    if (rect.isEmpty()) return {};
    const qreal scale = rect.width() / image_.width();
    return QRectF(rect.x() + selection_.x() * scale, rect.y() + selection_.y() * scale,
                  selection_.width() * scale, selection_.height() * scale);
}
QPainterPath ImageCropper::cropPath(const QRectF& rect) const {
    if (shape_ == CropShape::CUSTOM && pathFactory_) return pathFactory_(rect);
    QPainterPath path;
    if (shape_ == CropShape::CIRCLE) {
        const qreal side = qMin(rect.width(), rect.height());
        QRectF circle(0, 0, side, side);
        circle.moveCenter(rect.center());
        path.addEllipse(circle);
    } else
        path.addRect(rect);
    return path;
}
QImage ImageCropper::croppedImage() const {
    if (image_.isNull() || selection_.isEmpty()) return {};
    auto image = image_.copy(selection_.toAlignedRect().intersected(image_.rect()));
    // The crop is in source-pixel coordinates, not a screen-DPI logical image.
    image.setDevicePixelRatio(1);
    if (image.isNull() || shape_ == CropShape::RECTANGLE) return image;
    QImage result(image.size(), QImage::Format_ARGB32_Premultiplied);
    result.fill(Qt::transparent);
    QPainter painter(&result);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setClipPath(cropPath(QRectF(image.rect())));
    painter.drawImage(0, 0, image);
    return result;
}
QWidget* ImageCropper::canvasWidget() const { return canvas_; }
void ImageCropper::rotate() {
    if (!image_.isNull()) setImage(image_.transformed(QTransform().rotate(90)));
}
void ImageCropper::flip() {
    if (!image_.isNull()) setImage(image_.mirrored(true, false));
}
void ImageCropper::accept() {
    if (finishing_) return;
    const auto image = croppedImage();
    if (image.isNull()) return;
    finishing_ = true;
    QPointer<ImageCropper> guard(this);
    emit imageCropped(image);
    if (guard) QDialog::done(QDialog::Accepted);
}
void ImageCropper::done(int code) {
    if (code == QDialog::Accepted) return accept();
    if (finishing_) return;
    finishing_ = true;
    QDialog::done(code);
}
void ImageCropper::syncOwnerGeometry() {
    if (!owner_) return;
    setGeometry(owner_->rect());
    const int left = qRound(owner_->width() * .07), top = qRound(owner_->height() * .12);
    editorLayout->setContentsMargins(left, top, left, 24);
    const int available = qMax(0, height() - top - 24 - toolBar->height());
    const int gap = qMin(80, qMax(0, available - 160));
    toolbarGap_->changeSize(0, gap, QSizePolicy::Minimum, QSizePolicy::Fixed);
    canvas_->setMinimumSize(qMin(200, qMax(0, width() - left * 2)), qMin(160, available - gap));
    editorLayout->invalidate();
    editorLayout->activate();
}
void ImageCropper::showEvent(QShowEvent* event) {
    finishing_ = false;
    previousFocus_ = QApplication::focusWidget();
    syncOwnerGeometry();
    raise();
    qApp->installEventFilter(this);
    filterInstalled_ = true;
    QDialog::showEvent(event);
    cancelButton->setFocus(Qt::PopupFocusReason);
}
void ImageCropper::hideEvent(QHideEvent* event) {
    canvas_->resetDrag();
    if (filterInstalled_ && qApp) qApp->removeEventFilter(this);
    filterInstalled_ = false;
    auto previous = previousFocus_;
    previousFocus_.clear();
    QDialog::hideEvent(event);
    if (previous && previous->isVisible() && previous->isEnabled())
        previous->setFocus(Qt::OtherFocusReason);
}
void ImageCropper::resizeEvent(QResizeEvent* event) {
    if (windowMask) windowMask->resize(size());
    QDialog::resizeEvent(event);
}
bool ImageCropper::eventFilter(QObject* watched, QEvent* event) {
    if (!filterInstalled_) return QDialog::eventFilter(watched, event);
    if (watched == owner_) {
        if (event->type() == QEvent::Resize || event->type() == QEvent::Move)
            syncOwnerGeometry();
        else if (event->type() == QEvent::Hide || event->type() == QEvent::Close)
            reject();
    }
    const auto* target = qobject_cast<QWidget*>(watched);
    const bool inside = target && (target == this || isAncestorOf(target));
    const bool background = target && owner_ && target->window() == owner_ && !inside;
    if (inside || background) {
        if (event->type() == QEvent::ShortcutOverride) {
            event->accept();
            return true;
        }
        if (event->type() == QEvent::KeyPress) {
            auto* key = static_cast<QKeyEvent*>(event);
            if (key->key() == Qt::Key_Escape) {
                event->accept();
                if (!key->isAutoRepeat()) reject();
                return true;
            }
            if (background) {
                event->accept();
                return true;
            }
        }
        if (background && event->type() == QEvent::KeyRelease) {
            event->accept();
            return true;
        }
    }
    return QDialog::eventFilter(watched, event);
}
bool ImageCropper::focusNextPrevChild(bool next) {
    const QList<QPushButton*> buttons = {cancelButton, rotateButton, flipButton, shapeButton,
                                         yesButton};
    int index = buttons.indexOf(qobject_cast<QPushButton*>(QApplication::focusWidget()));
    for (int i = 0; i < buttons.size(); ++i) {
        index = (index + (next ? 1 : buttons.size() - 1) + buttons.size()) % buttons.size();
        if (buttons[index]->isEnabled()) {
            buttons[index]->setFocus(Qt::TabFocusReason);
            return true;
        }
    }
    return false;
}
}  // namespace qfw
