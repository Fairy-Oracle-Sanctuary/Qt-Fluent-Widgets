#include "components/widgets/screen_color_picker.h"
#include <QApplication>
#include <QCloseEvent>
#include <QCursor>
#include <QGraphicsDropShadowEffect>
#include <QKeyEvent>
#include <QOperatingSystemVersion>
#include <QPainterPath>
#include <QWindow>
#include <cmath>
#include <stdexcept>
#include "common/config.h"
#include "common/font.h"
#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace qfw {
namespace {
QPointer<ScreenColorPicker> activePicker;
bool contains(const QRect& rect,const QPointF& p) {
    return p.x()>=rect.x()&&p.x()<rect.x()+rect.width()&&p.y()>=rect.y()&&p.y()<rect.y()+rect.height();
}
QColor validScreenColor(const QColor& color) {
    if(!color.isValid()) throw std::invalid_argument("Invalid screen picker color");
    return color;
}
QPointF globalPosition(QMouseEvent* event) {
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
    return event->globalPosition();
#else
    return event->screenPos();
#endif
}
void drawSwatch(QPainter& painter,const QRectF& rect,const QColor& color,int radius) {
    QPainterPath clip; clip.addRoundedRect(rect,radius,radius);
    if(color.alpha()<255) {
        painter.save(); painter.setClipPath(clip);
        for(int y=0;y<rect.height()+4;y+=4) for(int x=0;x<rect.width()+4;x+=4)
            painter.fillRect(QRectF(rect.x()+x,rect.y()+y,4,4),QColor(isDarkTheme()?((x/4+y/4)%2?"#777777":"#555555"):((x/4+y/4)%2?"#DDDDDD":"#FFFFFF")));
        painter.restore();
    }
    painter.fillPath(clip,color);
}
class ScreenPreviewCard : public QWidget {
public:
    explicit ScreenPreviewCard(QWidget* parent) : QWidget(parent) {
        setObjectName("screenPreviewCard"); setFixedSize(106,40); setFont(qfw::getFont(14));
        setAttribute(Qt::WA_TransparentForMouseEvents);
        auto* shadow=new QGraphicsDropShadowEffect(this); shadow->setBlurRadius(22);
        shadow->setOffset(0,6); shadow->setColor(QColor(0,0,0,45)); setGraphicsEffect(shadow);
    }
    QColor color=Qt::white;
protected:
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this); painter.setRenderHints(QPainter::Antialiasing|QPainter::TextAntialiasing);
        const QColor border=isDarkTheme()?QColor(255,255,255,20):QColor(0,0,0,18);
        painter.setPen(QPen(border,1)); painter.setBrush(QColor(isDarkTheme()?"#2B2B2B":"#F7F7F7"));
        painter.drawRoundedRect(QRectF(rect()).adjusted(.5,.5,-.5,-.5),8,8);
        const QRectF swatch(layoutDirection()==Qt::RightToLeft?70:4,4,32,32);
        drawSwatch(painter,swatch,color,4); painter.setPen(QPen(border,1)); painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(swatch,4,4); painter.setPen(isDarkTheme()?Qt::white:Qt::black); painter.setFont(font());
        painter.drawText(QRectF(layoutDirection()==Qt::RightToLeft?4:44,0,58,40),Qt::AlignLeft|Qt::AlignVCenter,color.name());
    }
};
class ScreenPreview : public QWidget {
public:
    explicit ScreenPreview(QWidget* parent) : QWidget(parent) {
        setObjectName("screenColorPreview"); setAttribute(Qt::WA_TransparentForMouseEvents);
        card=new ScreenPreviewCard(this); setFixedSize(138,76); card->move(16,12); hide();
    }
    void setColor(const QColor& color) { if(card->color!=color) { card->color=color; card->update(); } }
    ScreenPreviewCard* card;
};
}

