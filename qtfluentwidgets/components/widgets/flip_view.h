#pragma once

#include <QColor>
#include <QEnterEvent>
#include <QImage>
#include <QListWidget>
#include <QModelIndex>
#include <QPropertyAnimation>
#include <QRectF>
#include <QSize>
#include <QSizeF>
#include <QStyledItemDelegate>
#include <QVariant>
#include <QWheelEvent>

#include "common/icon.h"
#include "components/widgets/button.h"
#include "components/widgets/scroll_bar.h"

class QStyleOptionViewItem;
class QPainter;

namespace qfw {

class FlipImageDelegate;

/**
 * @brief Scroll button for FlipView
 */
class FlipScrollButton : public ToolButton {
    Q_OBJECT
    Q_PROPERTY(float opacity READ opacity WRITE setOpacity)

public:
    explicit FlipScrollButton(QWidget* parent = nullptr);
    explicit FlipScrollButton(FluentIconEnum icon, QWidget* parent = nullptr);

    float opacity() const;
    void setOpacity(float o);

    bool isTransparent() const;

    void fadeIn();
    void fadeOut();

protected:
    void paintEvent(QPaintEvent* e) override;

private:
    void init();

    float opacity_ = 0;
    QPropertyAnimation* opacityAni_ = nullptr;
    FluentIconEnum iconEnum_ = FluentIconEnum::Up;
};

/**
 * @brief Flip view image delegate
 */
class FlipImageDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit FlipImageDelegate(QObject* parent = nullptr);

    void setBorderRadius(int radius);
    int borderRadius() const;

    QSize itemSize(int index) const;

protected:
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

private:
    int borderRadius_ = 0;
};

/**
 * @brief Flip view widget
 */
class FlipView : public QListWidget {
    Q_OBJECT
    Q_PROPERTY(QSize itemSize READ itemSize WRITE setItemSize)
    Q_PROPERTY(int borderRadius READ borderRadius WRITE setBorderRadius)
    Q_PROPERTY(Qt::AspectRatioMode aspectRatioMode READ aspectRatioMode WRITE setAspectRatioMode)

public:
    explicit FlipView(QWidget* parent = nullptr);
    explicit FlipView(Qt::Orientation orientation, QWidget* parent = nullptr);

    bool isHorizontal() const;

    QSize itemSize() const;
    void setItemSize(const QSize& size);

    int borderRadius() const;
    void setBorderRadius(int radius);

    Qt::AspectRatioMode aspectRatioMode() const;
    void setAspectRatioMode(Qt::AspectRatioMode mode);

    int currentIndex() const;
    void setCurrentIndex(int index);

    QImage image(int index) const;
    void addImage(const QImage& image);
    void addImage(const QPixmap& pixmap);
    void addImage(const QString& path);
    void addImages(const QList<QImage>& images);
    void addImages(const QList<QPixmap>& pixmaps);
    void addImages(const QList<QString>& paths);

    void setItemImage(int index, const QImage& image);
    void setItemImage(int index, const QPixmap& pixmap);
    void setItemImage(int index, const QString& path);

signals:
    void currentIndexChanged(int index);

public slots:
    void scrollPrevious();
    void scrollNext();

protected:
    void resizeEvent(QResizeEvent* e) override;
    void enterEvent(QEnterEvent* e) override;
    void leaveEvent(QEvent* e) override;
    void showEvent(QShowEvent* e) override;
    void wheelEvent(QWheelEvent* e) override;

private:
    void init();
    void scrollToIndex(int index);
    void adjustItemSize(QListWidgetItem* item);
    QImage itemImage(int index, bool load = true) const;

    Qt::Orientation orientation_ = Qt::Horizontal;
    bool isHover_ = false;
    int currentIndex_ = -1;
    Qt::AspectRatioMode aspectRatioMode_ = Qt::IgnoreAspectRatio;
    QSize itemSize_ = QSize(480, 270);

    FlipImageDelegate* delegate_ = nullptr;
    SmoothScrollBar* scrollBar_ = nullptr;
    FlipScrollButton* preButton_ = nullptr;
    FlipScrollButton* nextButton_ = nullptr;
};

/**
 * @brief Horizontal flip view
 */
class HorizontalFlipView : public FlipView {
    Q_OBJECT

public:
    explicit HorizontalFlipView(QWidget* parent = nullptr);
};

/**
 * @brief Vertical flip view
 */
class VerticalFlipView : public FlipView {
    Q_OBJECT

public:
    explicit VerticalFlipView(QWidget* parent = nullptr);
};

}  // namespace qfw
