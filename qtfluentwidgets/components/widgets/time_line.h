#pragma once

#include <QPointer>
#include <QVBoxLayout>

#include "components/widgets/card_widget.h"
#include "components/widgets/info_bar.h"
#include "components/widgets/label.h"

namespace qfw {
class TimeLineItem : public CardWidget {
    Q_OBJECT
public:
    explicit TimeLineItem(const QString& text, InfoBarIconEnum icon = InfoBarIconEnum::Information,
                          QWidget* parent = nullptr);
    QString text() const;
    void setText(const QString& text);
    InfoBarIconEnum icon() const;
    void setIcon(InfoBarIconEnum icon);
    void setClickEnabled(bool enabled);
    QWidget* iconWidget = nullptr;
    BodyLabel* textLabel = nullptr;

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    void resetPress(bool animate = false);
    InfoBarIconEnum icon_;
    bool mousePending_ = false;
};

class TimeLineGroup : public QWidget {
    Q_OBJECT
public:
    explicit TimeLineGroup(const QString& title,
                           InfoBarIconEnum icon = InfoBarIconEnum::Information,
                           QWidget* parent = nullptr);
    ~TimeLineGroup() override;
    QString title() const;
    void setTitle(const QString& title);
    InfoBarIconEnum icon() const;
    void setIcon(InfoBarIconEnum icon);
    TimeLineItem* addItem(const QString& text, InfoBarIconEnum icon = InfoBarIconEnum::Information);
    void removeItem(TimeLineItem* item);
    QList<TimeLineItem*> items() const;
    QWidget* header = nullptr;
    QWidget* iconWidget = nullptr;
    StrongBodyLabel* titleLabel = nullptr;
    QWidget* cards = nullptr;
    QVBoxLayout* cardsLayout = nullptr;
    QVBoxLayout* layoutBox = nullptr;

protected:
    void paintEvent(QPaintEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    void forgetItem(TimeLineItem* item);
    QList<QPointer<TimeLineItem>> items_;
    InfoBarIconEnum icon_;
};

class TimeLineWidget : public QWidget {
    Q_OBJECT
public:
    explicit TimeLineWidget(QWidget* parent = nullptr);
    ~TimeLineWidget() override;
    TimeLineGroup* addGroup(const QString& title,
                            InfoBarIconEnum icon = InfoBarIconEnum::Information);
    void removeGroup(TimeLineGroup* group);
    void clear();
    QList<TimeLineGroup*> groups() const;
    QSize sizeHint() const override;
    QVBoxLayout* vBoxLayout = nullptr;

private:
    QList<QPointer<TimeLineGroup>> groups_;
};
}  // namespace qfw
