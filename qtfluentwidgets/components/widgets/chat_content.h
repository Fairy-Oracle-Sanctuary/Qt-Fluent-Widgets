#pragma once
#include <QFrame>
#include <QImage>
#include <QTextBrowser>
#include <functional>

class QPlainTextEdit;
class QResizeEvent;
namespace qfw {
class TransparentToolButton;
using ChatFormulaRenderer = std::function<QImage(const QString&, const QColor&, int)>;
struct ChatContentBlock { QString kind, language, content; };
QVector<ChatContentBlock> splitMessage(const QString& text);

class ChatMarkdownBlock : public QTextBrowser {
    Q_OBJECT
public:
    explicit ChatMarkdownBlock(QWidget* parent=nullptr);
    QVariant loadResource(int type,const QUrl& name) override;
    void setContent(const QString& text, bool mathEnabled=true, const ChatFormulaRenderer& renderer={}, bool force=false);
signals:
    void warning(const QString& message);
protected:
    void resizeEvent(QResizeEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
private:
    void fitHeight();
    QString text_;quint64 revision_=0;bool initialized_=false,fitting_=false,mathEnabled_=true,customRenderer_=false;
};
class ChatCodeBlock : public QFrame {
    Q_OBJECT
public:
    explicit ChatCodeBlock(const QString& language,QWidget* parent=nullptr);
    void setContent(const QString& text,bool force=false);
    QPlainTextEdit* editor=nullptr;
    TransparentToolButton* copyButton=nullptr;
    QString language() const {return language_;}
signals:
    void warning(const QString& message);
private:
    QString language_,text_;quint64 revision_=0;bool initialized_=false;
};
}
