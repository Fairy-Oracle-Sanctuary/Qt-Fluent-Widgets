#pragma once

#include <QImage>
#include <QMargins>
#include <QObject>
#include <QParallelAnimationGroup>
#include <QPixmap>
#include <QPointer>
#include <QPropertyAnimation>
#include <QString>
#include <QVBoxLayout>
#include <QVariant>
#include <QWidget>

#include "common/animation.h"
#include "components/widgets/scroll_area.h"

class QPaintEvent;
class QEnterEvent;
class QMouseEvent;

namespace qfw {

class NavigationTreeWidget;
class AvatarWidget;

class NavigationWidget : public QWidget {
    Q_OBJECT

public:
    static constexpr int EXPAND_WIDTH = 312;

    explicit NavigationWidget(bool isSelectable, QWidget* parent = nullptr);

    bool isCompacted() const { return isCompacted_; }
    bool isSelected() const { return isSelected_; }
    bool isSelectable() const { return isSelectable_; }

    int nodeDepth() const { return nodeDepth_; }
    void setNodeDepth(int depth) { nodeDepth_ = depth; }

    virtual void setCompacted(bool compacted);
    virtual void setSelected(bool selected);

    QColor textColor() const;

    void setLightTextColor(const QColor& color);
    void setDarkTextColor(const QColor& color);
    void setTextColor(const QColor& light, const QColor& dark);

    virtual void setAboutSelected(bool selected);

    virtual QRectF indicatorRect() const;

    void setIndicatorColor(const QColor& light, const QColor& dark);
    QColor lightIndicatorColor() const { return lightIndicatorColor_; }
    QColor darkIndicatorColor() const { return darkIndicatorColor_; }

signals:
    void clicked(bool triggeredByUser);
    void selectedChanged(bool selected);

protected:
    void enterEvent(QEnterEvent* e) override;
    void leaveEvent(QEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;

    virtual QMargins marginsForPaint() const;

protected:
    bool isCompacted_ = true;
    bool isSelected_ = false;
    bool isPressed_ = false;
    bool isEnter_ = false;
    bool isAboutSelected_ = false;
    bool isSelectable_ = false;

    NavigationTreeWidget* treeParent_ = nullptr;
    int nodeDepth_ = 0;

    QColor lightTextColor_ = QColor(0, 0, 0);
    QColor darkTextColor_ = QColor(255, 255, 255);

    QColor lightIndicatorColor_;
    QColor darkIndicatorColor_;
};

class NavigationPushButton : public NavigationWidget {
    Q_OBJECT

public:
    NavigationPushButton(const QVariant& icon, const QString& text, bool isSelectable,
                         QWidget* parent = nullptr);

    QString text() const { return text_; }
    void setText(const QString& text);

    QVariant iconVariant() const { return icon_; }
    void setIcon(const QVariant& icon);

protected:
    void paintEvent(QPaintEvent* e) override;

    virtual bool canDrawIndicator() const;

private:
    QVariant icon_;
    QString text_;
};

class NavigationToolButton : public NavigationPushButton {
    Q_OBJECT

public:
    explicit NavigationToolButton(const QVariant& icon, QWidget* parent = nullptr);

    void setCompacted(bool compacted) override;
};

class NavigationSeparator : public NavigationWidget {
    Q_OBJECT

public:
    explicit NavigationSeparator(QWidget* parent = nullptr);

    void setCompacted(bool compacted) override;

protected:
    void paintEvent(QPaintEvent* e) override;
};

class NavigationItemHeader : public NavigationWidget {
    Q_OBJECT

public:
    explicit NavigationItemHeader(const QString& text, QWidget* parent = nullptr);

    QString text() const { return text_; }
    void setText(const QString& text);

    void setCompacted(bool compacted) override;

protected:
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void enterEvent(QEnterEvent* e) override;
    void leaveEvent(QEvent* e) override;

    void paintEvent(QPaintEvent* e) override;

private slots:
    void onHeightChanged(const QVariant& value);

private:
    QString text_;
    int targetHeight_ = 30;
    QPropertyAnimation* heightAni_ = nullptr;
};

class NavigationAvatarWidget : public NavigationWidget {
    Q_OBJECT

public:
    explicit NavigationAvatarWidget(const QString& name, QWidget* parent = nullptr);

    QString name() const { return name_; }
    void setName(const QString& name);

    void setAvatar(const QString& avatarPath);
    void setAvatar(const QPixmap& avatar);
    void setAvatar(const QImage& avatar);

protected:
    void paintEvent(QPaintEvent* e) override;

private:
    void drawText(QPainter* painter);
    void drawBackground(QPainter* painter);

protected:
    QString name_;
    AvatarWidget* avatar_ = nullptr;
};

class NavigationUserCard : public NavigationAvatarWidget {
    Q_OBJECT
    Q_PROPERTY(float textOpacity READ textOpacity WRITE setTextOpacity)
    Q_PROPERTY(QColor subtitleColor READ subtitleColor WRITE setSubtitleColor)

public:
    explicit NavigationUserCard(QWidget* parent = nullptr);

    void setAvatarIcon(const QVariant& icon);
    void setAvatarBackgroundColor(const QColor& light, const QColor& dark);

    QString title() const { return title_; }
    void setTitle(const QString& title);

    QString subtitle() const { return subtitle_; }
    void setSubtitle(const QString& subtitle);

    void setTitleFontSize(int size);
    void setSubtitleFontSize(int size);

    void setAnimationDuration(int duration);

    void setCompacted(bool compacted) override;

