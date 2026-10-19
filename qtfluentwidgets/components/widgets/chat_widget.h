#pragma once
#include "chat_content.h"
#include "common/icon.h"
#include <QHash>
#include <QSet>
#include <QTimer>
#include <QWidget>
#include <optional>

class QSplitter;class QVBoxLayout;class QHBoxLayout;class QLabel;class QPlainTextEdit;
namespace qfw {
class ScrollArea;class PushButton;class PrimaryPushButton;class ChatRow;
struct ChatMessage {QString id,role,content,name;bool streaming=false;QString kind="text";};
class ChatWidget : public QWidget {
    Q_OBJECT
public:
    explicit ChatWidget(QWidget* parent=nullptr);
    QSize sizeHint() const override {return {800,650};}
    QString addMessage(const QString& content={},const QString& role="assistant",const QString& name={},const QImage& avatar={},const QString& messageId={},bool streaming=false);
    QString addImageMessage(const QImage& image,const QString& role="assistant",const QString& name={},const QImage& avatar={},const QString& messageId={});
    QString addImageMessage(const QPixmap& image,const QString& role="assistant",const QString& name={},const QImage& avatar={},const QString& messageId={});
    QString addImageMessage(const QString& path,const QString& role="assistant",const QString& name={},const QImage& avatar={},const QString& messageId={});
    QString addWidgetMessage(QWidget* widget,const QString& role="assistant",const QString& name={},const QImage& avatar={},const QString& messageId={});
    ChatMessage message(const QString& id) const;
    QList<ChatMessage> messages() const;
    void removeMessage(const QString& id);
    void clear();
    void setMessageStreaming(const QString& id,bool streaming);
    void finishMessage(const QString& id){setMessageStreaming(id,false);}
    TransparentToolButton* addToolButton(const QIcon& icon,const QString& toolTip={},std::function<void()> callback={},const QString& side="left");
    TransparentToolButton* addToolButton(FluentIconEnum icon,const QString& toolTip={},std::function<void()> callback={},const QString& side="left");
    void addToolWidget(QWidget* widget,const QString& side="left");
    void removeToolWidget(QWidget* widget);
    void setMessageToolBarEnabled(bool enabled=true);
    bool isMessageToolBarEnabled() const {return messageTools_;}
    void addMessageAction(const QString& id,const QIcon& icon,const QString& toolTip={},std::function<void(const QString&)> callback={},std::optional<QStringList> roles={},std::optional<QStringList> kinds={});
    void addMessageAction(const QString& id,FluentIconEnum icon,const QString& toolTip={},std::function<void(const QString&)> callback={},std::optional<QStringList> roles={},std::optional<QStringList> kinds={});
    void removeMessageAction(const QString& id);
    void clearMessageActions();
    QWidget* messageToolBar(const QString& id) const;
    TransparentToolButton* messageActionButton(const QString& messageId,const QString& actionId) const;
    void setMathEnabled(bool enabled);
    void setFormulaRenderer(ChatFormulaRenderer renderer);
    void setMaximumBubbleWidth(int width);
    void setBubbleColors(const QColor& incomingLight,const QColor& incomingDark,const QColor& outgoingLight,const QColor& outgoingDark);
    void setComposerVisible(bool visible);
    void setSendOnEnter(bool enabled);
    void setBusy(bool busy);
    QSplitter* splitter=nullptr;ScrollArea* scrollArea=nullptr;QWidget* messageContainer=nullptr;
    QVBoxLayout* messageLayout=nullptr;QWidget* jumpButton=nullptr;QWidget* composer=nullptr;
    QWidget* toolBar=nullptr;QWidget* leftToolBar=nullptr;QWidget* rightToolBar=nullptr;
    QHBoxLayout* leftToolLayout=nullptr;QHBoxLayout* rightToolLayout=nullptr;
    QPlainTextEdit* inputEdit=nullptr;QLabel* shortcutLabel=nullptr;
    PushButton* stopButton=nullptr;PrimaryPushButton* sendButton=nullptr;
public slots:
    void appendText(const QString& id,const QString& delta);
    void setMessageText(const QString& id,const QString& content);
    void send();
    void scrollToBottom();
signals:
    void sendRequested(const QString& content);void stopRequested();
    void messageAdded(const QString& id);void messageChanged(const QString& id);void messageRemoved(const QString& id);
    void linkClicked(const QUrl& url);void imageClicked(const QString& id);void renderWarning(const QString& error);
    void messageActionTriggered(const QString& actionId,const QString& messageId);
protected:
    bool event(QEvent* event) override;
    bool eventFilter(QObject* watched,QEvent* event) override;
private:
    friend class ChatRow;
    struct Action {QString id;QIcon icon;QString toolTip;std::function<void(const QString&)> callback;std::optional<QStringList> roles,kinds;int fluent=-1,thumb=-1;
        bool accepts(const ChatMessage& m) const{return (!roles||roles->contains(m.role))&&(!kinds||kinds->contains(m.kind));}};
    QString add(const ChatMessage& message,const QImage& avatar,const QImage& image={},QWidget* widget=nullptr);
    ChatRow* row(const QString& id) const;
    void triggerAction(const QString& id,const QString& actionId);
    void syncActions();void flush();void fitRows();void followBottom();void applyTheme();void refreshContent();void updateSendButton();
    QColor bubbleColor(bool outgoing) const;
    QHash<QString,ChatRow*> rows_;QStringList order_;QVector<Action> actions_;QSet<QString> dirty_;QTimer timer_;
    QList<int> composerSizes_;ChatFormulaRenderer renderer_;QVector<QColor> bubbleColors_;
    int maximumBubbleWidth_=680,queuedDepth_=0;bool mathEnabled_=true,following_=true,busy_=false,messageTools_=false;
};
}
