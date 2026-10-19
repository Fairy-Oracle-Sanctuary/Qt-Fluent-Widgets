#include "components/widgets/drop_down_color_picker.h"
#include <QAbstractAnimation>
#include <QApplication>
#include <QCloseEvent>
#include <QGraphicsDropShadowEffect>
#include <QIntValidator>
#include <QKeyEvent>
#include <QLinearGradient>
#include <QPainterPath>
#include <QRegularExpressionValidator>
#include <QScreen>
#include <QVBoxLayout>
#include <array>
#include <cmath>
#include <stdexcept>
#include "common/config.h"
#include "common/animation.h"
#include "components/widgets/combo_box.h"
#include "components/widgets/flyout.h"
#include "components/widgets/label.h"
#include "components/widgets/line_edit.h"
#include "components/widgets/scroll_area.h"
#include "components/widgets/slider.h"

namespace qfw {
namespace {
QColor checkedColor(const QColor& color) {
    if (!color.isValid()) throw std::invalid_argument("Invalid color picker color");
    return QColor::fromRgba(color.rgba());
}
class HueSaturationPanel : public QWidget {
    Q_OBJECT
public:
    explicit HueSaturationPanel(QWidget* parent) : QWidget(parent) {
        setObjectName("huePanel"); setFixedSize(288,288); setFocusPolicy(Qt::StrongFocus);
        setAccessibleName(tr("Hue and saturation")); setCursor(Qt::CrossCursor);
    }
    void setPosition(qreal hue, qreal saturation) { hue_=hue; saturation_=saturation; update(); }
signals:
    void positionChanged(qreal hue, qreal saturation);
protected:
    void mousePressEvent(QMouseEvent* event) override {
        if(event->button()!=Qt::LeftButton) { event->ignore(); return; }
        setFocus(Qt::MouseFocusReason); pick(event->localPos()); event->accept();
    }
    void mouseMoveEvent(QMouseEvent* event) override {
        if(event->buttons()&Qt::LeftButton) { pick(event->localPos()); event->accept(); }
        else event->ignore();
    }
    void keyPressEvent(QKeyEvent* event) override {
        const int dx=event->key()==Qt::Key_Left?-1:event->key()==Qt::Key_Right?1:0;
        const int dy=event->key()==Qt::Key_Up?-1:event->key()==Qt::Key_Down?1:0;
        if(dx||dy) { pick(QPointF(hue_*(width()-1)+dx,(1-saturation_)*(height()-1)+dy)); event->accept(); }
        else QWidget::keyPressEvent(event);
    }
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this); painter.setRenderHint(QPainter::Antialiasing);
        const QRectF rect=QRectF(this->rect()).adjusted(.5,.5,-.5,-.5);
        QPainterPath clip; clip.addRoundedRect(rect,5,5); painter.setClipPath(clip);
        QLinearGradient hue(0,0,width()-1,0);
        for(int i=0;i<7;++i) hue.setColorAt(i/6.0,QColor::fromHsvF(i/6.0,1,1));
        painter.fillRect(rect,hue);
        QLinearGradient saturation(0,0,0,height()-1);
        saturation.setColorAt(0,QColor(255,255,255,0)); saturation.setColorAt(1,Qt::white);
        painter.fillRect(rect,saturation); painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(QColor(0,0,0,22),1)); painter.drawRoundedRect(rect,5,5);
        painter.setPen(QPen(Qt::black,2.5));
        painter.drawEllipse(QPointF(hue_*(width()-1),(1-saturation_)*(height()-1)),8,8);
    }
