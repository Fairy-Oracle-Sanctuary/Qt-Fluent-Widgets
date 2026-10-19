#include "acrylic_menu.h"
#include <QPainter>
#include <QScreen>
#include <QApplication>
#include "common/config.h"
#include "common/style_sheet.h"
#include "common/screen.h"

namespace qfw {
AcrylicMenuActionListWidget::AcrylicMenuActionListWidget(QWidget* parent)
    : MenuActionListWidget(parent),brush_(viewport(),35) {
    setViewportMargins(0,0,0,0); setProperty("transparent",true);
    setFrameShape(QFrame::NoFrame);
    viewport()->setAutoFillBackground(false);
    QListWidget::addItem(createPlaceholderItem()); QListWidget::addItem(createPlaceholderItem());
    connect(&QConfig::instance(),&QConfig::themeChanged,this,[this] { viewport()->update(); });
}
QListWidgetItem* AcrylicMenuActionListWidget::createPlaceholderItem(int height) {
    auto* item=new QListWidgetItem;
    item->setSizeHint(QSize(1,height)); item->setFlags(Qt::NoItemFlags); return item;
}
void AcrylicMenuActionListWidget::addItem(QListWidgetItem* item) {
    MenuActionListWidget::insertItem(qMax(1,count()-1),item);
}
void AcrylicMenuActionListWidget::setItemHeight(int height) {
    const int top=item(0)->sizeHint().height(),bottom=item(count()-1)->sizeHint().height();
    MenuActionListWidget::setItemHeight(height);
    item(0)->setSizeHint(QSize(1,top)); item(count()-1)->setSizeHint(QSize(1,bottom));
    adjustSizeForMenu();
}
QPainterPath AcrylicMenuActionListWidget::clipPath() const {
    QPainterPath path;
    if(objectName()==QStringLiteral("completerListWidget")) {
        path.setFillRule(Qt::WindingFill);
        path.addRoundedRect(QRectF(viewport()->rect()).adjusted(1,1,-2.5,-2.5),8,8);
        const int y=property("dropDown").toBool()?1:height()-11;
        path.addRect(1,y,11,11); path.addRect(width()-12,y,11,11);
        return path;
    }
    path.addRoundedRect(QRectF(viewport()->rect()).adjusted(0,0,-2.5,-2.5),8,8); return path;
}
void AcrylicMenuActionListWidget::paintEvent(QPaintEvent* event) {
    {
        QPainter painter(viewport());
        painter.setRenderHints(QPainter::Antialiasing|QPainter::SmoothPixmapTransform);
        brush_.setTintColor(isDarkTheme()?QColor(32,32,32,200):QColor(255,255,255,160));
        brush_.setLuminosityColor(isDarkTheme()?QColor(0,0,0,0):QColor(255,255,255,50));
        brush_.setClipPath(clipPath()); brush_.paint(&painter);
    }
    MenuActionListWidget::paintEvent(event);
}
AcrylicMenu::AcrylicMenu(const QString& title,QWidget* parent) : AcrylicMenuAdapter(title,parent) {}
AcrylicCheckableMenu::AcrylicCheckableMenu(const QString& title,QWidget* parent,MenuIndicatorType type)
    : AcrylicMenuAdapter(title,parent,type) {}
AcrylicLineEditMenu::AcrylicLineEditMenu(QLineEdit* parent) : AcrylicMenuAdapter(parent) {}

namespace {
template<class Menu> QPoint positionTray(Menu* menu) {
    const QRect screen=getCurrentScreenGeometry(true);
    const QMargins margins=menu->layout()->contentsMargins();
    QPropertyAnimation* animation=nullptr;
    for(auto* candidate:menu->template findChildren<QPropertyAnimation*>())
        if(candidate->propertyName()=="pos" && candidate->state()==QAbstractAnimation::Running) animation=candidate;
    const QPoint anchor=animation?animation->endValue().toPoint():menu->pos();
    const int x=qBound(screen.left(),anchor.x()-margins.left(),qMax(screen.left(),screen.right()-menu->width()-5));
    int y=anchor.y();
    if(y>screen.bottom()-menu->height()) y-=menu->height()-margins.bottom();
    const QPoint position(x,qMax(screen.top(),y));
    if(animation) animation->setEndValue(position); else menu->move(position);
    return position;
}
}
void AcrylicSystemTrayMenu::showEvent(QShowEvent* event) { prepareShowBackground(positionTray(this)); AcrylicMenu::showEvent(event); }
void AcrylicCheckableSystemTrayMenu::showEvent(QShowEvent* event) { prepareShowBackground(positionTray(this)); AcrylicCheckableMenu::showEvent(event); }
}
