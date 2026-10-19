#include "chat_math.h"
#include <QCache>
#include <QFile>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPainter>
#include <QPainterPath>
#include <QRawFont>
#include <QSet>
#include <QTransform>
#include <QtEndian>
#include <cmath>
#include <stdexcept>
#include <mutex>
#include <cstring>

static void registerChatMathResources() { Q_INIT_RESOURCE(chat_math); }
static void initChatMathResources(){static std::once_flag once;std::call_once(once,registerChatMathResources);}
namespace qfw {
namespace {
// All dimensions are points at 12pt / 72dpi; the final raster is 192dpi/DPR2.
// Box rules follow Matplotlib 3.9.4 _mathtext.py (see bundled notices).
struct Box {
    enum Kind {Placed,Horizontal,Vertical,Centered,Space,Rule} kind=Placed;
    qreal w=0,h=0,d=0;
    qreal shift=0;
    int level=0;
    QPainterPath path;
    QVector<Box> children;
    QVector<QPointF> offsets;
    QString glyphToken,glyphStyle;qreal glyphScale=0;
    qreal lastHeight=-1,lastDepth=-1;
    bool slanted=false,overunder=false,dropsub=false,operatorSpace=false,generatedKern=false;
    Box(){path.setFillRule(Qt::WindingFill);}
};
void add(Box& into,const Box& child,qreal x=0,qreal y=0) {
    into.children.append(child);into.offsets.append(QPointF(x,y));
    into.h=qMax(into.h,child.h-y-child.shift);into.d=qMax(into.d,child.d+y+child.shift);
}
Box scale(Box box,qreal factor) {
    box.path=QTransform::fromScale(factor,factor).map(box.path);
    for(auto& child:box.children)child=scale(child,factor);
    for(auto& offset:box.offsets)offset*=factor;
    box.w*=factor;box.h*=factor;box.d*=factor;box.shift*=factor;box.glyphScale*=factor;
    if(box.lastHeight>=0)box.lastHeight*=factor;if(box.lastDepth>=0)box.lastDepth*=factor;return box;
}
Box shrink(Box box) {
    // A fresh list starts at level zero even when its descendants have already
    // shrunk. Keep each node's size, cached pack metrics and ship-time positions.
    for(auto& child:box.children)child=shrink(child);
    if(++box.level<6) {
        box.path=QTransform::fromScale(.7,.7).map(box.path);
        for(auto& offset:box.offsets)offset*=.7;
        box.w*=.7;box.h*=.7;box.d*=.7;box.shift*=.7;box.glyphScale*=.7;
        if(box.lastHeight>=0)box.lastHeight*=.7;if(box.lastDepth>=0)box.lastDepth*=.7;
    }
    return box;
}
Box space(qreal width){Box b;b.kind=Box::Space;b.w=width;return b;}
Box padding(qreal depth){Box b;b.d=depth;return b;}
Box ruleBox(qreal thickness){Box b;b.kind=Box::Rule;b.h=b.d=thickness/2;return b;}
Box centered(const Box& body,qreal width){Box b;b.kind=Box::Centered;b.w=width;b.h=body.h;b.d=body.d;b.children.append(body);b.offsets.append(QPointF((width-body.w)/2,0));return b;}
Box vertical(const QVector<Box>& children) {
    Box b;b.kind=Box::Vertical;b.children=children;qreal depth=0;
    for(const auto& child:children){
        if(child.kind==Box::Space){b.h+=depth+child.w;depth=0;}
        else {b.h+=depth+child.h;depth=child.d;b.w=qMax(b.w,child.w+child.shift);}
    }b.d=depth;return b;
}
void ship(QPainterPath& path,const Box& box,qreal x=0,qreal y=0) {
    path.addPath(QTransform::fromTranslate(x,y).map(box.path));
    if(box.kind==Box::Horizontal) {
        for(const auto& child:box.children){ship(path,child,x,y+child.shift);x+=child.w;}
    }else if(box.kind==Box::Vertical) {
        y-=box.h;
        for(const auto& child:box.children) {
            if(child.kind==Box::Space){y+=child.w;continue;}
            y+=child.h;
            if(child.kind==Box::Rule){y+=child.d;if(child.h+child.d>0)path.addRect(QRectF(x,y,box.w,child.h+child.d));}
            else {ship(path,child,x+child.shift,y);y+=child.d;}
        }
    }else {
        for(int i=0;i<box.children.size();++i)ship(path,box.children[i],x+box.offsets[i].x(),y+box.offsets[i].y()+box.children[i].shift);
    }
}
Box row(const QVector<Box>& children,const QJsonObject& kerning={}) {
    Box result;result.kind=Box::Horizontal;const Box* previous=nullptr;
    for(const auto& child:children){
        if(previous&&previous->glyphScale>0&&child.glyphScale==previous->glyphScale&&child.glyphStyle==previous->glyphStyle)
        {
            const qreal kern=kerning[child.glyphStyle].toObject()[previous->glyphToken+child.glyphToken].toDouble()*child.glyphScale;
            if(kern!=0){auto adjustment=space(kern);adjustment.generatedKern=true;result.children.append(adjustment);result.offsets.append(QPointF());result.w+=kern;}
        }
        add(result,child,result.w);result.w+=child.w;previous=&child;
        result.lastHeight=child.h;result.lastDepth=child.d;
        result.slanted=child.glyphScale>0&&child.slanted;result.dropsub=child.glyphScale>0&&child.dropsub;
    }return result;
}
[[noreturn]] void invalid(const QString& reason) {throw std::invalid_argument(reason.toStdString());}
QJsonObject json(const QString& name) {
    QFile file(":/qfw/math/"+name);if(!file.open(QIODevice::ReadOnly))invalid("Missing formula resource: "+name);
    QJsonParseError error;auto document=QJsonDocument::fromJson(file.readAll(),&error);
    if(error.error!=QJsonParseError::NoError||!document.isObject())invalid("Invalid formula resource: "+name);
    return document.object();
}
class Resources {
public:
    QJsonObject rules,kerning,unicode,classes,units,hints;
    QByteArray hintData;bool rasterPass=false;
    QHash<QString,QJsonObject> tables;
    QHash<QString,QRawFont> fonts;
    QHash<QString,QPainterPath> outlines;
    QCache<QString,QImage> images{256};
    Resources(){
        initChatMathResources();rules=json("rules.json");kerning=json("kerning.json");unicode=json("unicode.json");
        const auto adjustments=unicode["kerning"].toObject();
        for(auto style=adjustments.begin();style!=adjustments.end();++style) {
            auto pairs=kerning[style.key()].toObject();const auto extra=style.value().toObject();
            for(auto pair=extra.begin();pair!=extra.end();++pair)pairs.insert(pair.key(),pair.value());
            kerning.insert(style.key(),pairs);
        }
        const auto named=json("named_kerning.json");
        for(auto style=named.begin();style!=named.end();++style){auto pairs=kerning[style.key()].toObject();const auto extra=style.value().toObject();for(auto pair=extra.begin();pair!=extra.end();++pair)pairs.insert(pair.key(),pair.value());kerning.insert(style.key(),pairs);}
        units=json("sized_units.json");hints=json("raster_hints.json");QFile hintFile(":/qfw/math/raster_hints.bin");if(!hintFile.open(QIODevice::ReadOnly))invalid("Missing formula hint resource");hintData=hintFile.readAll();
        classes=json("font_classes.json");const auto variants=classes["kerning"].toObject();
        for(const auto& font:QStringList{"it","rm","bf","bfit","tt","cal","bb","frak","scr","regular","sf","default"})
            for(const auto& cls:QStringList{"it","rm","bf","bfit"}){const auto key=font+"@"+cls;kerning.insert(key,variants.contains(key)?variants[key]:kerning[(font=="sf"||font=="default")?"rm":font]);}
    }
    const QJsonObject& table(QString style) {
        style=style.section('@',0,0);
        if(style=="sf"||style=="default")style="rm";
        if(!tables.contains(style))tables.insert(style,json("metrics/"+style+".json"));
        return tables[style];
    }
    bool member(const QString& set,const QString& token) const {
        for(const auto& value:rules[set].toArray())if(value.toString()==token)return true;return false;
    }
    QJsonArray metrics(const QString& style,const QString& token) {
        const auto key=style.contains('@')?style:style+"@it";
        const auto overlay=classes["glyphs"].toObject()[key].toObject();
        if(overlay.contains(token)){const int index=overlay[token].toInt();if(index<0)invalid("Unsupported formula symbol: "+token);return classes["records"].toArray().at(index).toArray();}
        auto result=table(style)["glyphs"].toObject()[token].toArray();
        if(result.isEmpty()&&!token.startsWith('\\')) {
            const auto code=token.toUcs4();
            if(code.size()==1) {
                auto font=style.section('@',0,0);if(font=="sf"||font=="default")font="rm";
                const auto index=unicode["styles"].toObject()[font].toObject().value(QString::number(code[0]));
                if(!index.isUndefined())result=unicode["records"].toArray().at(index.toInt()).toArray();
            }
        }
        if(result.isEmpty())invalid("Unsupported formula symbol: "+token);return result;
    }
    QPainterPath outline(const QJsonArray& metric) {
        const auto file=metric[0].toString();const auto code=uint(metric[1].toInt());
        const auto key=file+":"+QString::number(code);
        if(outlines.contains(key))return outlines[key];
        if(!fonts.contains(file)) {
            QFile data(":/qfw/math/fonts/"+file);if(!data.open(QIODevice::ReadOnly))invalid("Missing formula font: "+file);
            QRawFont font; font.loadFromData(data.readAll(),12,QFont::PreferNoHinting);
            if(!font.isValid())invalid("Invalid formula font: "+file);fonts.insert(file,font);
        }
        QString text;
        if(code<=0xffff)text=QString(QChar(ushort(code)));
        else {const uint value=code-0x10000;text.append(QChar(ushort(0xd800+(value>>10))));text.append(QChar(ushort(0xdc00+(value&1023))));}
        const auto indices=fonts[file].glyphIndexesForString(text);
        if(indices.isEmpty()||!indices[0])invalid("Missing formula glyph: "+QString::number(code));
        auto path=fonts[file].pathForGlyph(indices[0]);outlines.insert(key,path);return path;
    }
    Box glyph(const QString& token,const QString& style,qreal size,bool accent=false) {
        const auto m=metrics(style,token);Box box;
        box.w=m[2].toDouble();box.h=m[5].toDouble();box.d=m[4].toDouble()-box.h;
        box.slanted=m[10].toBool();box.overunder=member("overunder_symbols",token);box.dropsub=member("dropsub_symbols",token);
        qreal x=0,y=-m[11].toDouble();
        if(accent){box.w=m[7].toDouble()-m[6].toDouble();box.h=m[9].toDouble()-m[8].toDouble();box.d=0;x=-m[6].toDouble();y+=m[8].toDouble();}
        box.path=QTransform::fromTranslate(x,y).map(outline(m));
        if(!accent){box.glyphToken=token;box.glyphStyle=style.contains('@')?style:style+"@it";box.glyphScale=1;}
        box=scale(box,size);if(rasterPass&&!accent){box.h=qRound(box.h);box.d=qRound(box.d);}return box;
    }
    Box sizedGlyph(const QString& token,const QString& style,qreal size){
        const auto m=metrics(style,token),u=units[m[0].toString()].toObject()[QString::number(m[1].toInt())].toArray();
        if(u.isEmpty())return glyph(token,style,size);
        auto box=glyph(token,style,1);const qreal em=u[0].toDouble(),sx=qMax(1,qRound(12*size*8))/8.0,sy=qMax(1,qRound(12*size));
        const qreal top=qRound(u[5].toDouble()*sy/em*64)/64.0,bottom=qRound(u[4].toDouble()*sy/em*64)/64.0;
        const qreal offset=m[0].toString()=="cmex10.ttf"?(top-bottom)/2+4*size:0;
        box.path=QTransform::fromScale(sx/12,sy/12).map(outline(m));box.path=QTransform::fromTranslate(0,-offset).map(box.path);
        box.w=qRound(u[1].toDouble()*sx/em*65536)/65536.0;box.h=top+offset;box.d=-bottom-offset;box.glyphScale=size;
        if(rasterPass){
            const auto mapping=hints[m[0].toString()].toObject();const auto code=QString::number(m[1].toInt());
            if(mapping.contains(code)&&sy<=4096){const int index=mapping[code].toInt()+(int(sy)-1)*8;if(index<0||index+8>hintData.size())invalid("Invalid formula hint resource");
                auto read=[&](int position){const quint32 bits=qFromLittleEndian<quint32>(reinterpret_cast<const uchar*>(hintData.constData()+position));float value;std::memcpy(&value,&bits,sizeof(value));return qreal(value);};
                box.h=read(index)+offset;box.d=read(index+4)-offset;
            }else {box.h=qRound(box.h);box.d=qRound(box.d);}
        }return box;
    }
    qreal em(const QString& style,qreal size){return table(style)["em"].toDouble()*size;}
    qreal xheight(const QString& style,qreal size){return table(style)["xheight"].toDouble()*size;}
};
class Parser {
    Resources& res_;QString text_,style_="it@it";int pos_=0,depth_=0; qreal size_=1;bool operatorScript_=false;
    void setStyle(const QString& font){auto cls=style_.section('@',1,1);if(font=="rm"||font=="it"||font=="bf"||font=="bfit")cls=font;style_=font+"@"+cls;}
    void spaces(){while(pos_<text_.size()&&text_[pos_].isSpace())++pos_;}
    bool at(const QString& text)const{return text_.mid(pos_,text.size())==text;}
    QString command(){
        if(pos_>=text_.size())invalid("Expected formula token");
        if(text_[pos_]!='\\') {
            QString result=text_.mid(pos_++,1);
            if(result.at(0).isHighSurrogate()&&pos_<text_.size()&&text_.at(pos_).isLowSurrogate())result+=text_[pos_++];return result;
        }
        const int begin=pos_++;if(pos_>=text_.size())invalid("Incomplete formula command");
        if((text_[pos_]>='a'&&text_[pos_]<='z')||(text_[pos_]>='A'&&text_[pos_]<='Z'))
            while(pos_<text_.size()&&((text_[pos_]>='a'&&text_[pos_]<='z')||(text_[pos_]>='A'&&text_[pos_]<='Z')))++pos_;
        else ++pos_;return text_.mid(begin,pos_-begin);
    }
    Box group(bool required=true){
        spaces();if(!at("{"))invalid("Expected formula group");++pos_;const int start=pos_;
        auto box=sequence("}");if(!at("}"))invalid("Unclosed formula group");
        if(required&&text_.mid(start,pos_-start).trimmed().isEmpty())invalid("Empty formula group");++pos_;return box;
    }
    QString literalGroup(){
        spaces();if(!at("{"))invalid("Expected literal group");++pos_;QString result;
        while(pos_<text_.size()&&!at("}")){if(at("{"))invalid("Nested literal group");auto token=command();if(token.startsWith('\\')&&token.size()==2)token.remove(0,1);result+=token;}
        if(!at("}"))invalid("Unclosed literal group");++pos_;return result;
    }
    Box textBox(const QString& text,QString style){
        if(style=="rm")style="rm@rm";
        QVector<Box> chars;for(int i=0;i<text.size();++i){QString ch=text.mid(i,1);if(text.at(i).isHighSurrogate()&&i+1<text.size())ch+=text.at(++i);chars.append(res_.glyph(ch,style,size_));}return row(chars,res_.kerning);
    }
    Box heightGlyph(const QString& token,qreal height,qreal depth){
        auto alternatives=res_.rules["alternatives"].toObject()[token].toArray();
        if(alternatives.isEmpty())alternatives.append(QJsonArray{style_.section('@',0,0),token});
        Box glyph;QString font,symbol;
        for(const auto& value:alternatives){auto option=value.toArray();font=option[0].toString();symbol=option[1].toString();glyph=res_.sizedGlyph(symbol,font+"@"+style_.section('@',1,1),size_);if(glyph.h+glyph.d>=height+depth-.2*res_.xheight(style_,size_))break;}
        if(font!="0"||alternatives.size()==1){
            if(glyph.h+glyph.d<=0)invalid("Zero-height delimiter");
            const qreal factor=(height+depth)/(glyph.h+glyph.d);
            // FreeType's reference font uses an 8x horizontal hinting factor:
            // even NO_HINTING quantizes the selected size to 1/8px horizontally
            // and a whole pixel vertically. Reconstruct the newly loaded Char
            // rather than geometrically stretching its old pack metrics.
            glyph=res_.sizedGlyph(symbol,font+"@"+style_.section('@',1,1),size_*factor);
            auto box=row({glyph});box.shift=depth-glyph.d;return box;
        }
        return row({glyph});
    }
    Box delimiters(const QString& left,const QVector<Box>& middle,const QString& right){
        Box body=row(middle);QVector<Box> parts;if(left!=".")parts.append(heightGlyph(left,body.h,body.d));parts.append(body);if(right!=".")parts.append(heightGlyph(right,body.h,body.d));return row(parts);
    }
    QString delimiter(){spaces();auto token=command();if(!res_.member("left_delims",token)&&!res_.member("right_delims",token)&&!res_.member("ambi_delims",token))invalid("Invalid formula delimiter: "+token);return token;}
    Box fraction(Box num,Box den,int style,qreal rule,const QString& left={},const QString& right={}){
        for(int i=0;i<style;++i){num=shrink(num);den=shrink(den);}
        const qreal t=.75*size_,w=qMax(num.w,den.w);
        const auto equals=res_.metrics(style_,"=");const qreal axis=(equals[8].toDouble()+equals[9].toDouble())*.5*size_;
        // Vlist baseline shift is denominator.height - (equals midpoint - 3t).
        // The no-rule genfrac variant keeps the same spacer calculation.
        auto stack=vertical({centered(num,w),padding(2*t),ruleBox(rule),padding(2*t),centered(den,w)});
        stack.shift=den.h-axis+3*t;auto box=row({stack,space(2*t)});
        if(!left.isEmpty()||!right.isEmpty())return delimiters(left.isEmpty()?".":left,{box},right.isEmpty()?".":right);return box;
    }
    Box scripts(Box base){
        spaces();Box sub,sup;bool hasSub=false,hasSup=false;int primes=0;
        while(pos_<text_.size()) {
            if(at("_")||at("^")) {
                const bool down=at("_");++pos_;spaces();if((down&&hasSub)||(!down&&hasSup))invalid(down?"Double subscript":"Double superscript");
                if(pos_>=text_.size()||at("_")||at("^")||at("}"))invalid("Missing script");
                auto b=atom();if(down){sub=b;hasSub=true;}else{sup=b;hasSup=true;}
            }else if(at("'")){++primes;++pos_;}else break;spaces();
        }
        if(primes){QVector<Box> chars{hasSup?sup:Box{}};for(int i=0;i<primes;++i)chars.append(res_.glyph("\\prime",style_,size_));sup=row(chars);hasSup=true;}
        if(!hasSub&&!hasSup)return base;
        const qreal t=.75*size_,xh=res_.xheight(style_,size_);
        if(base.overunder){
            sup=shrink(sup);sub=shrink(sub);const qreal w=qMax(base.w,qMax(sup.w,sub.w));QVector<Box> parts;
            if(hasSup){parts.append(centered(sup,w));parts.append(padding(3*t));}parts.append(centered(base,w));
            if(hasSub){parts.append(padding(3*t));parts.append(centered(sub,w));}
            auto stack=vertical(parts);stack.shift=hasSub?sub.h+3*t+base.d:0;return row({stack});
        }
        const qreal lastHeight=base.lastHeight<0?base.h:base.lastHeight,lastDepth=base.lastDepth<0?base.d:base.lastDepth;
        qreal sk=.025*xh,dk=sk;
        if(base.slanted){sk+=.025*xh+.2*(lastHeight-xh*2/3);dk=0;if(base.dropsub){dk=(3*.025-.1)*lastHeight;sk=(3*.025+.1)*lastHeight;}}
        auto lower=shrink(row({space(dk),sub})),upper=shrink(row({space(sk),sup}));
        qreal up=base.dropsub?lastHeight-.4*xh:.7*xh;
        const qreal down=base.dropsub?lastDepth+.4*xh:(hasSup?.5:.3)*xh;
        Box attachment;
        if(hasSup&&hasSub){
            up+=qMax(qreal(0),2*t-((up-upper.d)-(lower.h-down)));
            attachment=vertical({upper,space((up-upper.d)-(lower.h-down)),lower});attachment.shift=down;
        }else if(hasSup){attachment=upper;attachment.shift=-up;}else{attachment=lower;attachment.shift=down;}
        if(!base.dropsub)attachment.w+=.05*xh;
        QVector<Box> parts{base,attachment};
        if(operatorScript_){parts.append(space(res_.em(style_,size_)*.16667));operatorScript_=false;}return row(parts);
    }
    Box atom(){
        spaces();if(++depth_>64)invalid("Formula nesting exceeds 64 levels");
        struct Guard{int& depth;~Guard(){--depth;}}guard{depth_};
        if(at("{")){const auto old=style_;auto out=group(false);style_=old;return out;}
        const int location=pos_;auto token=command();const qreal t=.75*size_;
        if(token=="}"||token=="_"||token=="^")invalid("Unexpected formula token: "+token);
        auto spacings=res_.rules["spaces"].toObject();
        if(spacings.contains(token)){Box space;space.w=res_.em(style_,size_)*spacings[token].toDouble();return space;}
        if(token=="\\hspace"){bool ok=false;const auto number=literalGroup().toDouble(&ok);if(!ok||!std::isfinite(number)||qAbs(number)>4096)invalid("Invalid formula space");Box box;box.w=res_.em(style_,size_)*number;return box;}
        const QString font=token.startsWith("\\math")?token.mid(5):token.mid(1);
        const QSet<QString> fontnames{"rm","cal","it","tt","sf","bf","bfit","default","bb","frak","scr","regular"};
        if(token.startsWith("\\math")&&fontnames.contains(font)){const auto old=style_;setStyle(font);auto out=group(false);style_=old;return out;}
        if(token.startsWith('\\')&&fontnames.contains(font)){setStyle(font);return {};}
        if(token=="\\frac"||token=="\\dfrac"||token=="\\binom"){
            auto num=group(),den=group();return fraction(num,den,token=="\\dfrac"?0:1,token=="\\binom"?0:t,token=="\\binom"?"(":"",token=="\\binom"?")":"");
        }
        if(token=="\\genfrac"){
            const auto left=literalGroup(),right=literalGroup();bool ok=false;auto rule=literalGroup().toDouble(&ok);
            if(!ok||rule<0||rule>100)invalid("Invalid fraction rule");auto style=literalGroup();int value=style.isEmpty()?0:style.toInt(&ok);if(!style.isEmpty()&&(!ok||value<0||value>3))invalid("Invalid fraction style");
            auto num=group(),den=group();return fraction(num,den,value,rule*size_,left,right);
        }
        if(token=="\\sqrt"){
            spaces();Box root;bool index=false;if(at("[")){++pos_;root=sequence("]");if(!at("]"))invalid("Unclosed root index");++pos_;index=true;}
            auto body=group();auto check=heightGlyph("\\__sqrt__",body.h+5*t,body.d);
            const qreal height=check.h-check.shift,depth=check.d+check.shift;
            auto padded=row({space(2*t),body,space(2*t)});const qreal target=height+.72*size_,limitedDepth=qMin(padded.d,depth);
            auto right=vertical({ruleBox(t),space(target-t-padded.h-(padded.d-limitedDepth)),padded});right.h=target;right.d=limitedDepth;
            root=index?shrink(shrink(root)):Box{};if(!index)root.w=check.w*.5;
            auto rootStack=vertical({row({root})});rootStack.shift=-height*.6;
            return row({rootStack,space(-check.w*.5),check,right});
        }
        if(token=="\\left"){
            const auto left=delimiter();QVector<Box> middle;QVector<int> marks;QVector<QString> delimitersFound;
            while(true){spaces();if(pos_>=text_.size())invalid("Missing right delimiter");if(at("\\right")){pos_+=6;break;}
                if(at("\\middle")){pos_+=7;marks.append(middle.size());delimitersFound.append(delimiter());middle.append(Box{});}else middle.append(scripts(atom()));}
            const auto right=delimiter();auto body=row(middle);for(int i=0;i<marks.size();++i)if(delimitersFound[i]!=".")middle[marks[i]]=heightGlyph(delimitersFound[i],body.h,body.d);
            return delimiters(left,middle,right);
        }
        if(token=="\\text")return textBox(literalGroup(),"rm");
        if(token=="\\substack"){
            spaces();if(!at("{"))invalid("Expected substack group");++pos_;QVector<Box> lines;
            while(true){lines.append(sequence("\\\\"));spaces();if(at("}")){++pos_;break;}if(!at("\\\\"))invalid("Unclosed substack");pos_+=2;}
            qreal width=0;for(const auto& line:lines)width=qMax(width,line.w);
            QVector<Box> stack;for(const auto& line:lines){if(!stack.isEmpty())stack.append(padding(2*t));stack.append(centered(line,width));}return row({vertical(stack)});
        }
        if(token=="\\operatorname"){
            auto box=textBox(literalGroup(),"rm");spaces();box.operatorSpace=at("^")||at("_");operatorScript_=box.operatorSpace;
            if(!box.operatorSpace&&!res_.member("left_delims",text_.mid(pos_,1))&&!res_.member("right_delims",text_.mid(pos_,1)))box.w+=res_.em(style_,size_)*.16667;return box;
        }
        if(token=="\\boldsymbol"){
            auto body=group(false);QVector<Box> children;
            for(auto child:body.children){
                if(child.generatedKern)continue;
                if(child.kind==Box::Horizontal&&child.children.size()==1&&child.children[0].glyphScale==0)
                    invalid("Unsupported one-child list inside boldsymbol");
                if(child.glyphScale>0){
                    const auto code=child.glyphToken;
                    const bool latin=code.size()==1&&((code[0]>='a'&&code[0]<='z')||(code[0]>='A'&&code[0]<='Z'));
                    child=res_.glyph(code,(latin||res_.member("small_greek",code.mid(1))?QString("bfit"):QString("bf"))+"@"+child.glyphStyle.section('@',1,1),child.glyphScale);
                }else if(child.kind==Box::Placed&&child.children.size()==1&&child.children[0].glyphScale>0){
                    auto& glyph=child.children[0];const auto replacement=res_.glyph(glyph.glyphToken,"bf@"+glyph.glyphStyle.section('@',1,1),glyph.glyphScale);
                    child.w+=replacement.w-glyph.w;glyph=replacement;child.h=glyph.h;child.d=glyph.d;
                }
                children.append(child);
            }return row(children,res_.kerning);
        }
        if(token=="\\overline"){
            auto body=row({group()});const qreal height=body.h+3*t+.72*size_;
            auto box=vertical({ruleBox(t),space(height-t-body.h),body});box.h=height;return row({box});
        }
        if(token=="\\overset"||token=="\\underset"){
            auto annotation=shrink(group(false)),body=group(false);const qreal w=qMax(annotation.w,body.w);
            if(token=="\\underset"){auto box=vertical({centered(body,w),padding(3*t),centered(annotation,w)});box.shift=body.d+3*t+annotation.h;return box;}
            return vertical({centered(annotation,w),padding(3*t),centered(body,w)});
        }
        auto accents=res_.rules["accents"].toObject();const auto name=token.mid(1);
        if(token.startsWith('\\')&&(accents.contains(name)||res_.member("wide_accents",name))){
            auto body=atom();Box accent;
            if(res_.member("wide_accents",name)) {
                auto options=res_.rules["alternatives"].toObject()[name].toArray();
                for(const auto& option:options){auto pair=option.toArray();accent=res_.glyph(pair[1].toString(),pair[0].toString(),size_,true);if(accent.w>=body.w)break;}
                if(accent.w<=0)invalid("Invalid wide accent");accent=scale(accent,body.w/accent.w);
            }else accent=res_.glyph(accents[name].toString(),style_,size_,true);
            if(name=="mathring")accent=shrink(shrink(accent));
            return vertical({centered(row({space(body.w/4),accent}),body.w),padding(2*t),row({body})});
        }
        if(token.startsWith('\\')&&res_.member("function_names",name)){
            auto box=textBox(name,"rm");box.overunder=res_.member("overunder_functions",name);spaces();box.operatorSpace=at("_")||at("^");operatorScript_=box.operatorSpace;
            if(!box.operatorSpace&&!box.overunder&&!res_.member("left_delims",text_.mid(pos_,1))&&!res_.member("right_delims",text_.mid(pos_,1)))box.w+=res_.em(style_,size_)*.16667;return box;
        }
        if(token=="-")token=QString(QChar(0x2212));
        if(token=="\\%"||token=="\\$"||token=="\\{"||token=="\\}"||token=="\\["||token=="\\]"||token=="\\_"||token=="\\|")token=token.mid(1);
        auto glyph=res_.glyph(token,style_,size_);
        bool spaced=res_.member("binary_operators",token)||res_.member("relation_symbols",token)||res_.member("arrow_symbols",token);
        if(operatorScript_)spaced=false;
        int previous=location-1;while(previous>=0&&text_[previous].isSpace())--previous;
        if(res_.member("binary_operators",token)&&(previous<0||text_[previous]=='{'||res_.member("left_delims",text_.mid(previous,1))))spaced=false;
        const qreal gap=res_.em(style_,size_)*.2;
        if(spaced){Box box;box.w=glyph.w+2*gap;add(box,glyph,gap);return box;}
        if(res_.member("punctuation_symbols",token)){
            int next=pos_;while(next<text_.size()&&text_[next].isSpace())++next;
            const bool decimal=token=="."&&previous>=0&&next<text_.size()&&text_[previous].isDigit()&&text_[next].isDigit();
            const bool braceComma=token==","&&previous>=0&&text_[previous]=='{'&&next<text_.size()&&text_[next]=='}';
            if(!decimal&&!braceComma)glyph.w+=gap;
        }
        return glyph;
    }
    Box sequence(const QString& terminator={}){
        const auto old=style_;QVector<Box> children;while(true){spaces();if(pos_>=text_.size()||at("}")||(!terminator.isEmpty()&&at(terminator)))break;children.append(scripts((at("_")||at("^"))?Box{}:atom()));}
        style_=old;return row(children,res_.kerning);
    }
public:
    Parser(Resources& resources,const QString& text,qreal size=1):res_(resources),text_(text),size_(size){}
    Box parse(){auto box=sequence();if(pos_!=text_.size())invalid("Unexpected formula suffix");if(box.h+box.d<=0||box.w<=0)invalid("Formula has no visible extent");return box;}
};
}
QImage renderFormula(const QString& expression,const QColor& color,int pointSize) {
    if(!qobject_cast<QGuiApplication*>(QCoreApplication::instance()))invalid("Formula rendering requires a QGuiApplication");
    if(expression.size()>8192||expression.toUcs4().size()>4096)invalid("Formula exceeds the 4096-character rendering limit");
    for(int i=0;i<expression.size();++i) {
        if(expression.at(i).isHighSurrogate()) {
            if(i+1>=expression.size()||!expression.at(i+1).isLowSurrogate())invalid("Unpaired formula surrogate");
            ++i;
        }else if(expression.at(i).isLowSurrogate())invalid("Unpaired formula surrogate");
    }
    if(pointSize<1||pointSize>144||!color.isValid())invalid("Invalid formula size or color");
    // QRawFont is local to the thread where it was loaded, even behind a mutex.
    // Worker renderers therefore own their fonts, outlines and bounded cache.
    thread_local Resources resources;
    const auto key=expression+QChar(0)+QString::number(color.rgba())+":"+QString::number(pointSize);
    if(auto* cached=resources.images.object(key))return *cached;
    resources.rasterPass=false;auto box=Parser(resources,expression.trimmed(),pointSize/12.0).parse();
    const qreal width=std::ceil(box.w),height=std::ceil(box.h),depth=std::ceil(box.d),dpiScale=192.0/72;
    const qreal rasterWidth=std::floor(width*dpiScale),rasterHeight=std::floor((height+depth)*dpiScale);
    if(!std::isfinite(rasterWidth+rasterHeight)||rasterWidth>8192||rasterHeight>8192||rasterWidth*rasterHeight>16777216)invalid("Formula raster exceeds rendering limits");
    QImage image(qMax(1,int(rasterWidth)),qMax(1,int(rasterHeight)),QImage::Format_ARGB32_Premultiplied);image.fill(Qt::transparent);
    // Figure extents come from the 72dpi vector pass, while Agg re-parses at
    // the output DPI. This matters for nested auto-sized radicals.
    resources.rasterPass=true;auto rasterBox=Parser(resources,expression.trimmed(),pointSize/12.0*dpiScale).parse();
    QPainterPath path;path.setFillRule(Qt::WindingFill);ship(path,rasterBox);
    QPainter painter(&image);painter.setRenderHint(QPainter::Antialiasing);painter.translate(0,height*dpiScale);painter.setPen(Qt::NoPen);painter.setBrush(color);painter.drawPath(path);painter.end();
    image.setDevicePixelRatio(2);resources.images.insert(key,new QImage(image),qMax(1,int((image.sizeInBytes()+131071)/131072)));return image;
}
}