private:
    void pick(const QPointF& p) {
        setPosition(qBound(qreal(0),p.x()/(width()-1),qreal(1)),1-qBound(qreal(0),p.y()/(height()-1),qreal(1)));
        emit positionChanged(hue_,saturation_);
    }
    qreal hue_=0, saturation_=1;
};
class ColorGradientSlider : public Slider {
public:
    explicit ColorGradientSlider(QWidget* parent) : Slider(Qt::Horizontal,parent) { setRange(0,255); setFixedHeight(22); }
    void setColors(const QList<QColor>& colors,const QColor& handle,bool checkerboard=false) {
        colors_=colors; checkerboard_=checkerboard;
        if(auto* h=findChild<SliderHandle*>()) h->setHandleColor(handle,handle);
        update();
    }
protected:
    void mousePressEvent(QMouseEvent* event) override {
        if(event->button()==Qt::LeftButton) { Slider::mousePressEvent(event); event->accept(); }
        else event->ignore();
    }
    void mouseMoveEvent(QMouseEvent* event) override {
        if(event->buttons()&Qt::LeftButton) { Slider::mouseMoveEvent(event); event->accept(); }
        else event->ignore();
    }
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this); painter.setRenderHint(QPainter::Antialiasing);
        QRectF rect(0,5,width(),12); QPainterPath clip; clip.addRoundedRect(rect,6,6); painter.setClipPath(clip);
        if(checkerboard_) for(int row=0;row<3;++row) for(int col=0;col<(width()+3)/4;++col)
            painter.fillRect(col*4,5+row*4,4,4,QColor((row+col)%2?"#DCDCDC":"#808080"));
        QLinearGradient gradient(11,0,qMax(12,width()-11),0);
        for(int i=0;i<colors_.size();++i) gradient.setColorAt(qreal(i)/(colors_.size()-1),colors_[i]);
        painter.fillRect(rect,gradient);
    }
private:
    QList<QColor> colors_{Qt::black,Qt::white}; bool checkerboard_=false;
};
class ColorModeCombo : public ComboBox {
public:
    using ComboBox::ComboBox;
    void openMenu() { toggleComboMenu(this); }
};
class PickerFlyout : public Flyout {
public:
    PickerFlyout(FlyoutViewBase* view,QWidget* parent) : Flyout(view,parent) {}
protected:
    bool eventFilter(QObject* watched,QEvent* event) override {
        // Nested combo and editor menus may extend past the picker rectangle.
        // Let their own popup close/activation rules handle those clicks.
        if(event->type()==QEvent::MouseButtonPress) {
            auto* widget=qobject_cast<QWidget*>(watched);
            if(widget&&isAncestorOf(widget)) return QWidget::eventFilter(watched,event);
        }
        return Flyout::eventFilter(watched,event);
    }
    void closeEvent(QCloseEvent* event) override {
        if(closing_) { event->accept(); return; }
        closing_=true;
        for(auto* animation:findChildren<QAbstractAnimation*>()) animation->stop();
        Flyout::closeEvent(event);
    }
    void hideEvent(QHideEvent* event) override {
        QWidget::hideEvent(event); if(!closing_) close();
    }
    void keyPressEvent(QKeyEvent* event) override {
        if(event->key()==Qt::Key_Escape) { close(); event->accept(); }
        else Flyout::keyPressEvent(event);
    }
private:
    bool closing_=false;
};
}

