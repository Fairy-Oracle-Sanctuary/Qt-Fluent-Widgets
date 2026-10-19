#pragma once
#include "gallery_interface.h"
#include "components/widgets/chat_widget.h"
#include <QTimer>
namespace qfw {
class BodyLabel;
class ChatDemo : public QWidget {
    Q_OBJECT
public:
    explicit ChatDemo(QWidget* parent=nullptr);
    ChatWidget* chat=nullptr;
protected:
    void hideEvent(QHideEvent* event) override;
private:
    void reset();void stop();void tick();void send(const QString& content);
    QTimer timer_;QString streamId_,answer_;int offset_=0;BodyLabel* status_=nullptr;
};
class ChatInterface : public GalleryInterface {
    Q_OBJECT
public:explicit ChatInterface(QWidget* parent=nullptr);
};
}
