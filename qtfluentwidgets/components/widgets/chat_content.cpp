#include "chat_content.h"
#include "code_edit.h"
#include "button.h"
#include "common/font.h"
#include "common/chat_math.h"
#include "common/style_sheet.h"
#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QClipboard>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QLabel>
#include <QPointer>
#include <QRegularExpression>
#include <QScrollBar>
#include <QTextCursor>
#include <QTextImageFormat>
#include <QUuid>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <cmath>
#include <stdexcept>

namespace qfw {
QVector<ChatContentBlock> splitMessage(const QString& text) {
    QVector<ChatContentBlock> blocks;QString prose,code,fence,language;
    const QRegularExpression lines("[^\\n\\r\\x0b\\x0c\\x1c-\\x1e\\x85\\x{2028}\\x{2029}]*(?:\\r\\n|[\\n\\r\\x0b\\x0c\\x1c-\\x1e\\x85\\x{2028}\\x{2029}]|$)");
    const QRegularExpression candidate("^ {0,3}(`{3,}|~{3,})([^\\r\\n]*)[\\r\\n]*$");
    auto matches=lines.globalMatch(text);
    while(matches.hasNext()) {
        const auto line=matches.next().captured();if(line.isEmpty())continue;
        const auto match=candidate.match(line);
        if(fence.isEmpty()) {
            if(match.hasMatch()&&(match.captured(1)[0]!='`'||!match.captured(2).contains('`'))) {
                if(!prose.isEmpty()){blocks.append({"text","",prose});prose.clear();}
                fence=match.captured(1);language=match.captured(2).trimmed().section(' ',0,0).toLower();
            } else prose+=line;
        } else if(match.hasMatch()&&match.captured(1)[0]==fence[0]&&match.captured(1).size()>=fence.size()&&match.captured(2).trimmed().isEmpty()) {
            if(code.endsWith('\n'))code.chop(1);if(code.endsWith('\r'))code.chop(1);
            blocks.append({"code",language,code});fence.clear();code.clear();
        } else code+=line;
    }
    if(!fence.isEmpty())blocks.append({"code",language,code});
    if(!prose.isEmpty())blocks.append({"text","",prose});
    if(blocks.isEmpty())blocks.append({"text","",""});return blocks;
}
ChatMarkdownBlock::ChatMarkdownBlock(QWidget* parent):QTextBrowser(parent) {
    setFrameShape(QFrame::NoFrame);setOpenLinks(false);setOpenExternalLinks(false);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    document()->setDocumentMargin(0);setFont(getFont(14));
    connect(document()->documentLayout(),&QAbstractTextDocumentLayout::documentSizeChanged,this,[this]{fitHeight();});
    connect(horizontalScrollBar(),&QScrollBar::rangeChanged,this,[this]{fitHeight();});
}
QVariant ChatMarkdownBlock::loadResource(int,const QUrl&){return {};}
void ChatMarkdownBlock::setContent(const QString& text,bool mathEnabled,const ChatFormulaRenderer& renderer,bool force) {
    // std::function cannot compare capturing callbacks; a supplied renderer
    // must be invoked when the caller replaces it for otherwise identical text.
    if(initialized_&&!renderer&&!customRenderer_&&!force&&text_==text&&mathEnabled_==mathEnabled)return;
    const auto callback=renderer;const QString content=text;const quint64 revision=++revision_;
    QPointer<ChatMarkdownBlock> guard(this);QStringList warnings;
    initialized_=true;text_=text;mathEnabled_=mathEnabled;customRenderer_=bool(callback);
    const auto foreground=isDarkTheme()?QColor("#eeeeee"):QColor("#202020");
    struct Resource {QString token,source;QImage image;bool display;};QVector<Resource> resources;
    // Code spans close with exactly the opening number of backticks. A simple
    // regex would consume an inner single tick and expose literal formulas.
    QVector<QPair<int,int>> codeSpans;
    for(int i=0;i<content.size();){
        if(content[i]!='`'){++i;continue;}int escaped=0;for(int j=i-1;j>=0&&content[j]=='\\';--j)++escaped;
        const int begin=i;while(i<content.size()&&content[i]=='`')++i;const int length=i-begin;
        if(escaped%2)continue;
        for(int j=i;j<content.size();){if(content[j]!='`'){++j;continue;}const int close=j;while(j<content.size()&&content[j]=='`')++j;
            if(j-close==length){codeSpans.append({begin,j});i=j;break;}}
    }
    const QRegularExpression math(R"((?<display>(?<!\\)\$\$[\s\S]+?(?<!\\)\$\$|\\\[[\s\S]+?\\\])|(?<inline>\\\([^\n]+?\\\)|(?<![\\$])\$(?!\$)[^\n$]+?(?<!\\)\$(?!\$)))");
    QString markdown;int end=0;auto matches=math.globalMatch(content);
    while(matches.hasNext()) {
        const auto match=matches.next();markdown+=content.mid(end,match.capturedStart()-end);end=match.capturedEnd();
        const QString source=match.captured();
        bool literal=false;for(const auto& span:codeSpans)if(span.first<match.capturedEnd()&&span.second>match.capturedStart()){literal=true;break;}
        if(!mathEnabled||literal){markdown+=source;continue;}
        const bool display=!match.captured("display").isNull();
        const int trim=source.startsWith("$$")||source.startsWith("\\(")||source.startsWith("\\[")?2:1;
        const QString expression=source.mid(trim,source.size()-trim*2),token="MATH"+QUuid::createUuid().toString(QUuid::Id128).toUpper();QImage image;
        try {
            image=callback?callback(expression,foreground,12):renderFormula(expression,foreground,12);
            if(!guard||revision_!=revision)return;
            const qreal ratio=image.devicePixelRatio();
            if(image.isNull()||!std::isfinite(ratio)||ratio<=0||image.width()>8192||image.height()>8192||qint64(image.width())*image.height()>16777216)
                throw std::runtime_error("Formula renderer must return a bounded, non-empty QImage with a valid device pixel ratio");
        } catch(const std::exception& e){if(!guard||revision_!=revision)return;image={};warnings.append(tr("Could not render formula: %1").arg(QString::fromUtf8(e.what())));}
        catch(...){if(!guard||revision_!=revision)return;image={};warnings.append(tr("Could not render formula: %1").arg(tr("Formula renderer failed")));}
        resources.append({token,source,image,display});markdown+=display?"\n\n"+token+"\n\n":token;
    }
    markdown+=content.mid(end);document()->clear();
    if(!guard||revision_!=revision)return;
    document()->setDefaultStyleSheet(QString("p { margin-top: 0px; margin-bottom: 8px; } a { color: %1; } code { font-family: Consolas, monospace; } table { border-collapse: collapse; } td, th { padding: 5px; }").arg(isDarkTheme()?"#79c0ff":"#0067c0"));
    document()->setMarkdown(markdown,QTextDocument::MarkdownFeatures(QTextDocument::MarkdownDialectGitHub|QTextDocument::MarkdownNoHTML));
    if(!guard||revision_!=revision)return;
    document()->setDefaultFont(getFont(14));
    if(!guard||revision_!=revision)return;
    for(const auto& resource:resources) {
        auto cursor=document()->find(resource.token);if(cursor.isNull())continue;
        if(resource.image.isNull()){cursor.insertText(resource.source);if(!guard||revision_!=revision)return;continue;}
        const QUrl name("chat-math:/"+resource.token);document()->addResource(QTextDocument::ImageResource,name,resource.image);
        QTextImageFormat format;format.setName(name.toString());format.setWidth(resource.image.width()/resource.image.devicePixelRatio());format.setHeight(resource.image.height()/resource.image.devicePixelRatio());cursor.insertImage(format);
        if(!guard||revision_!=revision)return;
        if(resource.display){auto block=cursor.blockFormat();block.setAlignment(Qt::AlignCenter);cursor.setBlockFormat(block);if(!guard||revision_!=revision)return;}
    }
    setStyleSheet(QString("QTextBrowser { background: transparent; border: none; padding: 0; color: %1; }").arg(foreground.name()));setFont(getFont(14));document()->setDefaultFont(getFont(14));fitHeight();
    if(!guard||revision_!=revision)return;
    for(const auto& message:warnings){emit warning(message);if(!guard||revision_!=revision)return;}
}
void ChatMarkdownBlock::fitHeight() {
    if(fitting_)return;fitting_=true;document()->setTextWidth(qMax(20,viewport()->width()));
    int height=int(std::ceil(document()->size().height()))+2;if(horizontalScrollBar()->maximum()>0)height+=horizontalScrollBar()->sizeHint().height();
    setFixedHeight(qMax(fontMetrics().height()+2,height));fitting_=false;
}
void ChatMarkdownBlock::resizeEvent(QResizeEvent* e){QTextBrowser::resizeEvent(e);fitHeight();}
void ChatMarkdownBlock::wheelEvent(QWheelEvent* e){if(e->angleDelta().y()&&!(e->modifiers()&Qt::ShiftModifier))e->ignore();else QTextBrowser::wheelEvent(e);}
ChatCodeBlock::ChatCodeBlock(const QString& language,QWidget* parent):QFrame(parent),language_(language) {
    setObjectName("chatCodeBlock");auto* layout=new QVBoxLayout(this);layout->setContentsMargins(10,5,10,10);layout->setSpacing(6);
    auto* header=new QHBoxLayout;header->addWidget(new QLabel(language.isEmpty()?tr("Plain text"):language,this));header->addStretch();
    copyButton=new TransparentToolButton(FluentIconEnum::Copy,this);copyButton->setFixedSize(26,26);copyButton->setToolTip(tr("Copy code"));copyButton->setAccessibleName(tr("Copy code"));header->addWidget(copyButton);layout->addLayout(header);
    connect(copyButton,&QAbstractButton::clicked,this,[this]{QPointer<ChatCodeBlock> guard(this);const auto text=text_;QApplication::clipboard()->setText(text);if(guard)copyButton->setToolTip(tr("Copied"));});
    try {auto* code=new CodeEdit(this,language);code->setLineNumbersVisible(false);editor=code;}
    catch(const std::invalid_argument&){editor=new QPlainTextEdit(this);}
    editor->setReadOnly(true);QFont font;font.setFamilies({"Cascadia Mono","Consolas","DejaVu Sans Mono","monospace"});font.setPixelSize(14);editor->setFont(font);
    editor->setLineWrapMode(QPlainTextEdit::NoWrap);editor->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);editor->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);editor->document()->setDocumentMargin(2);layout->addWidget(editor);
}
void ChatCodeBlock::setContent(const QString& text,bool force) {
    if(initialized_&&!force&&text_==text)return;initialized_=true;text_=text;
    QPointer<ChatCodeBlock> guard(this);const QString content=text;const auto revision=++revision_;
    const int position=editor->horizontalScrollBar()->value(),vertical=editor->verticalScrollBar()->value();
    const auto previous=editor->textCursor();editor->setPlainText(content);if(!guard||revision!=revision_)return;
    QTextCursor selection(editor->document());const int end=editor->document()->characterCount()-1;
    selection.setPosition(qMin(previous.anchor(),end));selection.setPosition(qMin(previous.position(),end),QTextCursor::KeepAnchor);editor->setTextCursor(selection);editor->horizontalScrollBar()->setValue(position);
    editor->setFixedHeight(qMin(360,qMax(48,editor->fontMetrics().lineSpacing()*(int(content.count('\n'))+1)+24)));
    editor->verticalScrollBar()->setValue(vertical);
    setStyleSheet(QString("QFrame#chatCodeBlock { background: %1; border: 1px solid %2; border-radius: 8px; } QLabel { background: transparent; color: %3; }").arg(isDarkTheme()?"#262626":"#ffffff",isDarkTheme()?"#484848":"#dedede",isDarkTheme()?"#aaaaaa":"#666666"));
    editor->setStyleSheet(QString("QPlainTextEdit { background: transparent; border: none; padding: 0; color: %1; }").arg(isDarkTheme()?"#eeeeee":"#202020"));copyButton->setToolTip(tr("Copy code"));
}
}