class ColorPickerView : public FlyoutViewBase {
    Q_OBJECT
public:
    ColorPickerView(const QColor& color,bool alpha) : FlyoutViewBase(nullptr),alpha_(alpha) {
        setObjectName("colorPickerView"); setFixedWidth(320);
        auto* body=new QWidget(this); body->setFixedSize(318,alpha?566:526);
        auto* scroll=new ScrollArea(this); scroll->setFrameShape(QFrame::NoFrame); scroll->setWidget(body);
        scroll->setWidgetResizable(false); scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scroll->enableTransparentBackground();
        panel=new HueSaturationPanel(body); panel->move(15,15);
        brightness=new ColorGradientSlider(body); brightness->setObjectName("brightnessSlider");
        brightness->setGeometry(15,319,288,22); brightness->setAccessibleName(tr("Brightness"));
        mode=new ColorModeCombo(body); mode->setObjectName("colorMode"); mode->addItems({"RGB","HSV"});
        mode->setGeometry(15,351,92,32); mode->setAccessibleName(tr("Color model"));
        hex=new LineEdit(body); hex->setObjectName("hexEdit"); hex->setGeometry(167,351,136,32);
        hex->setAccessibleName(tr("Hexadecimal color, AARRGGBB or RRGGBB"));
        hex->setValidator(new QRegularExpressionValidator(QRegularExpression("#?(?:[0-9a-fA-F]{6}|[0-9a-fA-F]{8})"),hex));
        for(int i=0;i<4;++i) {
            const int y=395+i*40;
            labels[i]=new BodyLabel(QString("RGBA").mid(i,1),body); labels[i]->setGeometry(15,y,16,32);
            edits[i]=new LineEdit(body); edits[i]->setObjectName(QString("channelEdit%1").arg(i));
            edits[i]->setGeometry(31,y,64,32); edits[i]->setValidator(new QIntValidator(0,255,edits[i]));
            sliders[i]=new ColorGradientSlider(body); sliders[i]->setObjectName(QString("channelSlider%1").arg(i));
            sliders[i]->setGeometry(103,y+5,200,22);
            connect(edits[i],&QLineEdit::textEdited,this,[this,i](const QString& text) {
                if(edits[i]->hasAcceptableInput()) { edits[i]->setError(false); channelChanged(i,text.toInt(),edits[i]); }
            });
            connect(edits[i],&QLineEdit::editingFinished,this,[this,i] { edits[i]->setText(QString::number(channelValues()[i])); });
            connect(sliders[i],&QSlider::valueChanged,this,[this,i](int value) { channelChanged(i,value); });
            connect(edits[i],&QLineEdit::returnPressed,this,&ColorPickerView::confirm);
            if(i==3&&!alpha) { labels[i]->hide(); edits[i]->hide(); sliders[i]->hide(); }
        }
        bar=new QWidget(this); bar->setFixedHeight(40);
        auto* footer=new QHBoxLayout(bar); footer->setContentsMargins(4,4,4,4); footer->setSpacing(0);
        auto* yes=new TransparentToolButton(FluentIcon(FluentIconEnum::Accept),bar);
        auto* cancel=new TransparentToolButton(FluentIcon(FluentIconEnum::Close),bar);
        yes->setObjectName("confirmColor"); cancel->setObjectName("cancelColor");
        yes->setAccessibleName(tr("Confirm")); cancel->setAccessibleName(tr("Cancel"));
        for(auto* button:{yes,cancel}) { button->setIconSize(QSize(16,16)); button->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding); footer->addWidget(button,1); }
        auto* layout=new QVBoxLayout(this); layout->setContentsMargins(1,1,1,1); layout->setSpacing(0);
        layout->addWidget(scroll,1); layout->addWidget(bar); setFixedHeight(alpha?608:568);
        connect(panel,&HueSaturationPanel::positionChanged,this,[this](qreal h,qreal s) { hue_=h; saturation_=s; setDraft(hsvColor(),true); });
        connect(brightness,&QSlider::valueChanged,this,[this](int v) { if(!updating_) { value_=v/255.0; setDraft(hsvColor(),true); } });
        connect(mode,&ComboBox::currentIndexChanged,this,[this] { sync(); });
        connect(hex,&QLineEdit::textEdited,this,[this](const QString& text) {
            if(!hex->hasAcceptableInput()) return;
            QString digits=text; if(digits.startsWith('#')) digits.remove(0,1);
            QColor color("#"+digits); if(digits.size()==6) color.setAlpha(color_.alpha());
            hex->setError(false); setDraft(color,false,hex);
        });
        connect(hex,&QLineEdit::editingFinished,this,[this] { restoreHex(); });
        connect(hex,&QLineEdit::returnPressed,this,&ColorPickerView::confirm);
        connect(yes,&QToolButton::clicked,this,&ColorPickerView::confirm);
        connect(cancel,&QToolButton::clicked,this,&ColorPickerView::cancelled);
        QList<QWidget*> controls{panel,brightness,mode,hex,yes,cancel};
        for(int i=0;i<4;++i) { controls<<edits[i]<<sliders[i]; }
        for(auto* widget:controls) widget->installEventFilter(this);
        setColor(color);
    }
    void addWidget(QWidget*,int,Qt::Alignment) override {}
    void setColor(const QColor& color) { setDraft(checkedColor(color),false,nullptr,false); }
    HueSaturationPanel* panel;
