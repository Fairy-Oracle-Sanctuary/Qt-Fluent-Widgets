#include "chat_interface.h"
#include "components/widgets/check_box.h"
#include "components/widgets/button.h"
#include "components/widgets/label.h"
#include "common/config.h"
#include "common/style_sheet.h"
#include "window/fluent_window.h"
#include <QFileDialog>
#include <QHBoxLayout>
#include <QHideEvent>
#include <QPainter>
#include <QPlainTextEdit>
#include <QVBoxLayout>
namespace qfw {
static QImage sampleChatImage() {
    QImage image(720,260,QImage::Format_ARGB32);image.setDevicePixelRatio(2);image.fill(QColor("#dff3ff"));QPainter p(&image);p.setRenderHint(QPainter::Antialiasing);p.setBrush(QColor("#55b8ed"));p.setPen(Qt::NoPen);p.drawEllipse(28,28,74,74);p.setPen(QColor("#16354a"));auto font=p.font();font.setPixelSize(24);p.setFont(font);p.drawText(QRect(120,0,225,130),Qt::AlignVCenter,"Native Qt chat");return image;
}
ChatDemo::ChatDemo(QWidget* parent):QWidget(parent) {
    auto* layout=new QVBoxLayout(this);layout->setContentsMargins(24,18,24,18);auto* header=new QHBoxLayout;
    header->addWidget(new BodyLabel(tr("ChatWidget — native chat"),this));header->addStretch();auto* tools=new CheckBox(tr("Message toolbar"),this);tools->setChecked(true);header->addWidget(tools);
    auto* resetButton=new PushButton(tr("Reset example"),this);header->addWidget(resetButton);auto* theme=new PushButton(tr("Toggle theme"),this);header->addWidget(theme);layout->addLayout(header);
    auto* options=new QHBoxLayout;auto* formulas=new CheckBox(tr("Render formulas"),this);formulas->setChecked(true);options->addWidget(formulas);
    auto* composer=new CheckBox(tr("Show composer"),this);composer->setChecked(true);options->addWidget(composer);
    auto* shortcut=new CheckBox(tr("Send with Ctrl+Enter"),this);options->addWidget(shortcut);options->addStretch();
    auto* stream=new PushButton(tr("Stream mixed content"),this);options->addWidget(stream);layout->addLayout(options);
    chat=new ChatWidget(this);chat->setMessageToolBarEnabled(true);layout->addWidget(chat,1);status_=new BodyLabel(tr("Local demo; no model or network request."),this);status_->setWordWrap(true);layout->addWidget(status_);
    connect(tools,&QCheckBox::toggled,chat,&ChatWidget::setMessageToolBarEnabled);connect(resetButton,&QAbstractButton::clicked,this,&ChatDemo::reset);
    connect(formulas,&QCheckBox::toggled,chat,&ChatWidget::setMathEnabled);
    connect(composer,&QCheckBox::toggled,chat,&ChatWidget::setComposerVisible);
    connect(shortcut,&QCheckBox::toggled,chat,[this](bool enabled){chat->setSendOnEnter(!enabled);});
    connect(stream,&QAbstractButton::clicked,this,[this]{send(tr("Show a mixed reply with code and formulas."));});
    connect(theme,&QAbstractButton::clicked,this,[]{QConfig::instance().setTheme(isDarkTheme()?Theme::Light:Theme::Dark);});
    chat->addToolButton(FluentIconEnum::EmojiTabSymbols,tr("Emoji"),[this]{chat->inputEdit->insertPlainText(QString::fromUtf8(" 😊 "));});
    chat->addToolButton(FluentIconEnum::Folder,tr("File (custom callback)"),[this]{status_->setText(tr("Connect your file selection and upload handler here."));});
    chat->addToolButton(FluentIconEnum::Photo,tr("Send local image"),[this]{const auto path=QFileDialog::getOpenFileName(this,tr("Choose image"),{},"Images (*.png *.jpg *.jpeg *.webp *.bmp)");if(path.isEmpty())return;try{chat->addImageMessage(path,"user",tr("Me"));chat->scrollToBottom();}catch(const std::exception&){status_->setText(tr("Cannot load image."));}});
    chat->addToolButton(FluentIconEnum::Microphone,tr("Voice (custom callback)"),[this]{status_->setText(tr("Connect your recording handler here."));});
    chat->addToolButton(FluentIconEnum::History,tr("Jump to latest"),[this]{chat->scrollToBottom();},"right");
    chat->addToolButton(FluentIconEnum::Delete,tr("Clear messages"),[this]{stop();chat->clear();},"right");
    connect(chat,&ChatWidget::sendRequested,this,&ChatDemo::send);connect(chat,&ChatWidget::stopRequested,this,&ChatDemo::stop);connect(chat,&ChatWidget::renderWarning,status_,&BodyLabel::setText);
    connect(chat,&ChatWidget::linkClicked,this,[this](const QUrl& url){status_->setText(tr("Link clicked (not opened): %1").arg(url.toString()));});connect(chat,&ChatWidget::imageClicked,this,[this](const QString& id){status_->setText(tr("Image clicked: %1").arg(id));});
    connect(chat,&ChatWidget::messageActionTriggered,this,[this](const QString& action,const QString& id){status_->setText(tr("Message action: %1; ID: %2").arg(action,id));});
    timer_.setInterval(35);connect(&timer_,&QTimer::timeout,this,&ChatDemo::tick);reset();
}
void ChatDemo::reset(){stop();chat->clear();chat->addMessage(tr("Hello! Can you show a Python example?"),"user",tr("Me"));chat->addMessage(tr("Of course. This is **native Qt**, with no WebView.")+"\n\n```python\nname = \"Fluent\"\nprint(f\"Hello, {name}!\")\n```\n", "assistant",tr("Fluent assistant"));chat->addMessage(tr("Images and custom cards have independent message APIs."),"assistant",tr("Friend"));chat->addImageMessage(sampleChatImage(),"assistant",tr("Friend"));chat->addMessage(tr("Type a message to try streaming. Input tools and message actions are customizable."),"assistant",tr("Fluent assistant"));chat->addMessage(tr("Inline and display formulas")+"\n\n$x_i^2$ — $\\alpha+\\beta=\\gamma$\n\n$$\\frac{1}{2}+\\sqrt{x^2+y^2}$$\n\n$$\\sum_{i=0}^{n}i=\\frac{n(n+1)}{2}$$","assistant",tr("Fluent assistant"));chat->addMessage(tr("Unicode formulas")+"\n\n$é_i^2+Ж+𝕏$\n\n$\\text{café}$","assistant",tr("Fluent assistant"));chat->addMessage(tr("Nested formulas and font styles")+"\n\n$\\sqrt{\\sqrt{\\sqrt{\\sqrt{\\sqrt{\\sqrt{x}}}}}}$\n\n$\\boldsymbol{123+\\alpha+x}$ — $\\mathbf{\\mathbb{ABCabc}}$","assistant",tr("Fluent assistant"));status_->setText(tr("Local demo; no model or network request."));}
void ChatDemo::send(const QString& content){stop();chat->addMessage(content,"user",tr("Me"));streamId_=chat->addMessage({},"assistant",tr("Fluent assistant"),{},{},true);answer_=tr("Received. This is a local streaming reply; nothing is sent to the network.")+"\n\n**Markdown**\n\n- "+tr("Native code highlighting and copying")+"\n- "+tr("Scroll up without being pulled back to the latest message")+"\n\n```cpp\nQString id = chat->addMessage();\nchat->appendText(id, \"Hello\");\nchat->finishMessage(id);\n```\n\n"+tr("The formula appears when its closing delimiter arrives.")+"\n\n$$\\frac{x_i^2+é}{\\sqrt{𝕏}}$$\n\n"+tr("Finished. Code and formula blocks keep their own layout.");offset_=0;chat->setBusy(true);chat->scrollToBottom();timer_.start();}
void ChatDemo::tick(){if(offset_>=answer_.size()){stop();return;}int length=qMin(4,int(answer_.size())-offset_);if(offset_+length<answer_.size()&&answer_.at(offset_+length-1).isHighSurrogate())++length;chat->appendText(streamId_,answer_.mid(offset_,length));offset_+=length;}
void ChatDemo::stop(){timer_.stop();if(!streamId_.isEmpty()){chat->finishMessage(streamId_);streamId_.clear();}answer_.clear();chat->setBusy(false);}
void ChatDemo::hideEvent(QHideEvent* event){stop();QWidget::hideEvent(event);}
ChatInterface::ChatInterface(QWidget* parent):GalleryInterface(tr("Chat"),"qtfluentwidgets.components.widgets.chat_widget",parent) {
    setObjectName("chatInterface");auto* host=new QWidget(this);auto* layout=new QVBoxLayout(host);layout->setContentsMargins(0,0,0,0);
    auto* demo=new ChatDemo(host);demo->setMinimumWidth(640);demo->setFixedHeight(700);layout->addWidget(demo);
    auto* open=new PushButton(tr("Open independent chat window"),host);layout->addWidget(open,0,Qt::AlignLeft);
    connect(open,&QAbstractButton::clicked,this,[]{auto* window=new FluentWidget;window->setAttribute(Qt::WA_DeleteOnClose);window->setWindowTitle(ChatInterface::tr("Native chat"));window->resize(1040,840);window->setMinimumSize(760,560);window->setContentWidget(new ChatDemo(window));window->show();});
    addExampleCard(tr("Native chat with streaming, images and custom toolbars"),host,{},1);
}
}
