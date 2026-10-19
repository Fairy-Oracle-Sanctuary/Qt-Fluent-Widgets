#include "chat_widget.h"
#include "button.h"
#include "extended_button.h"
#include "label.h"
#include "scroll_area.h"
#include "components/layout/flow_layout.h"
#include "common/config.h"
#include "common/font.h"
#include "common/style_sheet.h"
#include <QApplication>
#include <QClipboard>
#include <QInputMethodEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QEnterEvent>
#include <QLabel>
#include <QPainter>
#include <QPlainTextEdit>
#include <QPointer>
#include <QScrollBar>
#include <QSplitter>
#include <QUuid>
#include <QVBoxLayout>
#include <QtMath>
#include <stdexcept>

namespace qfw {
namespace {
class ChatInput : public QPlainTextEdit {
public:
    explicit ChatInput(ChatWidget* chat,QWidget* parent):QPlainTextEdit(parent),chat_(chat){setFrameShape(QFrame::NoFrame);setFont(getFont(14));}
    bool sendOnEnter=true;
protected:
    void inputMethodEvent(QInputMethodEvent* e) override{preediting_=!e->preeditString().isEmpty();QPlainTextEdit::inputMethodEvent(e);}
    void keyPressEvent(QKeyEvent* e) override {
        if((e->key()==Qt::Key_Return||e->key()==Qt::Key_Enter)&&!preediting_&&e->modifiers()==(sendOnEnter?Qt::NoModifier:Qt::ControlModifier)){chat_->send();e->accept();return;}
        QPlainTextEdit::keyPressEvent(e);
    }
private:ChatWidget* chat_;bool preediting_=false;
};
class ChatDivider : public QSplitterHandle {
public:ChatDivider(Qt::Orientation o,QSplitter* p):QSplitterHandle(o,p){setCursor(Qt::SplitVCursor);}
protected:
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
    void enterEvent(QEnterEvent* e) override{hover_=true;update();QSplitterHandle::enterEvent(e);}
#else
    void enterEvent(QEvent* e) override{hover_=true;update();QSplitterHandle::enterEvent(e);}
#endif
    void leaveEvent(QEvent* e) override{hover_=false;update();QSplitterHandle::leaveEvent(e);}
    void mousePressEvent(QMouseEvent* e) override{if(e->button()==Qt::LeftButton)pressed_=true;update();QSplitterHandle::mousePressEvent(e);}
    void mouseReleaseEvent(QMouseEvent* e) override{pressed_=false;update();QSplitterHandle::mouseReleaseEvent(e);}
    void paintEvent(QPaintEvent*) override{QPainter p(this);p.fillRect(rect(),isDarkTheme()?QColor("#202020"):QColor("#ffffff"));p.setPen(QPen(hover_||pressed_?themeColor():isDarkTheme()?QColor("#383838"):QColor("#e5e5e5"),1));p.drawLine(0,height()/2,width()-1,height()/2);}
private:bool hover_=false,pressed_=false;
};
class ChatSplitter : public QSplitter {
public:explicit ChatSplitter(QWidget* p):QSplitter(Qt::Vertical,p){}
protected:QSplitterHandle* createHandle() override{return new ChatDivider(orientation(),this);}
};
class ChatJumpButton : public RoundToolButton {
public:explicit ChatJumpButton(QWidget* p):RoundToolButton(FluentIcon(FluentIconEnum::Down),p){}
protected:void paintEvent(QPaintEvent* e) override{{QPainter p(this);p.setRenderHint(QPainter::Antialiasing);p.setPen(Qt::NoPen);p.setBrush(isDarkTheme()?QColor("#202020"):QColor("#ffffff"));p.drawEllipse(QRectF(rect()));}RoundToolButton::paintEvent(e);}
};
QIcon thumbIcon(bool down) {
    QPixmap pixmap(32,32);pixmap.fill(Qt::transparent);QPainter p(&pixmap);p.setRenderHint(QPainter::Antialiasing);p.scale(2,2);if(down){p.translate(0,16);p.scale(1,-1);}
    QPen pen(isDarkTheme()?Qt::white:Qt::black,1.2);pen.setCapStyle(Qt::RoundCap);pen.setJoinStyle(Qt::RoundJoin);p.setPen(pen);p.setBrush(Qt::NoBrush);p.drawRoundedRect(QRectF(1,7,3,8),.7,.7);
    QPainterPath path;path.moveTo(4,7);path.lineTo(7.5,3);path.cubicTo(7.5,.5,10.5,.5,10.5,3);path.lineTo(10,6.5);path.lineTo(13.5,6.5);path.quadTo(15.5,6.5,15,8.5);path.lineTo(13.7,13.8);path.quadTo(13.4,15,12,15);path.lineTo(4,15);path.closeSubpath();p.drawPath(path);p.end();return QIcon(pixmap);
}
class ChatBubble : public QWidget {
public:ChatBubble(std::function<QColor()> color,QWidget* p):QWidget(p),color_(std::move(color)){contentLayout=new QVBoxLayout(this);contentLayout->setContentsMargins(14,10,14,10);contentLayout->setSpacing(10);}
    QVBoxLayout* contentLayout;
protected:void paintEvent(QPaintEvent*) override{QPainter p(this);p.setRenderHint(QPainter::Antialiasing);p.setPen(Qt::NoPen);p.setBrush(color_());p.drawRoundedRect(QRectF(rect()),12,12);}
private:std::function<QColor()> color_;
};
}
class ChatRow : public QWidget {
public:
    ChatRow(ChatWidget* c,const ChatMessage& m,const QImage& avatarImage,const QImage& image,QWidget* widget):QWidget(c->messageContainer),chat(c),message(m),image(image) {
        outgoing=m.role=="user";auto* layout=new QHBoxLayout(this);layout->setContentsMargins(0,0,0,0);layout->setSpacing(10);
        avatar=new AvatarWidget(this);avatar->setRadius(18);if(!avatarImage.isNull())avatar->setImage(avatarImage);avatar->setText(m.name.isEmpty()?(outgoing?"U":"A"):m.name);
        body=new QWidget(this);auto* bodyLayout=new QVBoxLayout(body);bodyLayout->setContentsMargins(0,0,0,0);bodyLayout->setSpacing(6);
        nameLabel=new QLabel(m.name,body);nameLabel->setWordWrap(true);nameLabel->setVisible(!m.name.isEmpty());bodyLayout->addWidget(nameLabel);
        bubble=new ChatBubble([this]{return chat->bubbleColor(outgoing);},body);const auto alignment=outgoing?Qt::AlignRight:Qt::AlignLeft;bodyLayout->addWidget(bubble,0,alignment);
        if(!image.isNull()){imageLabel=new QLabel(bubble);imageLabel->setAlignment(Qt::AlignCenter);imageLabel->setCursor(Qt::PointingHandCursor);imageLabel->installEventFilter(this);bubble->contentLayout->addWidget(imageLabel);}
        else if(widget)bubble->contentLayout->addWidget(widget);
        statusLabel=new QLabel(body);bodyLayout->addWidget(statusLabel);statusLabel->setVisible(m.streaming);
        toolbar=new QWidget(body);flow=new FlowLayout(toolbar);flow->setContentsMargins(0,0,0,0);flow->setHorizontalSpacing(4);flow->setVerticalSpacing(4);syncActions();bodyLayout->addWidget(toolbar,0,alignment);
        if(outgoing){layout->addStretch();layout->addWidget(body,0,Qt::AlignTop);layout->addWidget(avatar,0,Qt::AlignTop);}
        else{layout->addWidget(avatar,0,Qt::AlignTop);layout->addWidget(body,0,Qt::AlignTop);layout->addStretch();}
        nameLabel->setAlignment(alignment);applyTheme(false);
    }
    void setContent(bool force=false) {
        if(message.kind!="text")return;
        QPointer<ChatRow> guard(this);const auto revision=++contentRevision;
        const auto parsed=splitMessage(message.content);
        auto removeFrom=[&](int index){while(blocks.size()>index){auto* block=blocks.takeLast().second;bubble->contentLayout->removeWidget(block);block->hide();block->deleteLater();}};
        for(int index=0;index<parsed.size();++index) {
            const auto& part=parsed[index];const QString key=part.kind+":"+part.language;
            if(index<blocks.size()&&blocks[index].first!=key)removeFrom(index);
            if(index==blocks.size()) {
                QWidget* block=nullptr;
                if(part.kind=="code"){auto* code=new ChatCodeBlock(part.language,bubble);connect(code,&ChatCodeBlock::warning,chat,&ChatWidget::renderWarning);block=code;}
                else{auto* markdown=new ChatMarkdownBlock(bubble);connect(markdown,&ChatMarkdownBlock::warning,chat,&ChatWidget::renderWarning);connect(markdown,&QTextBrowser::anchorClicked,chat,&ChatWidget::linkClicked);block=markdown;}
                blocks.append({key,block});bubble->contentLayout->addWidget(block);
            }
            if(auto* markdown=qobject_cast<ChatMarkdownBlock*>(blocks[index].second))markdown->setContent(part.content,chat->mathEnabled_,chat->renderer_,force);
            else qobject_cast<ChatCodeBlock*>(blocks[index].second)->setContent(part.content,force);
            if(!guard||revision!=contentRevision||chat->rows_.value(message.id)!=this)return;
        }
        removeFrom(parsed.size());bubble->update();
    }
    void applyTheme(bool render=true){QPointer<ChatRow> guard(this);const QString muted=isDarkTheme()?"#999999":"#707070";for(auto* label:{nameLabel,statusLabel})label->setStyleSheet("QLabel {color: "+muted+";background: transparent;}");statusLabel->setText(chat->tr("Typing..."));if(render)setContent(true);if(!guard)return;bubble->update();for(const auto& action:chat->actions_)if(action.thumb>=0&&buttons.contains(action.id))buttons[action.id]->setIcon(thumbIcon(action.thumb==1));}
    void syncActions() {
        for(auto* button:buttons){flow->removeWidget(button);button->hide();button->deleteLater();}buttons.clear();
        for(const auto& action:chat->actions_)if(action.accepts(message)) {
            auto* button=action.fluent>=0?new TransparentToolButton(static_cast<FluentIconEnum>(action.fluent),toolbar):new TransparentToolButton(action.thumb>=0?thumbIcon(action.thumb==1):action.icon,toolbar);
            button->setFixedSize(28,28);button->setIconSize({16,16});button->setToolTip(action.toolTip);button->setAccessibleName(action.toolTip);buttons.insert(action.id,button);flow->addWidget(button);
            connect(button,&QAbstractButton::clicked,chat,[c=chat,id=message.id,actionId=action.id]{c->triggerAction(id,actionId);});
        }
        updateVisibility();
    }
    void updateVisibility(){toolbar->setVisible(chat->messageTools_&&!buttons.isEmpty());}
    int naturalWidth() const{return qMax(0,int(buttons.size())*32-4);}
    void fitWidth(int available) {
        const int maximum=qMax(60,qMin(chat->maximumBubbleWidth_,qMin(int(available*.76),available-58)));int width;
        if(!image.isNull()){
            const qreal sourceRatio=image.devicePixelRatio(),targetRatio=devicePixelRatioF();
            QSize size(qMax(1,qRound(image.width()/sourceRatio)),qMax(1,qRound(image.height()/sourceRatio)));
            size.scale(qMax(1,qMin(maximum-28,size.width())),qMin(260,size.height()),Qt::KeepAspectRatio);
            auto pixmap=QPixmap::fromImage(image).scaled(QSize(qCeil(size.width()*targetRatio),qCeil(size.height()*targetRatio)),Qt::KeepAspectRatio,Qt::SmoothTransformation);
            pixmap.setDevicePixelRatio(targetRatio);imageLabel->setPixmap(pixmap);imageLabel->setFixedSize(size);width=qMin(maximum,size.width()+28);
        }
        else if(message.kind=="widget")width=maximum;
        else {bool rich=message.content.contains('\n');for(const auto& block:blocks)rich=rich||block.first.startsWith("code:");const int natural=QFontMetrics(getFont(14)).horizontalAdvance(message.content)+36;width=rich?maximum:qMin(maximum,qMax(60,natural));}
        bubble->setFixedWidth(width);const int toolWidth=chat->messageTools_?naturalWidth():0;body->setFixedWidth(qMax(width,qMin(maximum,toolWidth+1)));
        toolbar->setFixedWidth(qMax(28,qMin(body->width(),naturalWidth()+1)));toolbar->setFixedHeight(qMax(28,flow->heightForWidth(toolbar->width())));
    }
    ChatWidget* chat;ChatMessage message;QImage image;bool outgoing;AvatarWidget* avatar;QWidget* body;ChatBubble* bubble;QLabel* nameLabel;QLabel* statusLabel;QLabel* imageLabel=nullptr;
    QWidget* toolbar;FlowLayout* flow;QHash<QString,TransparentToolButton*> buttons;QVector<QPair<QString,QWidget*>> blocks;quint64 contentRevision=0;
protected:bool eventFilter(QObject* watched,QEvent* e) override{QPointer<ChatRow> guard(this);if(watched==imageLabel&&e->type()==QEvent::MouseButtonRelease&&static_cast<QMouseEvent*>(e)->button()==Qt::LeftButton)emit chat->imageClicked(message.id);return guard?QWidget::eventFilter(watched,e):true;}
};
ChatWidget::ChatWidget(QWidget* parent):QWidget(parent) {
    setFont(getFont(14));timer_.setSingleShot(true);timer_.setInterval(40);connect(&timer_,&QTimer::timeout,this,&ChatWidget::flush);
    auto standard=[&](QString id,FluentIconEnum icon,QString label,std::optional<QStringList> roles={},std::optional<QStringList> kinds={}){Action action{id,{},label,{},roles,kinds};action.fluent=int(icon);actions_.append(action);};
    standard("copy",FluentIconEnum::Copy,tr("Copy message"),{},QStringList{"text","image"});standard("retry",FluentIconEnum::Sync,tr("Retry"),QStringList{"assistant"},QStringList{"text"});
    for(int down=0;down<2;++down){Action action{down?"dislike":"like",{},down?tr("Dislike"):tr("Like"),{},QStringList{"assistant"},QStringList{"text"}};action.thumb=down;actions_.append(action);}
    standard("readAloud",FluentIconEnum::Volume,tr("Read aloud"),QStringList{"assistant"},QStringList{"text"});standard("share",FluentIconEnum::Share,tr("Share"));
    auto* layout=new QVBoxLayout(this);layout->setContentsMargins(0,0,0,0);layout->setSpacing(0);splitter=new ChatSplitter(this);splitter->setHandleWidth(6);splitter->setChildrenCollapsible(false);splitter->setOpaqueResize(true);layout->addWidget(splitter);
    scrollArea=new ScrollArea(this);scrollArea->setMinimumHeight(100);scrollArea->setWidgetResizable(true);scrollArea->setFrameShape(QFrame::NoFrame);scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    messageContainer=new QWidget;messageLayout=new QVBoxLayout(messageContainer);messageLayout->setContentsMargins(24,24,24,24);messageLayout->setSpacing(24);messageLayout->addStretch();scrollArea->setWidget(messageContainer);scrollArea->viewport()->installEventFilter(this);splitter->addWidget(scrollArea);
    connect(scrollArea->verticalScrollBar(),&QScrollBar::valueChanged,this,[this](int value){following_=scrollArea->verticalScrollBar()->maximum()-value<=32;jumpButton->setVisible(!following_);});
    connect(scrollArea->verticalScrollBar(),&QScrollBar::rangeChanged,this,[this]{if(following_)followBottom();});
    auto* jump=new ChatJumpButton(scrollArea->viewport());jumpButton=jump;jump->setFixedSize(36,36);jump->setIconSize({18,18});jump->setToolTip(tr("Jump to latest"));jump->setAccessibleName(tr("Jump to latest"));connect(jump,&QAbstractButton::clicked,this,&ChatWidget::scrollToBottom);jump->hide();
    composer=new QWidget(this);composer->setObjectName("chatComposer");auto* composerLayout=new QVBoxLayout(composer);composerLayout->setContentsMargins(20,12,20,14);composerLayout->setSpacing(6);
    toolBar=new QWidget(composer);auto* tools=new QHBoxLayout(toolBar);tools->setContentsMargins(0,0,0,0);tools->setSpacing(0);leftToolBar=new QWidget(toolBar);rightToolBar=new QWidget(toolBar);leftToolLayout=new QHBoxLayout(leftToolBar);rightToolLayout=new QHBoxLayout(rightToolBar);
    for(auto* side:{leftToolLayout,rightToolLayout}){side->setContentsMargins(0,0,0,0);side->setSpacing(6);}tools->addWidget(leftToolBar);tools->addStretch();tools->addWidget(rightToolBar);composerLayout->addWidget(toolBar);
    inputEdit=new ChatInput(this,composer);inputEdit->setMinimumHeight(76);inputEdit->setPlaceholderText(tr("Write a message..."));inputEdit->setAccessibleName(tr("Message input"));connect(inputEdit,&QPlainTextEdit::textChanged,this,&ChatWidget::updateSendButton);composerLayout->addWidget(inputEdit,1);
    auto* bottom=new QHBoxLayout;shortcutLabel=new QLabel(composer);bottom->addWidget(shortcutLabel);bottom->addStretch();stopButton=new PushButton(tr("Stop"),composer);connect(stopButton,&QAbstractButton::clicked,this,&ChatWidget::stopRequested);stopButton->hide();bottom->addWidget(stopButton);sendButton=new PrimaryPushButton(tr("Send"),composer);sendButton->setMinimumWidth(90);connect(sendButton,&QAbstractButton::clicked,this,&ChatWidget::send);bottom->addWidget(sendButton);composerLayout->addLayout(bottom);
    splitter->addWidget(composer);splitter->setStretchFactor(0,1);splitter->setStretchFactor(1,0);splitter->setSizes({460,184});setSendOnEnter(true);updateSendButton();connect(&QConfig::instance(),&QConfig::themeChangedFinished,this,&ChatWidget::applyTheme);applyTheme();
}
ChatRow* ChatWidget::row(const QString& id) const {auto* result=rows_.value(id,nullptr);if(!result)throw std::out_of_range("Unknown chat message id");return result;}
QString ChatWidget::add(const ChatMessage& value,const QImage& avatar,const QImage& image,QWidget* widget) {
    ChatMessage m=value;if(!QStringList{"user","assistant","system"}.contains(m.role))throw std::invalid_argument("Invalid message role");if(m.id.isNull())m.id=QUuid::createUuid().toString(QUuid::Id128);if(m.id.isEmpty()||rows_.contains(m.id))throw std::invalid_argument("Message id is empty or duplicated");
    QPointer<ChatWidget> guard(this);QPointer<ChatRow> item=new ChatRow(this,m,avatar,image,widget);rows_.insert(m.id,item);order_.append(m.id);messageLayout->insertWidget(messageLayout->count()-1,item);
    item->setContent();if(!guard||!item||rows_.value(m.id)!=item)return m.id;
    fitRows();emit messageAdded(m.id);return m.id;
}
QString ChatWidget::addMessage(const QString& content,const QString& role,const QString& name,const QImage& avatar,const QString& id,bool streaming){return add({id,role,content,name,streaming,"text"},avatar);}
QString ChatWidget::addImageMessage(const QImage& image,const QString& role,const QString& name,const QImage& avatar,const QString& id){if(image.isNull())throw std::invalid_argument("Cannot load chat image");return add({id,role,{},name,false,"image"},avatar,image);}
QString ChatWidget::addImageMessage(const QString& path,const QString& role,const QString& name,const QImage& avatar,const QString& id){return addImageMessage(QImage(path),role,name,avatar,id);}
QString ChatWidget::addImageMessage(const QPixmap& image,const QString& role,const QString& name,const QImage& avatar,const QString& id){return addImageMessage(image.toImage(),role,name,avatar,id);}
QString ChatWidget::addWidgetMessage(QWidget* widget,const QString& role,const QString& name,const QImage& avatar,const QString& id){if(!widget||widget==this||widget->isAncestorOf(this))throw std::invalid_argument("A widget message requires an independent QWidget");return add({id,role,{},name,false,"widget"},avatar,{},widget);}
ChatMessage ChatWidget::message(const QString& id) const{return row(id)->message;}
QList<ChatMessage> ChatWidget::messages() const{QList<ChatMessage> result;for(const auto& id:order_)result.append(row(id)->message);return result;}
void ChatWidget::appendText(const QString& id,const QString& delta){if(queuedDepth_&&!rows_.contains(id))return;setMessageText(id,row(id)->message.content+delta);}
void ChatWidget::setMessageText(const QString& id,const QString& content){if(queuedDepth_&&!rows_.contains(id))return;auto* item=row(id);if(item->message.kind!="text"){if(queuedDepth_){emit renderWarning(tr("Only text messages support content updates"));return;}throw std::invalid_argument("Only text messages support content updates");}item->message.content=content;dirty_.insert(id);if(!timer_.isActive())timer_.start();emit messageChanged(id);}
void ChatWidget::setMessageStreaming(const QString& id,bool streaming){
    const auto key=id;if(queuedDepth_&&!rows_.contains(key))return;
    QPointer<ChatWidget> guard(this);QPointer<ChatRow> item=row(key);item->message.streaming=streaming;dirty_.insert(key);flush();
    if(guard&&item&&rows_.value(key)==item)emit messageChanged(key);
}
void ChatWidget::removeMessage(const QString& id){auto* item=row(id);rows_.remove(id);order_.removeAll(id);dirty_.remove(id);messageLayout->removeWidget(item);item->hide();item->deleteLater();emit messageRemoved(id);}
void ChatWidget::clear(){QPointer<ChatWidget> guard(this);timer_.stop();dirty_.clear();QVector<QPair<QString,QPointer<ChatRow>>> snapshot;for(const auto& id:order_)snapshot.append({id,rows_.value(id)});for(const auto& entry:snapshot){if(entry.second&&rows_.value(entry.first)==entry.second)removeMessage(entry.first);if(!guard)return;}following_=true;jumpButton->hide();}
TransparentToolButton* ChatWidget::addToolButton(const QIcon& icon,const QString& label,std::function<void()> callback,const QString& side){if(side!="left"&&side!="right")throw std::invalid_argument("Invalid toolbar side");auto* button=new TransparentToolButton(icon,toolBar);button->setFixedSize(32,32);button->setToolTip(label);button->setAccessibleName(label);if(callback)connect(button,&QAbstractButton::clicked,this,[this,callback]{QPointer<ChatWidget> guard(this);try{callback();}catch(const std::exception& error){if(guard)emit renderWarning(tr("Tool callback failed: %1").arg(QString::fromUtf8(error.what())));}catch(...){if(guard)emit renderWarning(tr("Tool callback failed: %1").arg(tr("Unknown error")));}});addToolWidget(button,side);return button;}
TransparentToolButton* ChatWidget::addToolButton(FluentIconEnum icon,const QString& label,std::function<void()> callback,const QString& side){auto* button=addToolButton(QIcon(),label,std::move(callback),side);button->setIcon(FluentIcon(icon));return button;}
void ChatWidget::addToolWidget(QWidget* widget,const QString& side){if(!widget||widget==this||widget->isAncestorOf(this)||!(side=="left"||side=="right"))throw std::invalid_argument("Invalid toolbar widget or side");(side=="left"?leftToolLayout:rightToolLayout)->addWidget(widget);}
void ChatWidget::removeToolWidget(QWidget* widget){if(!widget)return;leftToolLayout->removeWidget(widget);rightToolLayout->removeWidget(widget);widget->hide();widget->setParent(nullptr);}
void ChatWidget::setMessageToolBarEnabled(bool enabled){messageTools_=enabled;for(auto* item:rows_)item->updateVisibility();fitRows();}
void ChatWidget::addMessageAction(const QString& id,const QIcon& icon,const QString& label,std::function<void(const QString&)> callback,std::optional<QStringList> roles,std::optional<QStringList> kinds) {
    if(id.isEmpty())throw std::invalid_argument("Empty message action id");for(const auto& action:actions_)if(action.id==id)throw std::invalid_argument("Duplicated message action id");
    auto validate=[](const std::optional<QStringList>& values,const QStringList& allowed){if(!values)return;if(values->isEmpty())throw std::invalid_argument("Empty message action filter");for(const auto& value:*values)if(!allowed.contains(value))throw std::invalid_argument("Invalid message action filter");};validate(roles,{"user","assistant","system"});validate(kinds,{"text","image","widget"});actions_.append({id,icon,label,std::move(callback),roles,kinds});syncActions();
}
void ChatWidget::addMessageAction(const QString& id,FluentIconEnum icon,const QString& label,std::function<void(const QString&)> callback,std::optional<QStringList> roles,std::optional<QStringList> kinds){addMessageAction(id,QIcon(),label,std::move(callback),roles,kinds);actions_.last().fluent=int(icon);syncActions();}
void ChatWidget::removeMessageAction(const QString& id){for(int i=0;i<actions_.size();++i)if(actions_[i].id==id){actions_.remove(i);syncActions();return;}throw std::out_of_range("Unknown message action id");}
void ChatWidget::clearMessageActions(){actions_.clear();syncActions();}
QWidget* ChatWidget::messageToolBar(const QString& id) const{return row(id)->toolbar;}
TransparentToolButton* ChatWidget::messageActionButton(const QString& id,const QString& actionId) const{return row(id)->buttons.value(actionId,nullptr);}
void ChatWidget::syncActions(){for(auto* item:rows_)item->syncActions();fitRows();}
void ChatWidget::triggerAction(const QString& id,const QString& actionId){
    auto* item=rows_.value(id);if(!item)return;
    for(const auto& definition:actions_)if(definition.id==actionId&&definition.accepts(item->message)){
        const auto action=definition;const auto key=id;QPointer<ChatWidget> guard(this);
        try{if(action.callback)action.callback(key);else if(actionId=="copy"){if(!item->image.isNull())QApplication::clipboard()->setImage(item->image);else QApplication::clipboard()->setText(item->message.content);}}
        catch(const std::exception& error){if(guard)emit renderWarning(tr("Message action failed: %1").arg(QString::fromUtf8(error.what())));return;}
        catch(...){if(guard)emit renderWarning(tr("Message action failed: %1").arg(tr("Unknown error")));return;}
        if(guard)emit messageActionTriggered(action.id,key);return;
    }
}
void ChatWidget::setMathEnabled(bool enabled){mathEnabled_=enabled;refreshContent();}
void ChatWidget::setFormulaRenderer(ChatFormulaRenderer renderer){renderer_=std::move(renderer);refreshContent();}
void ChatWidget::setMaximumBubbleWidth(int width){if(width<100)throw std::invalid_argument("Maximum bubble width must be at least 100");maximumBubbleWidth_=width;fitRows();}
void ChatWidget::setBubbleColors(const QColor& il,const QColor& id,const QColor& ol,const QColor& od){for(const auto& c:{il,id,ol,od})if(!c.isValid())throw std::invalid_argument("Invalid bubble color");bubbleColors_={il,id,ol,od};refreshContent();}
void ChatWidget::setComposerVisible(bool visible){if(visible==!composer->isHidden())return;if(!visible)composerSizes_=splitter->sizes();composer->setVisible(visible);if(visible&&!composerSizes_.isEmpty())splitter->setSizes(composerSizes_);}
void ChatWidget::setSendOnEnter(bool enabled){static_cast<ChatInput*>(inputEdit)->sendOnEnter=enabled;shortcutLabel->setText(enabled?tr("Enter to send, Shift+Enter for a new line"):tr("Ctrl+Enter to send, Enter for a new line"));}
void ChatWidget::setBusy(bool busy){busy_=busy;stopButton->setVisible(busy);updateSendButton();}
void ChatWidget::send(){const QString content=inputEdit->toPlainText();if(content.trimmed().isEmpty()||busy_)return;QPointer<ChatWidget> guard(this);inputEdit->clear();if(guard)emit sendRequested(content);}
void ChatWidget::updateSendButton(){sendButton->setEnabled(!inputEdit->toPlainText().trimmed().isEmpty()&&!busy_);}
QColor ChatWidget::bubbleColor(bool outgoing) const{if(!bubbleColors_.isEmpty())return bubbleColors_[(outgoing?2:0)+(isDarkTheme()?1:0)];return QColor(isDarkTheme()?(outgoing?"#253c53":"#333333"):(outgoing?"#e6f2ff":"#f1f1f1"));}
void ChatWidget::applyTheme(){const QString bg=isDarkTheme()?"#202020":"#ffffff",fg=isDarkTheme()?"#eeeeee":"#202020",muted=isDarkTheme()?"#999999":"#707070";scrollArea->setStyleSheet("QScrollArea {background:"+bg+";border:none;}");messageContainer->setStyleSheet("QWidget {background:transparent;}");scrollArea->viewport()->setStyleSheet("background:"+bg+";");composer->setStyleSheet("QWidget#chatComposer {background:"+bg+";}");splitter->handle(1)->update();jumpButton->update();inputEdit->setStyleSheet("QPlainTextEdit {border:none;padding:4px 0;background:transparent;color:"+fg+";}");shortcutLabel->setStyleSheet("QLabel {color:"+muted+";background:transparent;}");refreshContent();}
void ChatWidget::refreshContent(){QPointer<ChatWidget> guard(this);QVector<QPointer<ChatRow>> snapshot;for(auto* item:rows_)snapshot.append(item);for(const auto& item:snapshot){if(item&&rows_.value(item->message.id)==item)item->applyTheme();if(!guard)return;}fitRows();}
void ChatWidget::flush(){QPointer<ChatWidget> guard(this);timer_.stop();const auto dirty=dirty_;dirty_.clear();for(const auto& id:dirty){QPointer<ChatRow> item=rows_.value(id);if(item){item->setContent();if(!guard)return;if(item&&rows_.value(id)==item)item->statusLabel->setVisible(item->message.streaming);}}fitRows();}
bool ChatWidget::event(QEvent* event){
    if(event->type()!=QEvent::MetaCall)return QWidget::event(event);
    QPointer<ChatWidget> guard(this);++queuedDepth_;const bool handled=QWidget::event(event);if(guard)--queuedDepth_;return handled;
}
void ChatWidget::fitRows(){if(!scrollArea)return;const int width=qMax(100,scrollArea->viewport()->width()-48);for(auto* item:rows_)item->fitWidth(width);messageLayout->activate();if(following_)QTimer::singleShot(0,this,&ChatWidget::followBottom);}
void ChatWidget::followBottom(){if(following_)scrollArea->verticalScrollBar()->setValue(scrollArea->verticalScrollBar()->maximum());}
void ChatWidget::scrollToBottom(){following_=true;followBottom();jumpButton->hide();}
bool ChatWidget::eventFilter(QObject* watched,QEvent* e){if(scrollArea&&watched==scrollArea->viewport()&&e->type()==QEvent::Resize){fitRows();if(jumpButton){jumpButton->move(qMax(0,scrollArea->viewport()->width()-jumpButton->width()-18),qMax(0,scrollArea->viewport()->height()-jumpButton->height()-14));jumpButton->raise();}}return QWidget::eventFilter(watched,e);}
}