signals:
    void confirmed(const QColor& color);
    void cancelled();
    void previewed(const QColor& color);
protected:
    bool eventFilter(QObject* watched,QEvent* event) override {
        if(event->type()==QEvent::KeyPress) {
            auto* key=static_cast<QKeyEvent*>(event);
            if(key->key()==Qt::Key_Escape) { emit cancelled(); return true; }
            if(watched==mode&&(key->key()==Qt::Key_Return||key->key()==Qt::Key_Enter||key->key()==Qt::Key_Space||key->key()==Qt::Key_Down)) { mode->openMenu(); return true; }
        }
        return FlyoutViewBase::eventFilter(watched,event);
    }
    void paintEvent(QPaintEvent* event) override {
        FlyoutViewBase::paintEvent(event); QPainter painter(this);
        painter.fillRect(1,bar->y(),width()-2,1,isDarkTheme()?QColor(255,255,255,15):QColor(0,0,0,17));
    }
private:
    QColor hsvColor() const { return QColor::fromHsvF(hue_,saturation_,value_,color_.alphaF()); }
    std::array<int,4> channelValues() const {
        if(mode->currentIndex()==0) return {{color_.red(),color_.green(),color_.blue(),color_.alpha()}};
        return {{qMin(359,int(std::round(hue_*360))),int(std::round(saturation_*100)),int(std::round(value_*100)),color_.alpha()}};
    }
    void setDraft(QColor color,bool preserve=false,LineEdit* exclude=nullptr,bool preview=true) {
        color=QColor::fromRgba(color.rgba()); if(!alpha_) color.setAlpha(255);
        const bool changed=color_!=color; color_=color;
        if(!preserve) { if(color.hsvHueF()>=0) hue_=color.hsvHueF(); saturation_=color.saturationF(); value_=color.valueF(); }
        sync(exclude); if(changed&&preview) emit previewed(color_);
    }
    void channelChanged(int index,int value,LineEdit* exclude=nullptr) {
        if(updating_) return;
        QColor color=color_; bool preserve=false;
        if(index==3) { color.setAlpha(value); preserve=true; }
        else if(mode->currentIndex()==0) { if(index==0) color.setRed(value); else if(index==1) color.setGreen(value); else color.setBlue(value); }
        else { if(index==0) hue_=value/360.0; else if(index==1) saturation_=value/100.0; else value_=value/100.0; color=hsvColor(); preserve=true; }
        setDraft(color,preserve,exclude);
    }
    void restoreHex() { hex->setText(color_.name(alpha_?QColor::HexArgb:QColor::HexRgb)); hex->setError(false); }
    void sync(LineEdit* exclude=nullptr) {
        updating_=true; panel->setPosition(hue_,saturation_); brightness->setValue(int(std::round(value_*255)));
        const QColor full=QColor::fromHsvF(hue_,saturation_,1); QColor handle=color_; handle.setAlpha(255);
        brightness->setColors({Qt::black,full},handle);
        const bool rgb=mode->currentIndex()==0; const auto values=channelValues();
        for(int i=0;i<4;++i) {
            const int maximum=rgb||i==3?255:i==0?359:100;
            const QString name=QString(rgb?"RGBA":"HSVA").mid(i,1); labels[i]->setText(name);
            edits[i]->setAccessibleName(tr("%1 value").arg(name)); sliders[i]->setAccessibleName(tr("%1 slider").arg(name));
            static_cast<QIntValidator*>(const_cast<QValidator*>(edits[i]->validator()))->setRange(0,maximum);
            if(edits[i]!=exclude) edits[i]->setText(QString::number(values[i]));
            sliders[i]->setRange(0,maximum); sliders[i]->setValue(values[i]);
            QList<QColor> stops; QColor low=handle,high=handle;
            if(i==3) { low.setAlpha(0); stops={low,high}; }
            else if(rgb) { if(i==0) { low.setRed(0);high.setRed(255); } else if(i==1) {low.setGreen(0);high.setGreen(255);} else {low.setBlue(0);high.setBlue(255);} stops={low,high}; }
            else if(i==0) { for(int j=0;j<7;++j) stops<<QColor::fromHsvF(j/6.0,saturation_,value_); }
            else if(i==1) stops={QColor::fromHsvF(hue_,0,value_),QColor::fromHsvF(hue_,1,value_)};
            else stops={Qt::black,full};
            sliders[i]->setColors(stops,handle,i==3);
        }
        if(hex!=exclude) restoreHex(); updating_=false;
    }
    void confirm() {
        QList<LineEdit*> fields{hex}; for(int i=0;i<(alpha_?4:3);++i) fields<<edits[i];
        for(auto* field:fields) if(!field->hasAcceptableInput()) { field->setError(true);field->setFocus();return; }
        emit confirmed(color_);
    }
    bool alpha_,updating_=false;
    qreal hue_=0,saturation_=0,value_=0;
    QColor color_=Qt::black;
    ColorModeCombo* mode; LineEdit* hex; ColorGradientSlider* brightness; QWidget* bar;
    std::array<BodyLabel*,4> labels; std::array<LineEdit*,4> edits; std::array<ColorGradientSlider*,4> sliders;
};

