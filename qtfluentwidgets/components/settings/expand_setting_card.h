#pragma once

#include <QAbstractButton>
#include <QColor>
#include <QFrame>
#include <QPointer>
#include <QPropertyAnimation>
#include <QScrollArea>
#include <QVBoxLayout>

#include "components/settings/setting_card.h"

class QHBoxLayout;
class QLabel;

namespace qfw {

class VBoxLayout;

class ExpandButton : public QAbstractButton {
    Q_OBJECT
    Q_PROPERTY(qreal angle READ angle WRITE setAngle)

public:
    explicit ExpandButton(QWidget* parent = nullptr);

    qreal angle() const;
    void setAngle(qreal angle);

    void setExpand(bool isExpand);

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private slots:
    void onClicked();

private:
    void setHover(bool isHover);
    void setPressed(bool isPressed);

    qreal angle_ = 0;
    bool isHover_ = false;
    bool isPressed_ = false;
    QPropertyAnimation* rotateAni_ = nullptr;
};

class SpaceWidget : public QWidget {
    Q_OBJECT

public:
    explicit SpaceWidget(QWidget* parent = nullptr);
};

class HeaderSettingCard : public SettingCard {
    Q_OBJECT

public:
    explicit HeaderSettingCard(const QVariant& icon, const QString& title,
                               const QString& content = {}, QWidget* parent = nullptr);

    void addWidget(QWidget* widget);

    ExpandButton* expandButton() const;

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    ExpandButton* expandButton_ = nullptr;
};

class ExpandBorderWidget : public QWidget {
    Q_OBJECT

public:
    explicit ExpandBorderWidget(QWidget* parent);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
};

class ExpandSettingCard : public QScrollArea {
    Q_OBJECT

public:
    explicit ExpandSettingCard(const QVariant& icon, const QString& title,
                               const QString& content = {}, QWidget* parent = nullptr);

    int headerHeight() const;

    void addWidget(QWidget* widget);

    void setExpand(bool isExpand);
    void toggleExpand();

protected:
    void wheelEvent(QWheelEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

protected slots:
    void onExpandValueChanged();

protected:
    virtual void adjustViewSize();

    bool isExpand_ = false;

    QFrame* scrollWidget_ = nullptr;
    QFrame* view_ = nullptr;
    HeaderSettingCard* card_ = nullptr;

    QVBoxLayout* scrollLayout_ = nullptr;
    QVBoxLayout* viewLayout_ = nullptr;

    SpaceWidget* spaceWidget_ = nullptr;
    ExpandBorderWidget* borderWidget_ = nullptr;

    QPropertyAnimation* expandAni_ = nullptr;
};

class GroupSeparator : public QWidget {
    Q_OBJECT

public:
    explicit GroupSeparator(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
};

class GroupWidget : public QWidget {
    Q_OBJECT

public:
    explicit GroupWidget(const QVariant& icon, const QString& title, const QString& content,
                         QWidget* widget, int stretch = 0, QWidget* parent = nullptr);

    void setTitle(const QString& title);
    void setContent(const QString& content);

    void setIconSize(int width, int height);
    void setIcon(const QVariant& icon);

private:
    SettingIconWidget* iconWidget_;
    QLabel* titleLabel_;
    QLabel* contentLabel_;
    QWidget* widget_;

    QHBoxLayout* hBoxLayout_;
    QVBoxLayout* vBoxLayout_;
};

class ExpandGroupSettingCard : public ExpandSettingCard {
    Q_OBJECT

public:
    explicit ExpandGroupSettingCard(const QVariant& icon, const QString& title,
                                    const QString& content = {}, QWidget* parent = nullptr);

    void addGroupWidget(QWidget* widget);

    GroupWidget* addGroup(const QVariant& icon, const QString& title, const QString& content,
                          QWidget* widget, int stretch = 0);

    void removeGroupWidget(QWidget* widget);

protected:
    void adjustViewSize() override;

private:
    QList<QPointer<QWidget>> widgets_;
};

class SimpleExpandGroupSettingCard : public ExpandGroupSettingCard {
    Q_OBJECT

public:
    using ExpandGroupSettingCard::ExpandGroupSettingCard;

protected:
    void adjustViewSize() override;
};

}  // namespace qfw
