#include "code_edit.h"
#include "code_lexer_rules.h"
#include "common/config.h"
#include <QFontDatabase>
#include <QAbstractTextDocumentLayout>
#include <QFutureInterface>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QKeyEvent>
#include <QPainter>
#include <QRegularExpression>
#include <QRunnable>
#include <QSet>
#include <QTextBlock>
#include <QThreadPool>
#include <algorithm>
#include <stdexcept>

namespace qfw {
namespace {
struct LexToken { int start, length; QString kind; };
QVector<LexToken> tokenizeCode(const QString& text, const QString& language, int depth = 0, const QStringList& initialStack = {});
QStringList languages() {
    return {"python", "c", "cpp", "csharp", "java", "javascript", "typescript", "json",
            "html", "css", "xml", "yaml", "toml", "ini", "bash", "powershell", "sql",
            "go", "rust", "markdown"};
}
QString languageName(CodeLanguage language) {
    const int index=int(language);const auto names=languages();
    if(index<0||index>=names.size())throw std::invalid_argument("Unsupported code language enum");
    return names[index];
}

struct RegexCodeRule {
    QRegularExpression expression;
    QJsonValue action;
    QJsonValue transition;
};
using RegexCodeStates = QHash<QString, QVector<RegexCodeRule>>;
struct RegexCodeLexer { RegexCodeStates states; QSet<QString> identifierTypes; QStringList aliases; };
const QHash<QString, RegexCodeLexer>& regexCodeLanguages() {
    static const auto languages = [] {
        QByteArray encoded;
        for(const auto* piece:code_lexer_data::rules)encoded.append(piece);
        const auto decoded=qUncompress(QByteArray::fromBase64(encoded));
        QJsonParseError error;
        const auto document=QJsonDocument::fromJson(decoded,&error);
        if(error.error!=QJsonParseError::NoError||!document.isObject())throw std::runtime_error("Invalid native code lexer tables");
        QHash<QString,RegexCodeLexer> result;
        const QRegularExpression omittedMinimum(QStringLiteral(R"((?<!\\)\{,(\d+)\})"));
        const auto root=document.object();
        for(auto language=root.begin();language!=root.end();++language) {
            RegexCodeLexer lexer;
            const auto data=language.value().toObject();
            for(const auto& value:data.value("aliases").toArray())lexer.aliases.append(value.toString());
            for(const auto& value:data.value("identifierTypes").toArray())lexer.identifierTypes.insert(value.toString());
            const auto definitions=data.value("states").toObject();
            for(auto state=definitions.begin();state!=definitions.end();++state) {
                QVector<RegexCodeRule> rules;
                for(const auto& value:state.value().toArray()) {
                    const auto definition=value.toObject();const int flags=definition.value("flags").toInt();
                    QRegularExpression::PatternOptions options;
                    if(flags&2)options|=QRegularExpression::CaseInsensitiveOption;
                    if(flags&8)options|=QRegularExpression::MultilineOption;
                    if(flags&16)options|=QRegularExpression::DotMatchesEverythingOption;
                    if(flags&64)options|=QRegularExpression::ExtendedPatternSyntaxOption;
                    if(!(flags&256))options|=QRegularExpression::UseUnicodePropertiesOption;
                    QString pattern=definition.value("pattern").toString();
                    // Python's strict end-of-string anchor is PCRE's \z.
                    pattern.replace("\\Z","\\z");
                    // Older PCRE2 in Qt 5 treats Python's {,n} as literal text.
                    // Spell out the equivalent zero lower bound for both Qt versions.
                    pattern.replace(omittedMinimum,QStringLiteral("{0,\\1}"));
                    QRegularExpression expression(pattern,options);
                    if(!expression.isValid())throw std::runtime_error((language.key()+"/"+state.key()+": "+expression.errorString()).toStdString());
                    rules.append({expression,definition.value("action"),definition.value("next")});
                }
                lexer.states.insert(state.key(),std::move(rules));
            }
            result.insert(language.key(),std::move(lexer));
        }
        return result;
    }();
    return languages;
}

bool validUtf16(const QString& text) {
    for(int i=0;i<text.size();++i) {
        if(text[i].isHighSurrogate()){if(i+1>=text.size()||!text[i+1].isLowSurrogate())return false;++i;}
        else if(text[i].isLowSurrogate())return false;
    }
    return true;
}

QVector<LexToken> regexTokenize(const QString& text,const QString& language,const RegexCodeLexer& lexer,int depth,const QStringList& initialStack) {
    QVector<LexToken> tokens;
    const auto& states=lexer.states;
    QStringList stack=initialStack.isEmpty()?QStringList{"root"}:initialStack;int position=0,zeroSteps=0;
    int indent=-1,nextIndent=0,scalarIndent=-1;QVector<int> indentStack;
    QRegularExpression::MatchOptions matchOptions=QRegularExpression::AnchoredMatchOption;
    // Check once, rather than rescanning the whole document for every rule.
    if(validUtf16(text))matchOptions|=QRegularExpression::DontCheckSubjectStringMatchOption;
    auto append=[&](int start,int length,const QJsonValue& action) {
        if(start<0||action.isNull())return;
        const auto definition=action.toObject();
        if(definition.contains("yaml")) {
            const auto operation=definition.value("yaml").toString();
            auto kind=definition.value("token").toString();
            if(operation=="reset_indent"){indentStack.clear();indent=-1;nextIndent=0;scalarIndent=-1;}
            else if(operation=="save_indent") {
                if(definition.value("start").toBool()) {
                    nextIndent=length;
                    while(nextIndent<indent&&!indentStack.isEmpty()){indent=indentStack.takeLast();}
                    // Whitespace.Error is still uncolored in the reference.
                } else nextIndent+=length;
            } else if(operation=="set_indent") {
                if(indent<nextIndent){indentStack.append(indent);indent=nextIndent;}
                if(!definition.value("implicit").toBool())nextIndent+=length;
            } else if(operation=="set_block_scalar_indent") {
                scalarIndent=-1;
                const auto value=text.mid(start,length);
                for(QChar c:value)if(c>='1'&&c<='9'){scalarIndent=qMax(indent,0)+c.digitValue();break;}
            } else if(operation=="parse_block_scalar_empty_line") {
                if(scalarIndent>=0&&length>scalarIndent) {
                    tokens.append({start+scalarIndent,length-scalarIndent,definition.value("content").toString()});
                    length=scalarIndent;
                }
            } else if(operation=="parse_block_scalar_indent"||operation=="parse_plain_scalar_indent") {
                const bool leave=operation=="parse_plain_scalar_indent"?length<=indent:
                    scalarIndent<0?length<=qMax(indent,0):length<scalarIndent;
                if(leave){for(int i=0;i<2&&stack.size()>1;++i)stack.removeLast();return;}
                if(operation=="parse_block_scalar_indent"&&scalarIndent<0)scalarIndent=length;
            }
            // Empty scalar lines may split indentation/content into two tokens,
            // but the context still advances over the entire regex match.
            if(operation=="parse_block_scalar_empty_line") {
                const int newline=text.indexOf('\n',start);position=newline<0?int(text.size()):newline;
            } else position=start+length;
            if(length>0&&kind!="Text")tokens.append({start,length,kind});
            return;
        }
        if(length<=0)return;
        if(action.isString()) {
            auto kind=action.toString();
            if(kind=="CIdentifier")kind=lexer.identifierTypes.contains(text.mid(start,length))?"Keyword":"Name";
            if(kind!="Text")tokens.append({start,length,kind});
        } else if(action.toObject().contains("using")) {
            if(depth>=32){tokens.append({start,length,"Name"});return;}
            QStringList nestedStack;
            for(const auto& state:definition.value("stack").toArray())nestedStack.append(state.toString());
            const auto nestedLanguage=definition.value("using").toString()=="self"?language:definition.value("language").toString();
            const auto nested=tokenizeCode(text.mid(start,length),nestedLanguage,depth+1,nestedStack);
            for(const auto& token:nested)tokens.append({start+token.start,token.length,token.kind});
        }
    };
    while(position<text.size()) {
        const auto state=states.constFind(stack.last());
        if(state==states.constEnd())throw std::runtime_error("Unknown native code lexer state");
        bool matched=false;
        for(const auto& rule:state.value()) {
            const auto match=rule.expression.match(text,position,QRegularExpression::NormalMatch,matchOptions);
            if(!match.hasMatch())continue;
            const int previousPosition=position;
            const auto groups=rule.action.toObject().value("groups").toArray();
            if(rule.action.toObject().contains("markdownFence")) {
                for(const auto& group:QStringList{"initial","lang","terminator"})
                    tokens.append({int(match.capturedStart(group)),int(match.capturedLength(group)),"String"});
                const QString alias=match.captured("lang").trimmed().toLower();QString nestedLanguage;
                const auto& languages=regexCodeLanguages();
                for(auto it=languages.begin();it!=languages.end();++it)if(it.key()==alias||it->aliases.contains(alias)){nestedLanguage=it.key();break;}
                const int start=match.capturedStart("code"),length=match.capturedLength("code");
                if(depth>=32||nestedLanguage.isEmpty()) {if(length>0)tokens.append({start,length,"String"});}
                else for(const auto& token:tokenizeCode(match.captured("code"),nestedLanguage,depth+1))
                    tokens.append({start+token.start,token.length,token.kind});
                position=match.capturedEnd();
            } else
            if(rule.action.isObject()&&rule.action.toObject().contains("groups")) {
                for(int group=0;group<groups.size();++group)append(match.capturedStart(group+1),match.capturedLength(group+1),groups[group]);
            } else append(position,match.capturedLength(),rule.action);
            // ExtendedRegexLexer callbacks control their own cursor and stack.
            // In particular, dedenting a scalar retries this position in its parent.
            if(language!="yaml")position=match.capturedEnd();
            else if(rule.action.isString()||rule.action.toObject().contains("groups"))position=match.capturedEnd();
            const auto next=rule.transition;
            if(next.isDouble()) {
                int count=qMin(-next.toInt(),int(stack.size())-1);while(count-->0)stack.removeLast();
            } else if(next.isArray()) {
                for(const auto& value:next.toArray()) {
                    const auto name=value.toString();
                    if(name=="#pop"){if(stack.size()>1)stack.removeLast();}
                    else if(name=="#push")stack.append(stack.last());else stack.append(name);
                }
            } else if(next.toString()=="#push") {
                const auto current=stack.last();stack.append(current);
            }
            if(position==previousPosition){if(++zeroSteps>512)throw std::runtime_error("Native code lexer made no progress");}
            else zeroSteps=0;
            if(stack.size()>65536)throw std::runtime_error("Native code lexer nesting limit exceeded");
            matched=true;break;
        }
        if(matched)continue;
        // RegexLexer resets an unmatched newline to root; other unmatched
        // characters are Token.Error (the reference's default foreground).
        if(text[position]=='\n')stack=QStringList{"root"};
        else {int length=text[position].isHighSurrogate()&&position+1<text.size()&&text[position+1].isLowSurrogate()?2:1;tokens.append({position,length,"Name"});position+=length;zeroSteps=0;continue;}
        ++position;zeroSteps=0;
    }
    std::stable_sort(tokens.begin(),tokens.end(),[](const LexToken& a,const LexToken& b){return a.start<b.start;});
    return tokens;
}

QVector<LexToken> jsonTokenize(const QString& text) {
    enum State {Idle,String,Whitespace,Constant,Number,Punctuation,SingleComment,MultiComment,CommentOpener};
    State state=Idle;bool escape=false,closer=false;int unicode=0,start=0;
    QVector<LexToken> tokens,queue;
    auto emitToken=[&](int end,const QString& kind,bool queued=false) {
        if(end<=start)return;
        LexToken token{start,end-start,kind};
        if(queued)queue.append(token);else if(kind!="Text")tokens.append(token);
    };
    auto flush=[&](bool key=false) {
        for(auto token:queue){if(key&&token.kind=="String")token.kind="Tag";if(token.kind!="Text")tokens.append(token);}
        queue.clear();
    };
    const QString integers="-0123456789",floats=".eE+",constants="tneuarfls",punctuation="{}[],";
    for(int pos=0;pos<text.size();++pos) {
        const QChar c=text[pos];
        if(state==String) {
            if(unicode){if(QString("0123456789abcdefABCDEF").contains(c)){if(--unicode==0)escape=false;}else{unicode=0;escape=false;}}
            else if(escape){if(c=='u')unicode=4;else escape=false;}
            else if(c=='\\')escape=true;
            else if(c=='"'){emitToken(pos+1,"String",true);state=Idle;escape=false;unicode=0;}
            continue;
        }
        if(state==Whitespace){if(QString(" \t\r\n").contains(c))continue;emitToken(pos,"Text",!queue.isEmpty());state=Idle;}
        else if(state==Constant){if(constants.contains(c))continue;emitToken(pos,"Constant");state=Idle;}
        else if(state==Number){if(integers.contains(c)||floats.contains(c))continue;emitToken(pos,"Number");state=Idle;}
        else if(state==Punctuation){if(punctuation.contains(c))continue;emitToken(pos,"Name");state=Idle;}
        else if(state==SingleComment){if(c!='\n')continue;emitToken(pos,"Comment",!queue.isEmpty());state=Idle;}
        else if(state==MultiComment){if(c=='*')closer=true;else if(closer){closer=false;if(c=='/'){emitToken(pos+1,"Comment",!queue.isEmpty());state=Idle;}}continue;}
        else if(state==CommentOpener){state=Idle;if(c=='/'){state=SingleComment;continue;}if(c=='*'){state=MultiComment;continue;}flush();emitToken(pos,"Name");}
        start=pos;
        if(c=='"')state=String;
        else if(QString(" \t\r\n").contains(c))state=Whitespace;
        else if(c=='f'||c=='n'||c=='t'){flush();state=Constant;}
        else if(integers.contains(c)){flush();state=Number;}
        else if(c==':'){flush(true);state=Punctuation;}
        else if(punctuation.contains(c)){flush();state=Punctuation;}
        else if(c=='/')state=CommentOpener;
        else {flush();emitToken(pos+1,"Name");}
    }
    flush();
    if(state!=Idle)emitToken(text.size(),state==Constant?"Constant":state==Number?"Number":state==Whitespace?"Text":state==SingleComment?"Comment":"Name");
    std::stable_sort(tokens.begin(),tokens.end(),[](const LexToken& a,const LexToken& b){return a.start<b.start;});
    return tokens;
}
QVector<LexToken> tokenizeCode(const QString& text,const QString& language,int depth,const QStringList& initialStack) {
    if(language=="json")return jsonTokenize(text);
    const auto& rules=regexCodeLanguages();const auto lexer=rules.constFind(language);
    if(lexer!=rules.constEnd())return regexTokenize(text,language,lexer.value(),depth,initialStack);
    throw std::invalid_argument("Unsupported code language");
}
} // namespace

CodeLexResult lexCode(const QString& text, const QString& language) {
    CodeLexResult result; result.lines=text.split('\n'); result.spans.resize(result.lines.size());
    QVector<int> starts; int offset=0;
    for(const auto& line:result.lines){starts.append(offset);offset+=line.size()+1;}
    const auto tokens=tokenizeCode(text,language);
    for(const auto& token:tokens) {
        int pos=token.start, end=token.start+token.length;
        auto it=std::upper_bound(starts.begin(),starts.end(),pos);
        int row=int(it-starts.begin())-1;
        while(pos<end&&row<starts.size()) {
            int length=qMin(end,starts[row]+int(result.lines[row].size()))-pos;
            if(length>0) {
                auto& spans=result.spans[row];const int start=pos-starts[row];
                // Adjacent tokens with the same reference color form one run;
                // e.g. PowerShell emits each here-string character separately.
                if(!spans.isEmpty()&&spans.last().token==token.kind&&spans.last().start+spans.last().length==start)spans.last().length+=length;
                else spans.append({start,length,token.kind});
            }
            ++row; if(row<starts.size())pos=starts[row]; else break;
        }
    }
    return result;
}

CodeHighlighter::CodeHighlighter(QTextDocument* document):QSyntaxHighlighter(document) {}
QTextCharFormat CodeHighlighter::tokenFormat(const QString& token) const {
    const bool dark=isDarkTheme(); QString color;
    if(token=="Comment")color=dark?"#80a879":"#6a9955";
    else if(token=="Tag"||token=="Attribute")color=dark?"#ff8585":"#ff3636";
    else if(token=="String")color=dark?"#a4d88b":"#32a852";
    else if(token=="Constant")color=dark?"#dc9cff":"#c000d4";
    else if(token=="Keyword"||token=="Number"||token=="Heading")color=dark?"#72b9ff":"#005cc5";
    else if(token=="Function"||token=="Class")color=dark?"#e8cc85":"#795e26";
    else color=dark?"#eeeeee":"#172b4d";
    if(token=="Number"&&dark)color="#93caff";
    QTextCharFormat format;format.setForeground(QColor(color));return format;
}
void CodeHighlighter::highlightBlock(const QString& text) {
    int row=currentBlock().blockNumber();
    if(row>=snapshot.lines.size()||snapshot.lines[row]!=text)return;
    for(const auto& span:snapshot.spans[row])setFormat(span.start,span.length,tokenFormat(span.token));
}
class CodeLineNumberArea : public QWidget {
public:
    explicit CodeLineNumberArea(CodeEdit* editor):QWidget(editor),editor_(editor) {}
protected:
    void paintEvent(QPaintEvent* event) override {editor_->paintLineNumbers(event);}
private:
    CodeEdit* editor_;
};

CodeEdit::CodeEdit(QWidget* parent,CodeLanguage language):CodeEdit(parent,languageName(language)) {}
CodeEdit::CodeEdit(QWidget* parent,const QString& language):PlainTextEdit(parent) {
    setLineWrapMode(QPlainTextEdit::NoWrap);
    QFont font=QFontDatabase::systemFont(QFontDatabase::FixedFont);font.setPointSize(11);setFont(font);
    gutter_=new CodeLineNumberArea(this);highlighter_=new CodeHighlighter(document());
    lexTimer_.setSingleShot(true);lexTimer_.setInterval(80);applyTimer_.setSingleShot(true);
    connect(&lexTimer_,&QTimer::timeout,this,&CodeEdit::startHighlighting);
    connect(&applyTimer_,&QTimer::timeout,this,&CodeEdit::applyHighlightBatch);
    connect(this,&QPlainTextEdit::blockCountChanged,this,&CodeEdit::updateGutter);
    connect(this,&QPlainTextEdit::updateRequest,this,[this](const QRect& rect,int dy){
        if(dy)gutter_->scroll(0,dy);else gutter_->update(0,rect.y(),gutterWidth(),rect.height());
    });
    connect(this,&QPlainTextEdit::cursorPositionChanged,this,&CodeEdit::highlightCurrentLine);
    connect(document(),&QTextDocument::contentsChange,this,[this](int,int removed,int added){
        if(!removed&&!added)return;
        ++version_;pendingRows_.clear();applyTimer_.stop();lexTimer_.start(80);
    });
    connect(&QConfig::instance(),&QConfig::themeChangedFinished,this,[this]{highlighter_->rehighlight();highlightCurrentLine();});
    connect(&watcher_,&QFutureWatcher<CodeLexResult>::finished,this,[this]{
        jobActive_=false;
        if(jobVersion_!=version_){lexTimer_.start(0);return;}
        auto result=watcher_.result();
        if(!result.error.isEmpty()){emit highlightingFailed(result.error);return;}
        pendingRows_.clear();
        for(int row=0;row<result.lines.size();++row)
            if(row>=highlighter_->snapshot.lines.size()||result.lines[row]!=highlighter_->snapshot.lines[row]||result.spans[row]!=highlighter_->snapshot.spans[row])pendingRows_.append(row);
        highlighter_->snapshot=std::move(result);applyVersion_=version_;applyHighlightBatch();
    });
    setIndentSize(4);updateGutter();highlightCurrentLine();setLanguage(language);
}
QStringList CodeEdit::supportedLanguages(){return languages();}
void CodeEdit::setLanguage(CodeLanguage language){setLanguage(languageName(language));}
void CodeEdit::setLanguage(const QString& value) {
    QString lang=value.toLower();
    static const QHash<QString,QString> aliases={{"py","python"},{"c++","cpp"},{"c#","csharp"},{"js","javascript"},{"ts","typescript"},{"shell","bash"},{"ps1","powershell"},{"md","markdown"},{"yml","yaml"}};
    lang=aliases.value(lang,lang);
    if(!languages().contains(lang))throw std::invalid_argument(("Unsupported CodeEdit language: "+lang).toStdString());
    if(lang==language_)return;
    language_=lang;++version_;pendingRows_.clear();applyTimer_.stop();highlighter_->snapshot={};highlighter_->rehighlight();lexTimer_.start(80);emit languageChanged(lang);
}
void CodeEdit::setIndentSize(int size){indentSize_=qBound(1,size,16);setTabStopDistance(fontMetrics().horizontalAdvance(' ')*indentSize_);}
void CodeEdit::setLineNumbersVisible(bool visible){numbersVisible_=visible;gutter_->setVisible(visible);updateGutter();}
int CodeEdit::gutterWidth() const {return numbersVisible_?20+fontMetrics().horizontalAdvance('9')*QString::number(qMax(1,blockCount())).size():0;}
void CodeEdit::updateGutter(){setViewportMargins(gutterWidth(),0,0,0);gutter_->setGeometry(viewport()->x()-gutterWidth(),viewport()->y(),gutterWidth(),viewport()->height());gutter_->update();}
void CodeEdit::resizeEvent(QResizeEvent* event){PlainTextEdit::resizeEvent(event);if(gutter_)updateGutter();}
void CodeEdit::changeEvent(QEvent* event){
    PlainTextEdit::changeEvent(event);
    if(!gutter_)return;
    if(event->type()==QEvent::FontChange){setIndentSize(indentSize_);updateGutter();}
    else if(event->type()==QEvent::ReadOnlyChange)highlightCurrentLine();
}
void CodeEdit::paintLineNumbers(QPaintEvent* event) {
    QPainter painter(gutter_);painter.setFont(font());QTextBlock block=firstVisibleBlock();
    qreal top=blockBoundingGeometry(block).translated(contentOffset()).top();int current=textCursor().blockNumber();
    while(block.isValid()&&top<=event->rect().bottom()) {
        qreal height=blockBoundingRect(block).height();
        if(block.isVisible()&&top+height>=event->rect().top()) {
            auto line=block.layout()->lineAt(0);qreal lineTop=top+(line.isValid()?line.y():0),lineHeight=line.isValid()?line.height():fontMetrics().height();
            if(block.blockNumber()==current&&!isReadOnly())painter.fillRect(0,qRound(lineTop),gutterWidth(),qRound(lineHeight),isDarkTheme()?QColor(255,255,255,18):QColor(0,0,0,24));
            painter.setPen(QColor(isDarkTheme()?"#aaaaaa":"#646464"));painter.drawText(0,qRound(lineTop),gutterWidth()-8,qRound(lineHeight),Qt::AlignRight|Qt::AlignVCenter,QString::number(block.blockNumber()+1));
        }
        top+=height;block=block.next();
    }
}
void CodeEdit::highlightCurrentLine() {
    QList<QTextEdit::ExtraSelection> selections;
    if(!isReadOnly()) {QTextEdit::ExtraSelection s;s.format.setBackground(isDarkTheme()?QColor(255,255,255,18):QColor(0,0,0,24));s.format.setProperty(QTextFormat::FullWidthSelection,true);s.cursor=textCursor();s.cursor.clearSelection();selections.append(s);}
    setExtraSelections(selections);gutter_->update();
}
void CodeEdit::setReadOnly(bool readonly){PlainTextEdit::setReadOnly(readonly);if(gutter_)highlightCurrentLine();}
void CodeEdit::paintEvent(QPaintEvent* event) {
    PlainTextEdit::paintEvent(event);
    if(isReadOnly())return;
    const auto block=textCursor().block();
    const auto line=block.layout()->lineAt(0);
    if(!line.isValid())return;
    const QRectF geometry=blockBoundingGeometry(block).translated(contentOffset());
    const qreal top=geometry.top()+line.y();
    // FullWidthSelection excludes QTextDocument's outer margin. Fill those
    // two strips as well, without painting over text or its selection.
    const qreal margin=document()->documentMargin();
    const qreal leftEnd=qBound(qreal(0),geometry.left()+line.x(),qreal(viewport()->width()));
    // Match QPlainTextEdit's inclusive maxX clip boundary. Painting a whole
    // document margin here would apply the translucent color twice at its edge.
    const qreal documentWidth=document()->documentLayout()->documentSize().width();
    int cursorCorrection=0;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    cursorCorrection=cursorWidth();
#endif
    const int rightStart=qBound(0,int(contentOffset().x()+qMax(qreal(viewport()->width()),documentWidth)-margin+cursorCorrection)+1,viewport()->width());
    QPainter painter(viewport());
    const QColor color=isDarkTheme()?QColor(255,255,255,18):QColor(0,0,0,24);
    painter.fillRect(QRectF(0,top,leftEnd,line.height()),color);
    painter.fillRect(QRectF(rightStart,top,viewport()->width()-rightStart,line.height()),color);
}
void CodeEdit::startHighlighting() {
    if(jobActive_)return;
    const QString text=toPlainText(),lang=language_;
    // Match Python's character count, including surrogate pairs.
    int characters=text.size();for(QChar c:text)if(c.isLowSurrogate())--characters;
    if(characters>1000000){highlighter_->snapshot={};highlighter_->rehighlight();emit highlightingFailed(tr("Highlighting skipped: document exceeds 1,000,000 characters"));return;}
    jobActive_=true;jobVersion_=version_;
    QFutureInterface<CodeLexResult> promise;promise.reportStarted();watcher_.setFuture(promise.future());
    QThreadPool::globalInstance()->start(QRunnable::create([promise,text,lang]() mutable {
        CodeLexResult result;
        try {result=lexCode(text,lang);}
        catch(const std::exception& error){result.error=QString::fromUtf8(error.what());}
        catch(...){result.error=QStringLiteral("Native code highlighting failed");}
        promise.reportResult(result);promise.reportFinished();
    }));
}
void CodeEdit::applyHighlightBatch() {
    if(applyVersion_!=version_){pendingRows_.clear();return;}
    int count=qMin(128,int(pendingRows_.size()));
    for(int i=0;i<count;++i){auto block=document()->findBlockByNumber(pendingRows_[i]);if(block.isValid())highlighter_->rehighlightBlock(block);}
    pendingRows_.remove(0,count);
    if(!pendingRows_.isEmpty())applyTimer_.start(0);else emit highlightingFinished();
}
void CodeEdit::indentSelection(bool backwards) {
    auto cursor=textCursor();int first=document()->findBlock(cursor.selectionStart()).blockNumber();int end=cursor.selectionEnd();auto endBlock=document()->findBlock(end);int last=endBlock.blockNumber();
    if(cursor.hasSelection()&&end==endBlock.position())last=qMax(first,last-1);
    cursor.beginEditBlock();
    for(int row=last;row>=first;--row){auto block=document()->findBlockByNumber(row);QTextCursor edit(block);if(backwards){QString text=block.text();int count=0;if(text.startsWith('\t'))count=1;else while(count<indentSize_&&count<text.size()&&text[count]==' ')++count;for(int n=0;n<count;++n)edit.deleteChar();}else edit.insertText(QString(indentSize_,' '));}
    cursor.endEditBlock();setTextCursor(cursor);
}
void CodeEdit::keyPressEvent(QKeyEvent* event) {
    if(isReadOnly()){PlainTextEdit::keyPressEvent(event);return;}
    if((event->key()==Qt::Key_Tab||event->key()==Qt::Key_Backtab)&&!(event->modifiers()&Qt::ControlModifier)) {
        bool backwards=event->key()==Qt::Key_Backtab||(event->modifiers()&Qt::ShiftModifier);
        if(textCursor().hasSelection()||backwards)indentSelection(backwards);else insertPlainText(QString(indentSize_-textCursor().positionInBlock()%indentSize_,' '));return;
    }
    if((event->key()==Qt::Key_Return||event->key()==Qt::Key_Enter)&&event->modifiers()==Qt::NoModifier) {
        auto cursor=textCursor();QString prefix=cursor.block().text().left(cursor.positionInBlock());int n=0;while(n<prefix.size()&&(prefix[n]==' '||prefix[n]=='\t'))++n;QString indent=prefix.left(n);
        if(prefix.trimmed().endsWith('{')||(language_=="python"&&prefix.trimmed().endsWith(':')))indent+=QString(indentSize_,' ');
        cursor.beginEditBlock();cursor.insertText('\n'+indent);cursor.endEditBlock();setTextCursor(cursor);return;
    }
    PlainTextEdit::keyPressEvent(event);
}
} // namespace qfw
