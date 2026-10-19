#pragma once

#include <QEnterEvent>

#include "common/qtcompat.h"
#include "components/widgets/label.h"

namespace qfw {

class ImageMagnifierWidget : public ImageLabel {
    Q_OBJECT
    Q_PROPERTY(qreal magnification READ magnification WRITE setMagnification)
    Q_PROPERTY(int radius READ radius WRITE setRadius)
    Q_PROPERTY(bool magnifierEnabled READ isMagnifierEnabled WRITE setMagnifierEnabled)
public:
    explicit ImageMagnifierWidget(QWidget* parent = nullptr);
    explicit ImageMagnifierWidget(const QImage& image, QWidget* parent = nullptr);
    explicit ImageMagnifierWidget(const QPixmap& image, QWidget* parent = nullptr);
    explicit ImageMagnifierWidget(const QString& path, QWidget* parent = nullptr);
    qreal magnification() const;
    void setMagnification(qreal factor);
    int radius() const;
    void setRadius(int radius);
    bool isMagnifierEnabled() const;
    void setMagnifierEnabled(bool enabled);
    QPointF lensPosition() const;

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(enterEvent_QEnterEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    qreal magnification_ = 2;
    int radius_ = 50;
    bool magnifierEnabled_ = true;
    bool hovering_ = false;
    QPointF position_;
};

}  // namespace qfw