    float textOpacity() const { return textOpacity_; }
    void setTextOpacity(float value);

    QColor subtitleColor() const { return subtitleColor_; }
    void setSubtitleColor(const QColor& color);

protected:
    void paintEvent(QPaintEvent* e) override;

private:
    void drawText(QPainter* painter);
    void updateAvatarPosition();

private:
    QString title_;
    QString subtitle_;
    int titleSize_ = 14;
    int subtitleSize_ = 12;
    QColor subtitleColor_;

    float textOpacity_ = 0.0f;
    int animationDuration_ = 250;
    QParallelAnimationGroup* animationGroup_ = nullptr;
    QPropertyAnimation* radiusAni_ = nullptr;
    QPropertyAnimation* opacityAni_ = nullptr;
};

class NavigationFlyoutMenu : public ScrollArea {
    Q_OBJECT

public:
    explicit NavigationFlyoutMenu(NavigationTreeWidget* tree, QWidget* parent = nullptr);

    QList<NavigationTreeWidget*> visibleTreeNodes() const;

signals:
    void expanded();

private:
    void initNode(NavigationTreeWidget* root);
    void adjustViewSize(bool emitSignal = true);
    int suitableWidth() const;

private:
    QWidget* view_ = nullptr;
    NavigationTreeWidget* treeWidget_ = nullptr;
    QList<NavigationTreeWidget*> treeChildren_;
    QVBoxLayout* vBoxLayout_ = nullptr;
};

class NavigationTreeItem : public NavigationPushButton {
    Q_OBJECT

    Q_PROPERTY(float arrowAngle READ arrowAngle WRITE setArrowAngle)

public:
    NavigationTreeItem(const QVariant& icon, const QString& text, bool isSelectable,
                       QWidget* parent = nullptr);

    void setExpanded(bool expanded);

    float arrowAngle() const { return arrowAngle_; }
    void setArrowAngle(float angle);

public:
    QMargins marginsForPaint() const override;

signals:
    void itemClicked(bool triggeredByUser, bool clickArrow);

protected:
    void mouseReleaseEvent(QMouseEvent* e) override;
    void paintEvent(QPaintEvent* e) override;

    bool canDrawIndicator() const override;

private:
    void drawDropDownArrow();

    float arrowAngle_ = 0.0f;
    QPropertyAnimation* rotateAni_ = nullptr;
};

class NavigationTreeWidgetBase : public NavigationWidget {
    Q_OBJECT

public:
    using NavigationWidget::NavigationWidget;

    virtual void addChild(NavigationTreeWidgetBase* child) = 0;
    virtual void insertChild(int index, NavigationTreeWidgetBase* child) = 0;
    virtual void removeChild(NavigationTreeWidgetBase* child) = 0;

    virtual bool isRoot() const { return true; }
    virtual bool isLeaf() const { return true; }

    virtual void setExpanded(bool expanded, bool ani = false) = 0;
    virtual QList<NavigationTreeWidgetBase*> childItems() const = 0;

    virtual void setRememberExpandState(bool remember) = 0;
    virtual void saveExpandState() = 0;
    virtual void restoreExpandState(bool ani = true) = 0;
};

class NavigationTreeWidget : public NavigationTreeWidgetBase {
    Q_OBJECT

public:
    NavigationTreeWidget(const QVariant& icon, const QString& text, bool isSelectable,
                         QWidget* parent = nullptr);

    int nodeDepth() const { return nodeDepth_; }
    NavigationTreeItem* itemWidget() const { return itemWidget_; }

    void addChild(NavigationTreeWidgetBase* child) override;
    void insertChild(int index, NavigationTreeWidgetBase* child) override;
    void removeChild(NavigationTreeWidgetBase* child) override;

    QList<NavigationTreeWidgetBase*> childItems() const override;

    QString text() const;
    void setText(const QString& text);

    QVariant iconVariant() const;
    void setIcon(const QVariant& icon);

    void setFont(const QFont& font);

    NavigationTreeWidget* clone() const;

    int suitableWidth() const;

    void setExpanded(bool expanded, bool ani = false) override;

    bool isRoot() const override;
    bool isLeaf() const override;

    void setSelected(bool selected) override;
    void setCompacted(bool compacted) override;
    void setAboutSelected(bool selected) override;

    void setRememberExpandState(bool remember) override;
    void saveExpandState() override;
    void restoreExpandState(bool ani = true) override;

signals:
    void expanded();

private slots:
    void onClicked(bool triggerByUser, bool clickArrow);

private:
    void initWidget();

private:
    QList<NavigationTreeWidget*> treeChildren_;
    bool isExpanded_ = false;
    QVariant icon_;

    bool rememberExpandState_ = false;
    bool wasExpanded_ = false;

    NavigationTreeItem* itemWidget_ = nullptr;
    QVBoxLayout* vBoxLayout_ = nullptr;
    QPropertyAnimation* expandAni_ = nullptr;
};

class NavigationIndicator : public QWidget {
    Q_OBJECT

public:
    explicit NavigationIndicator(QWidget* parent = nullptr);

    void startAnimation(const QRectF& startRect, const QRectF& endRect, bool useCrossFade = false);
    void stopAnimation();

    void setIndicatorColor(const QColor& light, const QColor& dark);

signals:
    void aniFinished();

protected:
    void paintEvent(QPaintEvent* e) override;

private:
    QColor lightColor_;
    QColor darkColor_;
    qfw::ScaleSlideAnimation* scaleSlideAni_ = nullptr;
};

}  // namespace qfw
