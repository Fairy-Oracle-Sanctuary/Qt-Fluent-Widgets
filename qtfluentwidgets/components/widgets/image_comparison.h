#pragma once

#include <QImage>
#include <QPixmap>
#include <QWidget>

namespace qfw {

// Before is always on the physical left, after on the right (also in RTL).
class ImageComparisonSlider : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal value READ value WRITE setValue NOTIFY valueChanged)
public:
    explicit ImageComparisonSlider(QWidget* parent = nullptr);
    ImageComparisonSlider(const QImage& before, const QImage& after, QWidget* parent = nullptr);
    ImageComparisonSlider(const QPixmap& before, const QPixmap& after, QWidget* parent = nullptr);
    ImageComparisonSlider(const QString& before, const QString& after, QWidget* parent = nullptr);

    void setImages(const QImage& before, const QImage& after);
    void setImages(const QPixmap& before, const QPixmap& after);
    void setImages(const QString& before, const QString& after);
    void setBeforeImage(const QImage& image);
    void setBeforeImage(const QPixmap& image);
    void setBeforeImage(const QString& path);
    void setAfterImage(const QImage& image);
    void setAfterImage(const QPixmap& image);
    void setAfterImage(const QString& path);
    QImage beforeImage() const;
    QImage afterImage() const;
    qreal value() const;
    void setValue(qreal value);
    QSize sizeHint() const override;
    void scaledToWidth(int width);
    void scaledToHeight(int height);
    void setScaledSize(const QSize& size);

signals:
    void valueChanged(qreal value);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    void moveDivider(const QPointF& position);
    QImage before_;
    QImage after_;
    qreal value_ = 50;
    bool dragging_ = false;
};

}  // namespace qfw
