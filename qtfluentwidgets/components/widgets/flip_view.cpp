#include "components/widgets/flip_view.h"

#include <QApplication>
#include <QImageReader>
#include <QListWidgetItem>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOptionViewItem>

#include "common/icon.h"
#include "common/style_sheet.h"

namespace qfw {

// ============================================================================
// FlipScrollButton
// ============================================================================

FlipScrollButton::FlipScrollButton(QWidget* parent) : ToolButton(parent) { init(); }

FlipScrollButton::FlipScrollButton(FluentIconEnum icon, QWidget* parent)
    : ToolButton(parent), iconEnum_(icon) {
    init();
}

void FlipScrollButton::init() {
    opacity_ = 0;
    opacityAni_ = new QPropertyAnimation(this, "opacity", this);
    opacityAni_->setDuration(150);
}

float FlipScrollButton::opacity() const { return opacity_; }

void FlipScrollButton::setOpacity(float o) {
    opacity_ = o;
    update();
}

bool FlipScrollButton::isTransparent() const { return opacity_ == 0; }

void FlipScrollButton::fadeIn() {
    opacityAni_->setStartValue(opacity_);
    opacityAni_->setEndValue(1.0);
    opacityAni_->start();
}

void FlipScrollButton::fadeOut() {
    opacityAni_->setStartValue(opacity_);
    opacityAni_->setEndValue(0.0);
    opacityAni_->start();
}

void FlipScrollButton::paintEvent(QPaintEvent* e) {
    Q_UNUSED(e);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setOpacity(opacity_);

    // draw background
    if (!isDarkTheme()) {
        painter.setBrush(QColor(252, 252, 252, 217));
    } else {
        painter.setBrush(QColor(44, 44, 44, 245));
    }

    painter.drawRoundedRect(rect(), 4, 4);

    // draw icon
    QColor color;
    float iconOpacity;
    if (isDarkTheme()) {
        color = QColor(255, 255, 255);
        iconOpacity = (isHover || isPressed) ? 0.773f : 0.541f;
    } else {
        color = QColor(0, 0, 0);
        iconOpacity = (isHover || isPressed) ? 0.616f : 0.45f;
    }

    painter.setOpacity(opacity_ * iconOpacity);

    int s = isPressed ? 6 : 8;
    int w = width();
    int h = height();
    qreal x = (w - s) / 2.0;
    qreal y = (h - s) / 2.0;

    FluentIcon icon(iconEnum_);
    icon.render(&painter, QRect(x, y, s, s));
}

// ============================================================================
// FlipImageDelegate
// ============================================================================

FlipImageDelegate::FlipImageDelegate(QObject* parent) : QStyledItemDelegate(parent) {}

void FlipImageDelegate::setBorderRadius(int radius) {
    borderRadius_ = radius;
    if (auto* view = qobject_cast<QListWidget*>(parent())) {
        if (view->viewport()) {
            view->viewport()->update();
        }
    }
}

int FlipImageDelegate::borderRadius() const { return borderRadius_; }

QSize FlipImageDelegate::itemSize(int index) const {
    auto* view = qobject_cast<QListWidget*>(parent());
    if (!view) return QSize();

    if (auto* item = view->item(index)) {
        return item->sizeHint();
    }
    return QSize();
}

void FlipImageDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                              const QModelIndex& index) const {
    painter->save();
    painter->setRenderHints(QPainter::Antialiasing);
    painter->setPen(Qt::NoPen);

    auto* view = qobject_cast<QListWidget*>(parent());
    if (!view) {
        painter->restore();
        return;
    }

    QSize size = itemSize(index.row());

    // draw image
    qreal r = view->devicePixelRatioF();
    QImage image = index.data(Qt::UserRole).value<QImage>();

    // lazy load image
    if (image.isNull() && index.data(Qt::DisplayRole).isValid()) {
        QString path = index.data(Qt::DisplayRole).toString();
        if (!path.isEmpty()) {
            image.load(path);
        }
    }

    if (image.isNull()) {
        painter->restore();
        return;
    }

    int x = option.rect.x() + (option.rect.width() - size.width()) / 2;
    int y = option.rect.y() + (option.rect.height() - size.height()) / 2;
    QRectF rect(x, y, size.width(), size.height());

    // clipped path
    QPainterPath path;
    path.addRoundedRect(rect, borderRadius_, borderRadius_);

    QPainterPath subPath;
    subPath.addRoundedRect(QRectF(view->rect()), borderRadius_, borderRadius_);
    path = path.intersected(subPath);

    image = image.scaled(size * r, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    painter->setClipPath(path);

    painter->drawImage(rect, image);
    painter->restore();
}

// ============================================================================
// FlipView
// ============================================================================

FlipView::FlipView(QWidget* parent) : QListWidget(parent) {
    orientation_ = Qt::Horizontal;
    init();
}

FlipView::FlipView(Qt::Orientation orientation, QWidget* parent) : QListWidget(parent) {
    orientation_ = orientation;
    init();
}

void FlipView::init() {
    isHover_ = false;
    currentIndex_ = -1;
    aspectRatioMode_ = Qt::IgnoreAspectRatio;
    itemSize_ = QSize(480, 270);

    delegate_ = new FlipImageDelegate(this);
    scrollBar_ = new SmoothScrollBar(orientation_, this);

    scrollBar_->setScrollAnimation(500);
    scrollBar_->setForceHidden(true);

    setMinimumSize(itemSize_);
    setItemDelegate(delegate_);
    setMovement(QListWidget::Static);
    setVerticalScrollMode(QListWidget::ScrollPerPixel);
    setHorizontalScrollMode(QListWidget::ScrollPerPixel);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    qfw::setStyleSheet(this, FluentStyleSheet::FlipView);

    if (isHorizontal()) {
        setFlow(QListWidget::LeftToRight);
        preButton_ = new FlipScrollButton(FluentIconEnum::CareLeftSolid, this);
        nextButton_ = new FlipScrollButton(FluentIconEnum::CareRightSolid, this);
        preButton_->setFixedSize(16, 38);
        nextButton_->setFixedSize(16, 38);
    } else {
        preButton_ = new FlipScrollButton(FluentIconEnum::CareUpSolid, this);
        nextButton_ = new FlipScrollButton(FluentIconEnum::CareDownSolid, this);
        preButton_->setFixedSize(38, 16);
        nextButton_->setFixedSize(38, 16);
    }

    connect(preButton_, &FlipScrollButton::clicked, this, &FlipView::scrollPrevious);
    connect(nextButton_, &FlipScrollButton::clicked, this, &FlipView::scrollNext);
}

bool FlipView::isHorizontal() const { return orientation_ == Qt::Horizontal; }

QSize FlipView::itemSize() const { return itemSize_; }

void FlipView::setItemSize(const QSize& size) {
    if (size == itemSize_) return;

    itemSize_ = size;
    for (int i = 0; i < count(); ++i) {
        adjustItemSize(item(i));
    }
    viewport()->update();
}

int FlipView::borderRadius() const { return delegate_->borderRadius(); }

void FlipView::setBorderRadius(int radius) { delegate_->setBorderRadius(radius); }

Qt::AspectRatioMode FlipView::aspectRatioMode() const { return aspectRatioMode_; }

void FlipView::setAspectRatioMode(Qt::AspectRatioMode mode) {
    if (mode == aspectRatioMode_) return;

    aspectRatioMode_ = mode;
    for (int i = 0; i < count(); ++i) {
        adjustItemSize(item(i));
    }
    viewport()->update();
}

int FlipView::currentIndex() const { return currentIndex_; }

void FlipView::setCurrentIndex(int index) {
    if (!(0 <= index && index < count()) || index == currentIndex_) return;

    scrollToIndex(index);

    // update the visibility of scroll button
    if (index == 0) {
        preButton_->fadeOut();
    } else if (preButton_->isTransparent() && isHover_) {
        preButton_->fadeIn();
    }

    if (index == count() - 1) {
        nextButton_->fadeOut();
    } else if (nextButton_->isTransparent() && isHover_) {
        nextButton_->fadeIn();
    }

    emit currentIndexChanged(index);
}

QImage FlipView::image(int index) const {
    if (!(0 <= index && index < count())) return QImage();
    return item(index)->data(Qt::UserRole).value<QImage>();
}

void FlipView::addImage(const QImage& image) { addImages(QList<QImage>{image}); }

void FlipView::addImage(const QPixmap& pixmap) { addImages(QList<QPixmap>{pixmap}); }

void FlipView::addImage(const QString& path) { addImages(QList<QString>{path}); }

void FlipView::addImages(const QList<QImage>& images) {
    if (images.isEmpty()) return;

    int n = count();
    for (int i = 0; i < images.size(); ++i) {
        addItem("");
        setItemImage(n + i, images[i]);
    }

    if (currentIndex_ < 0) currentIndex_ = 0;
}

void FlipView::addImages(const QList<QPixmap>& pixmaps) {
    if (pixmaps.isEmpty()) return;

    int n = count();
    for (int i = 0; i < pixmaps.size(); ++i) {
        addItem("");
        setItemImage(n + i, pixmaps[i]);
    }

    if (currentIndex_ < 0) currentIndex_ = 0;
}

void FlipView::addImages(const QList<QString>& paths) {
    if (paths.isEmpty()) return;

    int n = count();
    for (int i = 0; i < paths.size(); ++i) {
        addItem("");
        setItemImage(n + i, paths[i]);
    }

    if (currentIndex_ < 0) currentIndex_ = 0;
}

void FlipView::setItemImage(int index, const QImage& image) {
    if (!(0 <= index && index < count())) return;

    QListWidgetItem* it = item(index);
    it->setData(Qt::UserRole, image);
    adjustItemSize(it);
}

void FlipView::setItemImage(int index, const QPixmap& pixmap) {
    setItemImage(index, pixmap.toImage());
}

void FlipView::setItemImage(int index, const QString& path) {
    if (!(0 <= index && index < count())) return;

    QListWidgetItem* it = item(index);
    it->setData(Qt::UserRole, QImage());
    it->setData(Qt::DisplayRole, path);
    adjustItemSize(it);
}

void FlipView::scrollPrevious() { setCurrentIndex(currentIndex() - 1); }

void FlipView::scrollNext() { setCurrentIndex(currentIndex() + 1); }

void FlipView::scrollToIndex(int index) {
    if (!(0 <= index && index < count())) return;

    currentIndex_ = index;

    int value = 0;
    if (isHorizontal()) {
        for (int i = 0; i < index; ++i) {
            value += item(i)->sizeHint().width();
        }
    } else {
        for (int i = 0; i < index; ++i) {
            value += item(i)->sizeHint().height();
        }
    }

    value += (2 * index + 1) * spacing();
    scrollBar_->scrollTo(value);
}

void FlipView::adjustItemSize(QListWidgetItem* it) {
    QImage img = itemImage(row(it), false);

    QSize size;
    if (!img.isNull()) {
        size = img.size();
    } else {
        QString imagePath = it->data(Qt::DisplayRole).toString();
        QImageReader reader(imagePath);
        size = reader.size().expandedTo(QSize(1, 1));
    }

    int w, h;
    if (aspectRatioMode_ == Qt::KeepAspectRatio) {
        if (isHorizontal()) {
            h = itemSize_.height();
            w = static_cast<int>(size.width() * h / static_cast<double>(size.height()));
        } else {
            w = itemSize_.width();
            h = static_cast<int>(size.height() * w / static_cast<double>(size.width()));
        }
    } else {
        w = itemSize_.width();
        h = itemSize_.height();
    }

    it->setSizeHint(QSize(w, h));
}

QImage FlipView::itemImage(int index, bool load) const {
    if (!(0 <= index && index < count())) return QImage();

    QListWidgetItem* it = item(index);
    QImage image = it->data(Qt::UserRole).value<QImage>();

    QString imagePath = it->data(Qt::DisplayRole).toString();
    if (image.isNull() && !imagePath.isEmpty() && load) {
        image.load(imagePath);
    }

    return image;
}

void FlipView::resizeEvent(QResizeEvent* e) {
    QListWidget::resizeEvent(e);

    int w = width();
    int h = height();
    int bw = preButton_->width();
    int bh = preButton_->height();

    if (isHorizontal()) {
        preButton_->move(2, h / 2 - bh / 2);
        nextButton_->move(w - bw - 2, h / 2 - bh / 2);
    } else {
        preButton_->move(w / 2 - bw / 2, 2);
        nextButton_->move(w / 2 - bw / 2, h - bh - 2);
    }
}

void FlipView::enterEvent(QEnterEvent* e) {
    QListWidget::enterEvent(e);
    isHover_ = true;

    if (currentIndex() > 0) {
        preButton_->fadeIn();
    }

    if (currentIndex() < count() - 1) {
        nextButton_->fadeIn();
    }
}

void FlipView::leaveEvent(QEvent* e) {
    QListWidget::leaveEvent(e);
    isHover_ = false;
    preButton_->fadeOut();
    nextButton_->fadeOut();
}

void FlipView::showEvent(QShowEvent* e) {
    QListWidget::showEvent(e);
    scrollBar_->setDuration(0);
    scrollToIndex(currentIndex());
    scrollBar_->setDuration(500);
}

void FlipView::wheelEvent(QWheelEvent* e) {
    e->accept();
    if (scrollBar_->isAnimationRunning()) return;

    if (e->angleDelta().y() < 0) {
        scrollNext();
    } else {
        scrollPrevious();
    }
}

// ============================================================================
// HorizontalFlipView
// ============================================================================

HorizontalFlipView::HorizontalFlipView(QWidget* parent) : FlipView(Qt::Horizontal, parent) {}

// ============================================================================
// VerticalFlipView
// ============================================================================

VerticalFlipView::VerticalFlipView(QWidget* parent) : FlipView(Qt::Vertical, parent) {}

}  // namespace qfw
