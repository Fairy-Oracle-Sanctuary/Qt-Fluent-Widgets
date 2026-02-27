#pragma once

#include <QColor>
#include <QFrame>
#include <QIcon>
#include <QLabel>
#include <QList>
#include <QPointer>
#include <QPropertyAnimation>
#include <QVBoxLayout>
#include <QWidget>

#include "common/animation.h"
#include "common/icon.h"
#include "components/widgets/icon_widget.h"

namespace qfw {

class BodyLabel;
class CaptionLabel;

class CardWidget : public QFrame {
    Q_OBJECT
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor)
    Q_PROPERTY(int borderRadius READ borderRadius WRITE setBorderRadius)

public:
    explicit CardWidget(QWidget* parent = nullptr);

    void setClickEnabled(bool enabled);
    bool isClickEnabled() const;

    QColor backgroundColor() const;
    void setBackgroundColor(const QColor& c);

    int borderRadius() const;
    void setBorderRadius(int radius);

signals:
    void clicked();

protected:
    void enterEvent(QEnterEvent* e) override;
    void leaveEvent(QEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void paintEvent(QPaintEvent* e) override;

    virtual QColor normalBackgroundColor() const;
    virtual QColor hoverBackgroundColor() const;
    virtual QColor pressedBackgroundColor() const;

    bool isHover_ = false;
    bool isPressed_ = false;

private:
    void updateBackgroundColor(const QColor& target);

    bool clickEnabled_ = false;
    int borderRadius_ = 5;

    QColor backgroundColor_;
    QPointer<QPropertyAnimation> bgAni_;
};

class SimpleCardWidget : public CardWidget {
    Q_OBJECT

public:
    explicit SimpleCardWidget(QWidget* parent = nullptr);

protected:
    QColor hoverBackgroundColor() const override;
    QColor pressedBackgroundColor() const override;
    void paintEvent(QPaintEvent* e) override;
};

class ElevatedCardWidget : public SimpleCardWidget {
    Q_OBJECT

public:
    explicit ElevatedCardWidget(QWidget* parent = nullptr);

protected:
    void enterEvent(QEnterEvent* e) override;
    void leaveEvent(QEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;

    QColor hoverBackgroundColor() const override;
    QColor pressedBackgroundColor() const override;

private:
    void startElevateAnimation(const QPoint& start, const QPoint& end);

    QPointer<DropShadowAnimation> shadowAni_;
    QPointer<QPropertyAnimation> elevatedAni_;
    QPoint originalPos_;
};

class CardSeparator : public QWidget {
    Q_OBJECT

public:
    explicit CardSeparator(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* e) override;
};

class HeaderCardWidget : public SimpleCardWidget {
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle)

public:
    explicit HeaderCardWidget(QWidget* parent = nullptr);
    explicit HeaderCardWidget(const QString& title, QWidget* parent = nullptr);

    QString title() const;
    void setTitle(const QString& title);

protected:
    virtual void postInit();

    QPointer<QWidget> headerView_;
    QPointer<QLabel> headerLabel_;
    QPointer<CardSeparator> separator_;
    QPointer<QWidget> view_;

    QPointer<QVBoxLayout> vBoxLayout_;
    QPointer<QHBoxLayout> headerLayout_;
    QPointer<QHBoxLayout> viewLayout_;
};

class CardGroupWidget : public QWidget {
    Q_OBJECT

public:
    CardGroupWidget(const QIcon& icon, const QString& title, const QString& content,
                    QWidget* parent = nullptr);
    CardGroupWidget(const FluentIconBase& icon, const QString& title, const QString& content,
                    QWidget* parent = nullptr);
    CardGroupWidget(const QString& iconPath, const QString& title, const QString& content,
                    QWidget* parent = nullptr);

    QString title() const;
    void setTitle(const QString& text);

    QString content() const;
    void setContent(const QString& text);

    QIcon icon() const;
    void setIcon(const QIcon& icon);
    void setIcon(const FluentIconBase& icon);
    void setIcon(const QString& iconPath);

    void setIconSize(const QSize& size);

    void setSeparatorVisible(bool visible);
    bool isSeparatorVisible() const;

    void addWidget(QWidget* widget, int stretch = 0);

private:
    void initWidget();

    QPointer<QVBoxLayout> vBoxLayout_;
    QPointer<QHBoxLayout> hBoxLayout_;
    QPointer<qfw::IconWidget> iconWidget_;
    QPointer<BodyLabel> titleLabel_;
    QPointer<CaptionLabel> contentLabel_;
    QPointer<QVBoxLayout> textLayout_;
    QPointer<CardSeparator> separator_;
};

class GroupHeaderCardWidget : public HeaderCardWidget {
    Q_OBJECT

public:
    using HeaderCardWidget::HeaderCardWidget;

    CardGroupWidget* addGroup(const QIcon& icon, const QString& title, const QString& content,
                              QWidget* widget, int stretch = 0);
    CardGroupWidget* addGroup(const FluentIconBase& icon, const QString& title,
                              const QString& content, QWidget* widget, int stretch = 0);
    CardGroupWidget* addGroup(const QString& iconPath, const QString& title, const QString& content,
                              QWidget* widget, int stretch = 0);

    int groupCount() const;

protected:
    void postInit() override;

private:
    QList<QPointer<CardGroupWidget>> groupWidgets_;
    QPointer<QVBoxLayout> groupLayout_;
};

}  // namespace qfw