DropDownColorPicker::DropDownColorPicker(QWidget* parent) : DropDownColorPicker(QColor("#0078D4"),parent) {}
DropDownColorPicker::DropDownColorPicker(const QColor& color,QWidget* parent,bool alpha)
    : DropDownPushButton(parent),color_(checkedColor(color)),alphaEnabled_(alpha) {
    if(!alpha) color_.setAlpha(255);
    setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed); setAccessibleName(tr("Color picker"));
    connect(this,&QPushButton::clicked,this,&DropDownColorPicker::showPicker);
    connect(&QConfig::instance(),&QConfig::themeChangedFinished,this,[this] {
        update(); if(view_) { view_->update(); for(auto* child:view_->findChildren<QWidget*>()) child->update(); }
        if(view_) if(auto* shadow=qobject_cast<QGraphicsDropShadowEffect*>(view_->graphicsEffect()))
            shadow->setColor(QColor(0,0,0,isDarkTheme()?80:30));
    });
}
DropDownColorPicker::~DropDownColorPicker() { blockSignals(true); if(popup_) delete popup_.data(); }
QColor DropDownColorPicker::color() const { return color_; }
void DropDownColorPicker::setColor(const QColor& value) {
    QColor color=checkedColor(value); if(!alphaEnabled_) color.setAlpha(255);
    const bool changed=color_!=color; color_=color; update(); if(view_) view_->setColor(color);
    if(changed) emit colorChanged(color_);
}
void DropDownColorPicker::setAlphaEnabled(bool enabled) {
    if(enabled==alphaEnabled_) return;
    QPointer<DropDownColorPicker> guard(this); closePicker(); if(!guard) return;
    alphaEnabled_=enabled; setColor(color_);
}
bool DropDownColorPicker::isAlphaEnabled() const { return alphaEnabled_; }
bool DropDownColorPicker::isPickerVisible() const { return popup_&&popup_->isVisible(); }
void DropDownColorPicker::showPicker() {
    if(!isEnabled()) return;
    if(popup_) { closePicker(); return; }
    confirmed_=false; auto* view=new ColorPickerView(color_,alphaEnabled_); view_=view;
    auto* popup=new PickerFlyout(view,window()); popup_=popup;
    connect(popup,&Flyout::closed,this,[this] {
        popup_=nullptr; view_=nullptr; setDown(false);
        if(isVisible()&&isEnabled()) setFocus(Qt::PopupFocusReason);
        if(!confirmed_) emit pickingCanceled();
    });
    connect(view,&ColorPickerView::confirmed,this,[this](const QColor& color) {
        confirmed_=true; QPointer<DropDownColorPicker> guard(this); closePicker(); if(!guard) return;
        setColor(color); if(guard) emit colorSelected(color_);
    });
    connect(view,&ColorPickerView::cancelled,this,&DropDownColorPicker::closePicker);
    connect(view,&ColorPickerView::previewed,this,&DropDownColorPicker::colorPreviewed);
    auto* screen=this->screen(); const int screenHeight=screen?screen->availableGeometry().height():1080;
    view->setFixedHeight(qMin(view->height(),qMax(180,screenHeight-36))); popup->adjustSize();
    popup->exec(mapToGlobal(QPoint(width()/2-popup->sizeHint().width()/2,height())),FlyoutAnimationType::DropDown);
    view->panel->setFocus(Qt::PopupFocusReason);
}
void DropDownColorPicker::closePicker() { if(popup_) popup_->close(); }
QSize DropDownColorPicker::sizeHint() const { return QSize(54,32); }
QSize DropDownColorPicker::minimumSizeHint() const { return sizeHint(); }
void DropDownColorPicker::mouseReleaseEvent(QMouseEvent* event) { FluentPushButton::mouseReleaseEvent(event); }
void DropDownColorPicker::keyPressEvent(QKeyEvent* event) {
    if(isEnabled()&&(event->key()==Qt::Key_Return||event->key()==Qt::Key_Enter||event->key()==Qt::Key_Down)) {
        if(!event->isAutoRepeat()) showPicker(); event->accept();
    } else DropDownPushButton::keyPressEvent(event);
}
void DropDownColorPicker::hideEvent(QHideEvent* event) {
    QPointer<DropDownColorPicker> guard(this); closePicker();
    if(guard) DropDownPushButton::hideEvent(event);
}
void DropDownColorPicker::changeEvent(QEvent* event) {
    QPointer<DropDownColorPicker> guard(this);
    if(event->type()==QEvent::EnabledChange&&!isEnabled()) closePicker();
    if(guard) DropDownPushButton::changeEvent(event);
}
void DropDownColorPicker::paintEvent(QPaintEvent* event) {
    FluentPushButton::paintEvent(event); QPainter painter(this); painter.setRenderHint(QPainter::Antialiasing);
    if(!isEnabled()) painter.setOpacity(.3628); else if(isDown()) painter.setOpacity(.786);
    const QRectF rect(layoutDirection()==Qt::RightToLeft?width()-26:4,(height()-22)/2.0,22,22);
    QPainterPath clip; clip.addRoundedRect(rect,4,4); painter.setClipPath(clip);
    if(color_.alpha()<255) for(int row=0;row<7;++row) for(int col=0;col<7;++col)
        painter.fillRect(QRectF(rect.x()+col*4,rect.y()+row*4,4,4),QColor(isDarkTheme()?((row+col)%2?"#777777":"#555555"):((row+col)%2?"#DDDDDD":"#FFFFFF")));
    painter.fillPath(clip,color_);
    painter.setClipping(false);
    painter.setOpacity(!isEnabled()?.3628:isDown()?.7:underMouse()?.8:1);
    auto* animation=findChild<TranslateYAnimation*>();
    const QRect arrowRect=QRectF(layoutDirection()==Qt::RightToLeft?12:width()-22,
                                 height()/2.0-5+(animation?animation->getY():0),10,10).toRect();
    FluentIcon(FluentIconEnum::ChevronDown).render(&painter,arrowRect,Theme::Auto,
        isDarkTheme()?QVariantMap():QVariantMap{{"fill","#646464"},{"stroke","#646464"}});
}
}
#include "drop_down_color_picker.moc"
