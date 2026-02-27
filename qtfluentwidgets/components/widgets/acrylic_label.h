#pragma once

#include <QColor>
#include <QImage>
#include <QLabel>
#include <QPainterPath>
#include <QPixmap>
#include <QPointer>
#include <QRect>
#include <QWidget>

namespace qfw {

class AcrylicTextureLabel : public QLabel {
    Q_OBJECT

public:
    explicit AcrylicTextureLabel(const QColor& tintColor, const QColor& luminosityColor,
                                 qreal noiseOpacity = 0.03, QWidget* parent = nullptr);

    void setTintColor(const QColor& color);

protected:
    void paintEvent(QPaintEvent* e) override;

private:
    QColor tintColor_;
    QColor luminosityColor_;
    qreal noiseOpacity_;
    QImage noiseImage_;
};

class AcrylicLabel : public QLabel {
    Q_OBJECT

public:
    explicit AcrylicLabel(int blurRadius, const QColor& tintColor,
                          const QColor& luminosityColor = QColor(255, 255, 255, 0),
                          const QSize& maxBlurSize = QSize(), QWidget* parent = nullptr);

    void setImage(const QString& imagePath);
    void setTintColor(const QColor& color);
    void setBlurRadius(int radius);

protected:
    void resizeEvent(QResizeEvent* e) override;

private:
    void updateBlur();

    QString imagePath_;
    QPixmap blurPixmap_;
    int blurRadius_;
    QSize maxBlurSize_;

    QPointer<AcrylicTextureLabel> acrylicTextureLabel_;
};

class AcrylicBrush {
public:
    explicit AcrylicBrush(QWidget* device, int blurRadius,
                          const QColor& tintColor = QColor(242, 242, 242, 150),
                          const QColor& luminosityColor = QColor(255, 255, 255, 10),
                          qreal noiseOpacity = 0.03);

    void setBlurRadius(int radius);
    void setTintColor(const QColor& color);
    void setLuminosityColor(const QColor& color);

    void grabImage(const QRect& rect);
    void setImage(const QPixmap& image);
    void setClipPath(const QPainterPath& path);

    QImage textureImage() const;
    void paint(QPainter* painter);

    bool isAvailable() const { return true; }

private:
    QWidget* device_ = nullptr;
    int blurRadius_;
    QColor tintColor_;
    QColor luminosityColor_;
    qreal noiseOpacity_;

    QImage noiseImage_;
    QPixmap originalImage_;
    QPixmap image_;

    QPainterPath clipPath_;
};

}  // namespace qfw