class ScreenPickerOverlay : public QWidget {
public:
    ScreenPickerOverlay(ScreenColorPicker* picker,const ScreenColorPicker::ScreenSnapshot& snapshot)
        : QWidget(picker,Qt::Tool|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint),
          picker_(picker),screen(snapshot.screen),geometry(snapshot.geometry),frozen_(picker->isFreezeScreenEnabled()) {
        setObjectName("screenPickerOverlay");
        image_=frozen_?snapshot.image:QImage();
        if(!frozen_) setAttribute(Qt::WA_TranslucentBackground);
        setMouseTracking(true); setFocusPolicy(Qt::StrongFocus); setCursor(Qt::ArrowCursor);
        setAccessibleName(QCoreApplication::translate("qfw::ScreenColorPicker","Screen color picking: click to select, Escape to cancel"));
        winId(); if(screen&&windowHandle()) windowHandle()->setScreen(screen);
        setGeometry(geometry); preview=new ScreenPreview(this);
    }
    QColor colorAt(const QPointF& p) {
        return frozen_?ScreenColorPicker::sampleImage(image_,geometry,p):picker_->captureLiveColor(screen,geometry,p);
    }
    void dispose() { hide(); image_=QImage(); picker_=nullptr; deleteLater(); }
    QPointer<QScreen> screen; QRect geometry; ScreenPreview* preview;
protected:
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);
        if(!frozen_) painter.fillRect(rect(),QColor(0,0,0,1));
        else painter.drawImage(QRectF(rect()),image_,QRectF(0,0,image_.width(),image_.height()));
    }
    void mouseMoveEvent(QMouseEvent* event) override {
        if(picker_) picker_->previewAt(globalPosition(event)); event->accept();
    }
    void mousePressEvent(QMouseEvent* event) override {
        if(picker_) {
            if(event->button()==Qt::LeftButton) picker_->confirmAt(globalPosition(event));
            else if(event->button()==Qt::RightButton) picker_->cancelPicking();
        }
        event->accept();
    }
    void mouseReleaseEvent(QMouseEvent* event) override { event->accept(); }
    void closeEvent(QCloseEvent* event) override { if(picker_) picker_->cancelPicking(); event->accept(); }
private:
    QPointer<ScreenColorPicker> picker_; bool frozen_; QImage image_;
};

