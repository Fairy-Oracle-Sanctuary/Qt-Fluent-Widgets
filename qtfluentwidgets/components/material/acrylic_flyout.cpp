#include "acrylic_flyout.h"
#include <QPainter>
#include "components/widgets/label.h"
#include <memory>
#include "common/config.h"
#include "common/style_sheet.h"

namespace qfw {
namespace {
QPainterPath viewPath(const QWidget* widget) {
    QPainterPath path;
    path.addRoundedRect(QRectF(widget->rect().adjusted(1,1,-1,-1)),8,8);
    return path;
}
void updateColors(AcrylicBrush& brush) {
    brush.setTintColor(isDarkTheme()?QColor(32,32,32,200):QColor(255,255,255,180));
    brush.setLuminosityColor(isDarkTheme()?QColor(0,0,0,0):QColor(255,255,255,0));
}
void paintSurface(QWidget* widget, AcrylicBrush& brush, const QPainterPath& clip, const QColor& border) {
    QPainter painter(widget); painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    updateColors(brush); brush.setClipPath(clip); brush.paint(&painter);
    painter.setBrush(Qt::NoBrush); painter.setPen(border);
    painter.drawRoundedRect(QRectF(widget->rect().adjusted(1,1,-1,-1)),8,8);
}
void observeTheme(QWidget* widget) {
    QObject::connect(&QConfig::instance(), &QConfig::themeChanged, widget, [widget] { widget->update(); });
}
}
AcrylicFlyoutViewBase::AcrylicFlyoutViewBase(QWidget* parent) : FlyoutViewBase(parent), brush_(this,30) {
    updateColors(brush_); observeTheme(this);
}
QPainterPath AcrylicFlyoutViewBase::acrylicClipPath() const { return viewPath(this); }
void AcrylicFlyoutViewBase::paintEvent(QPaintEvent*) { paintSurface(this,brush_,acrylicClipPath(),borderColor()); }
AcrylicFlyoutView::AcrylicFlyoutView(const QString& title,const QString& content,QWidget* parent)
    : FlyoutView(title,content,parent),brush_(this,30) { updateColors(brush_); observeTheme(this); }
AcrylicFlyoutView::AcrylicFlyoutView(const QString& title,const QString& content,const QIcon& icon,QWidget* parent)
    : AcrylicFlyoutView(title,content,parent) { setIcon(icon); }
AcrylicFlyoutView::AcrylicFlyoutView(const QString& title,const QString& content,const FluentIconBase& icon,QWidget* parent)
    : AcrylicFlyoutView(title,content,toQIcon(icon),parent) {}
AcrylicFlyoutView::AcrylicFlyoutView(const QString& title,const QString& content,const QString& image,QWidget* parent)
    : AcrylicFlyoutView(title,content,parent) { setImage(image); }
QPainterPath AcrylicFlyoutView::acrylicClipPath() const { return viewPath(this); }
void AcrylicFlyoutView::paintEvent(QPaintEvent*) { paintSurface(this,brush_,acrylicClipPath(),borderColor()); }
void AcrylicFlyoutView::adjustImage() {
    auto* image=findChild<ImageLabel*>();
    if (!image || image->isNull() || !layout()) return;
    auto* row=layout()->itemAt(layout()->count()-1)->layout();
    if (row) image->scaledToWidth(row->sizeHint().width());
}
AcrylicFlyout::AcrylicFlyout(FlyoutViewBase* view,QWidget* parent,bool deleteOnClose)
    : Flyout(view,parent,deleteOnClose),acrylicView_(view) {}
void AcrylicFlyout::exec(const QPoint& pos,FlyoutAnimationType type) {
    if (acrylicView_) acrylicView_->adjustSize();
    adjustSize();
    if (layout()) layout()->activate();
    Flyout::exec(pos,type);
}
void AcrylicFlyout::prepareShowBackground(const QPoint& pos) {
    if (!acrylicView_) return;
    const QRect rect(pos+acrylicView_->pos(),acrylicView_->size());
    if (auto* view=qobject_cast<AcrylicFlyoutView*>(acrylicView_)) captureBackground(view->acrylicBrush(),rect);
    else if (auto* view=qobject_cast<AcrylicFlyoutViewBase*>(acrylicView_)) captureBackground(view->acrylicBrush(),rect);
}
void AcrylicFlyout::captureBackground(AcrylicBrush& brush,const QRect& rect) { brush.grabImage(rect); }
AcrylicFlyout* AcrylicFlyout::make(FlyoutViewBase* view,const QVariant& target,QWidget* parent,
                                  FlyoutAnimationType type,bool deleteOnClose) {
    auto* flyout=new AcrylicFlyout(view,parent,deleteOnClose);
    if (!target.isValid()) return flyout;
    QPoint position;
    if (target.canConvert<QPoint>()) position=target.toPoint();
    else if (target.canConvert<QWidget*>()) {
        if (auto* widget=target.value<QWidget*>()) {
            std::unique_ptr<FlyoutAnimationManager> manager(FlyoutAnimationManager::make(type,flyout));
            position=manager->position(widget);
        }
    }
    // Do not show before exec: the backdrop must be sampled before the popup covers it.
    flyout->exec(position,type); return flyout;
}
AcrylicFlyout* AcrylicFlyout::create(const QString& title,const QString& content,const QVariant& icon,
                                    const QVariant& image,bool closable,const QVariant& target,QWidget* parent,
                                    FlyoutAnimationType type,bool deleteOnClose) {
    auto* view=new AcrylicFlyoutView(title,content);
    view->setClosable(closable);
    if (icon.canConvert<QIcon>()) view->setIcon(icon.value<QIcon>());
    else if (icon.canConvert<QString>()) view->setIcon(QIcon(icon.toString()));
    if (image.canConvert<QPixmap>()) view->setImage(image.value<QPixmap>());
    else if (image.canConvert<QImage>()) view->setImage(image.value<QImage>());
    else if (image.canConvert<QString>()) view->setImage(image.toString());
    auto* flyout=make(view,target,parent,type,deleteOnClose);
    QObject::connect(view,&FlyoutView::closed,flyout,&QWidget::close);
    return flyout;
}
}
