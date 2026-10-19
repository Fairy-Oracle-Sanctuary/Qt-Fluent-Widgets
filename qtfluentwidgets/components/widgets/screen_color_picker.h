#pragma once
#include <QImage>
#include <QScreen>
#include <QTimer>
#include "components/widgets/button.h"

namespace qfw {
class ScreenPickerOverlay;
class ScreenColorPicker : public FluentPushButton {
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(bool freezeScreenEnabled READ isFreezeScreenEnabled WRITE setFreezeScreenEnabled)
    Q_PROPERTY(bool picking READ isPicking NOTIFY pickingChanged)
public:
    explicit ScreenColorPicker(QWidget* parent = nullptr);
    explicit ScreenColorPicker(const QColor& color,QWidget* parent = nullptr);
    ~ScreenColorPicker() override;
    QColor color() const;
    void setColor(const QColor& color);
    bool isPicking() const;
    bool isFreezeScreenEnabled() const;
    void setFreezeScreenEnabled(bool enabled);
    bool startPicking();
    void cancelPicking();
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
signals:
    void colorChanged(const QColor& color);
    void colorPicked(const QColor& color);
    void colorHovered(const QColor& color);
    void pickingChanged(bool picking);
    void pickingCanceled();
    void errorOccurred(const QString& error);
protected:
    struct ScreenSnapshot { QPointer<QScreen> screen; QRect geometry; QImage image; };
    // Capture hooks also allow applications with a custom capture backend to
    // preserve the same multi-screen coordinates and session behavior.
    virtual QList<ScreenSnapshot> captureScreens();
    virtual QColor captureLiveColor(QScreen* screen,const QRect& geometry,const QPointF& position);
    virtual void prepareLiveOverlay(QWidget* overlay);
    static QColor sampleImage(const QImage& image,const QRect& geometry,const QPointF& position);
    static QPoint previewPosition(const QPoint& position,const QRect& bounds,const QSize& size);
    bool eventFilter(QObject* watched,QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void changeEvent(QEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
private:
    friend class ScreenPickerOverlay;
    void cleanup();
    void previewAt(const QPointF& position);
    void confirmAt(const QPointF& position);
    void updateFromCursor();
    ScreenPickerOverlay* overlayAt(const QPointF& position) const;
    QColor readColor(ScreenPickerOverlay* overlay,const QPointF& position);
    QColor color_,hoverColor_;
    bool freeze_=true,picking_=false,hasPosition_=false;
    QPointF lastPosition_;
    QList<ScreenPickerOverlay*> overlays_;
    QList<QMetaObject::Connection> topologyConnections_;
    QPointer<QWidget> previousFocus_;
    QTimer timer_;
};
}