ScreenColorPicker::ScreenColorPicker(QWidget* parent) : ScreenColorPicker(QColor("#0078D4"),parent) {}
ScreenColorPicker::ScreenColorPicker(const QColor& color,QWidget* parent)
    : FluentPushButton(parent),color_(validScreenColor(color)) {
    setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed); setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::StrongFocus); setAccessibleName(tr("Pick a screen color"));
    setAccessibleDescription(color_.name(QColor::HexArgb)); timer_.setInterval(33);
    connect(&timer_,&QTimer::timeout,this,&ScreenColorPicker::updateFromCursor);
    connect(this,&QPushButton::clicked,this,&ScreenColorPicker::startPicking);
    connect(qApp,&QCoreApplication::aboutToQuit,this,&ScreenColorPicker::cancelPicking);
    connect(&QConfig::instance(),&QConfig::themeChangedFinished,this,[this] {
        update(); for(auto* overlay:overlays_) overlay->preview->card->update();
    });
}
ScreenColorPicker::~ScreenColorPicker() { blockSignals(true); cleanup(); }
QColor ScreenColorPicker::color() const { return color_; }
void ScreenColorPicker::setColor(const QColor& value) {
    QColor color=validScreenColor(value); if(color_==color) return;
    color_=color; setAccessibleDescription(color.name(QColor::HexArgb)); update(); emit colorChanged(color_);
}
bool ScreenColorPicker::isPicking() const { return picking_; }
bool ScreenColorPicker::isFreezeScreenEnabled() const { return freeze_; }
void ScreenColorPicker::setFreezeScreenEnabled(bool enabled) {
    if(freeze_==enabled) return;
    QPointer<ScreenColorPicker> guard(this); cancelPicking(); if(guard) freeze_=enabled;
}
QList<ScreenColorPicker::ScreenSnapshot> ScreenColorPicker::captureScreens() {
    const auto screens=QGuiApplication::screens();
    if(screens.isEmpty()) throw std::runtime_error(tr("No screen is available for color picking.").toStdString());
    QList<ScreenSnapshot> result;
    for(auto* screen:screens) {
        const auto image=screen->grabWindow(0).toImage(); const auto geometry=screen->geometry();
        if(image.isNull()||geometry.isEmpty()) throw std::runtime_error(tr("Screen capture is unavailable. Check screen recording permissions.").toStdString());
        result.append({screen,geometry,image});
    }
    return result;
}
QColor ScreenColorPicker::sampleImage(const QImage& image,const QRect& geometry,const QPointF& p) {
    if(image.isNull()||geometry.isEmpty()||!contains(geometry,p)) return QColor();
    const int x=int((p.x()-geometry.x())*image.width()/geometry.width());
    const int y=int((p.y()-geometry.y())*image.height()/geometry.height());
    return image.pixelColor(qMin(image.width()-1,x),qMin(image.height()-1,y));
}
QPoint ScreenColorPicker::previewPosition(const QPoint& p,const QRect& bounds,const QSize& size) {
    int x=p.x()+16,y=p.y()+16;
    if(x+size.width()>bounds.x()+bounds.width()) x=p.x()-size.width()-16;
    if(y+size.height()>bounds.y()+bounds.height()) y=p.y()-size.height()-16;
    return QPoint(qBound(bounds.x(),x,bounds.x()+qMax(0,bounds.width()-size.width())),
                  qBound(bounds.y(),y,bounds.y()+qMax(0,bounds.height()-size.height())));
}
QColor ScreenColorPicker::captureLiveColor(QScreen* screen,const QRect& geometry,const QPointF& p) {
    if(!screen||!contains(geometry,p)) return QColor();
    const QPointF local=p-QPointF(geometry.topLeft()); const int x=int(std::floor(local.x())),y=int(std::floor(local.y()));
    const QImage image=screen->grabWindow(0,x,y,1,1).toImage();
    if(image.isNull()) throw std::runtime_error(tr("Live screen capture failed. Check screen recording permissions.").toStdString());
    return image.pixelColor(qMin(image.width()-1,int((local.x()-x)*image.width())),qMin(image.height()-1,int((local.y()-y)*image.height())));
}
void ScreenColorPicker::prepareLiveOverlay(QWidget* overlay) {
#ifdef Q_OS_WIN
    if(QGuiApplication::platformName()=="windows"&&QOperatingSystemVersion::current().microVersion()>=19041) {
        using Affinity=BOOL (WINAPI*)(HWND,DWORD);
        const auto affinity=reinterpret_cast<Affinity>(GetProcAddress(GetModuleHandleW(L"user32.dll"),"SetWindowDisplayAffinity"));
        if(!affinity) throw std::runtime_error(tr("Live screen capture exclusion is unavailable. Enable screen freezing instead.").toStdString());
        if(!affinity(reinterpret_cast<HWND>(overlay->winId()),0x11))
            throw std::runtime_error(tr("Cannot exclude the live picker overlay from screen capture (Windows error %1). Enable screen freezing instead.").arg(GetLastError()).toStdString());
        return;
    }
#else
    Q_UNUSED(overlay)
#endif
    throw std::runtime_error(tr("Live screen picking requires Windows 10 version 2004 or newer; enable screen freezing on this platform.").toStdString());
}
bool ScreenColorPicker::startPicking() {
    if(!isEnabled()||picking_) return false;
    if(activePicker&&activePicker!=this) { emit errorOccurred(tr("Another screen color picker is already active.")); return false; }
    QList<ScreenSnapshot> snapshots;
    try {
        snapshots=captureScreens();
        if(snapshots.isEmpty()) throw std::runtime_error(tr("Screen capture is unavailable.").toStdString());
        for(const auto& snapshot:snapshots) if(snapshot.image.isNull()||snapshot.geometry.isEmpty())
            throw std::runtime_error(tr("Screen capture is unavailable.").toStdString());
    } catch(const std::exception& error) { emit errorOccurred(QString::fromUtf8(error.what())); return false; }
    previousFocus_=QApplication::focusWidget(); hoverColor_=QColor(); hasPosition_=false; activePicker=this;
    try {
        for(const auto& snapshot:snapshots) overlays_<<new ScreenPickerOverlay(this,snapshot);
        if(!freeze_) for(auto* overlay:overlays_) prepareLiveOverlay(overlay);
        picking_=true;
        for(auto* overlay:overlays_) overlay->show();
        for(const auto& snapshot:snapshots) if(snapshot.screen) {
            topologyConnections_<<connect(snapshot.screen,&QScreen::geometryChanged,this,&ScreenColorPicker::cancelPicking);
            topologyConnections_<<connect(snapshot.screen,&QScreen::logicalDotsPerInchChanged,this,&ScreenColorPicker::cancelPicking);
        }
        topologyConnections_<<connect(qApp,&QGuiApplication::screenRemoved,this,&ScreenColorPicker::cancelPicking);
        topologyConnections_<<connect(qApp,&QGuiApplication::screenAdded,this,&ScreenColorPicker::cancelPicking);
        qApp->installEventFilter(this);
        auto* active=overlayAt(QCursor::pos()); if(!active) active=overlays_.first();
        active->raise(); active->activateWindow(); active->setFocus(Qt::OtherFocusReason);
    } catch(const std::exception& error) { cleanup(); emit errorOccurred(QString::fromUtf8(error.what())); return false; }
    QPointer<ScreenColorPicker> guard(this); emit pickingChanged(true);
    if(guard&&picking_) { updateFromCursor(); if(guard&&picking_) timer_.start(); }
    return true;
}
ScreenPickerOverlay* ScreenColorPicker::overlayAt(const QPointF& p) const {
    for(auto* overlay:overlays_) if(contains(overlay->geometry,p)) return overlay;
    return nullptr;
}
QColor ScreenColorPicker::readColor(ScreenPickerOverlay* overlay,const QPointF& p) {
    if(!overlay) return QColor();
    try { return overlay->colorAt(p); }
    catch(const std::exception& error) {
        const QString message=QString::fromUtf8(error.what()); cleanup();
        QPointer<ScreenColorPicker> guard(this); emit pickingChanged(false);
        if(guard) emit errorOccurred(message); return QColor();
    }
}
void ScreenColorPicker::updateFromCursor() {
    if(!picking_) return;
    const QPoint p=QCursor::pos(); if(!hasPosition_||lastPosition_.toPoint()!=p) previewAt(p);
    else if(!freeze_) previewAt(lastPosition_);
}
void ScreenColorPicker::previewAt(const QPointF& p) {
    if(!picking_||(freeze_&&hasPosition_&&p==lastPosition_)) return;
    lastPosition_=p; hasPosition_=true; auto* active=overlayAt(p);
    for(auto* overlay:overlays_) if(overlay!=active) overlay->preview->hide();
    if(!active) return;
    const QColor color=readColor(active,p); if(!color.isValid()) return;
    active->preview->setColor(color);
    active->preview->move(previewPosition((p-QPointF(active->geometry.topLeft())).toPoint(),active->rect(),active->preview->size()));
    active->preview->show(); if(color!=hoverColor_) { hoverColor_=color; emit colorHovered(color); }
}
void ScreenColorPicker::confirmAt(const QPointF& p) {
    if(!picking_) return;
    const QColor color=readColor(overlayAt(p),p); if(!color.isValid()) return;
    cleanup(); QPointer<ScreenColorPicker> guard(this); emit pickingChanged(false);
    if(!guard) return; setColor(color); if(guard) emit colorPicked(color);
}
void ScreenColorPicker::cancelPicking() {
    if(!picking_) return; cleanup(); QPointer<ScreenColorPicker> guard(this);
    emit pickingChanged(false); if(guard) emit pickingCanceled();
}
void ScreenColorPicker::cleanup() {
    picking_=false; timer_.stop(); if(qApp) qApp->removeEventFilter(this);
    for(const auto& connection:topologyConnections_) disconnect(connection); topologyConnections_.clear();
    const auto overlays=overlays_; overlays_.clear(); for(auto* overlay:overlays) overlay->dispose();
    hasPosition_=false; hoverColor_=QColor(); if(activePicker==this) activePicker=nullptr;
    auto focus=previousFocus_; previousFocus_=nullptr;
    if(qApp&&focus&&QGuiApplication::applicationState()==Qt::ApplicationActive&&focus->isVisible()&&focus->isEnabled()) focus->setFocus(Qt::OtherFocusReason);
}
bool ScreenColorPicker::eventFilter(QObject* watched,QEvent* event) {
    if(picking_) {
        if(event->type()==QEvent::KeyPress) {
            auto* key=static_cast<QKeyEvent*>(event);
            if(key->key()==Qt::Key_Escape) { cancelPicking(); return true; }
            if(key->key()==Qt::Key_Return||key->key()==Qt::Key_Enter||key->key()==Qt::Key_Space) {
                QPointer<ScreenColorPicker> guard(this); updateFromCursor();
                if(guard&&picking_) confirmAt(hasPosition_?lastPosition_:QPointF(QCursor::pos())); return true;
            }
        } else if(event->type()==QEvent::ApplicationDeactivate) { cancelPicking(); return false; }
    }
    return FluentPushButton::eventFilter(watched,event);
}
QSize ScreenColorPicker::sizeHint() const { return QSize(60,32); }
QSize ScreenColorPicker::minimumSizeHint() const { return sizeHint(); }
void ScreenColorPicker::hideEvent(QHideEvent* event) {
    QPointer<ScreenColorPicker> guard(this); cancelPicking();
    if(guard) FluentPushButton::hideEvent(event);
}
void ScreenColorPicker::changeEvent(QEvent* event) {
    QPointer<ScreenColorPicker> guard(this);
    if(event->type()==QEvent::EnabledChange&&!isEnabled()) cancelPicking();
    if(guard) FluentPushButton::changeEvent(event);
}
void ScreenColorPicker::keyPressEvent(QKeyEvent* event) {
    if(isEnabled()&&(event->key()==Qt::Key_Return||event->key()==Qt::Key_Enter)) {
        if(!event->isAutoRepeat()) click(); event->accept();
    } else FluentPushButton::keyPressEvent(event);
}
void ScreenColorPicker::paintEvent(QPaintEvent* event) {
    FluentPushButton::paintEvent(event); QPainter painter(this); painter.setRenderHint(QPainter::Antialiasing);
    if(!isEnabled()) painter.setOpacity(.3628); else if(isDown()) painter.setOpacity(.786);
    const qreal x=(width()-60)/2.0,y=(height()-32)/2.0;
    const bool rtl=layoutDirection()==Qt::RightToLeft;
    drawSwatch(painter,QRectF(x+(rtl?33:5),y+5,22,22),color_,4);
    FluentIcon(FluentIconEnum::Eyedropper).render(&painter,QRectF(x+(rtl?8:36),y+8,16,16).toRect());
}
}
