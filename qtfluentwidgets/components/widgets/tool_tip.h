#pragma once

#include <QFrame>
#include <QLabel>
#include <QObject>
#include <QPoint>
#include <QRect>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include <QAbstractItemView>
#include <QStyleOptionViewItem>
#include <QHelpEvent>

class QHBoxLayout;

namespace qfw {

enum class ToolTipPosition {
    Top = 0,
    Bottom = 1,
    Left = 2,
    Right = 3,
    TopLeft = 4,
    TopRight = 5,
    BottomLeft = 6,
    BottomRight = 7
};

enum class ItemViewToolTipType {
    List = 0,
    Table = 1
};

class ToolTip : public QFrame {
    Q_OBJECT

public:
    explicit ToolTip(const QString& text = QString(), QWidget* parent = nullptr);

    QString text() const;
    void setText(const QString& text);

    int duration() const;
    void setDuration(int duration);

    void adjustPos(QWidget* widget, ToolTipPosition position);

protected:
    void showEvent(QShowEvent* e) override;
    void hideEvent(QHideEvent* e) override;

    virtual QFrame* createContainer();

private:
    void initUi();
    void setQss();

    QString text_;
    int duration_ = 1000;

    QFrame* container_ = nullptr;
    QLabel* label_ = nullptr;
    QTimer* timer_ = nullptr;
    QPropertyAnimation* opacityAni_ = nullptr;
    QGraphicsDropShadowEffect* shadowEffect_ = nullptr;
    QHBoxLayout* layout_ = nullptr;
    QHBoxLayout* containerLayout_ = nullptr;
};

class ToolTipPositionManager {
public:
    virtual ~ToolTipPositionManager() = default;

    QPoint position(ToolTip* tooltip, QWidget* parent);
    virtual QPoint _pos(ToolTip* tooltip, QWidget* parent) = 0;

    static ToolTipPositionManager* make(ToolTipPosition position);
};

class TopToolTipManager : public ToolTipPositionManager {
public:
    QPoint _pos(ToolTip* tooltip, QWidget* parent) override;
};

class BottomToolTipManager : public ToolTipPositionManager {
public:
    QPoint _pos(ToolTip* tooltip, QWidget* parent) override;
};

class LeftToolTipManager : public ToolTipPositionManager {
public:
    QPoint _pos(ToolTip* tooltip, QWidget* parent) override;
};

class RightToolTipManager : public ToolTipPositionManager {
public:
    QPoint _pos(ToolTip* tooltip, QWidget* parent) override;
};

class TopRightToolTipManager : public ToolTipPositionManager {
public:
    QPoint _pos(ToolTip* tooltip, QWidget* parent) override;
};

class TopLeftToolTipManager : public ToolTipPositionManager {
public:
    QPoint _pos(ToolTip* tooltip, QWidget* parent) override;
};

class BottomRightToolTipManager : public ToolTipPositionManager {
public:
    QPoint _pos(ToolTip* tooltip, QWidget* parent) override;
};

class BottomLeftToolTipManager : public ToolTipPositionManager {
public:
    QPoint _pos(ToolTip* tooltip, QWidget* parent) override;
};

class ItemViewToolTipManager : public ToolTipPositionManager {
public:
    explicit ItemViewToolTipManager(const QRect& itemRect = QRect());

    QPoint _pos(ToolTip* tooltip, QWidget* view) override;

    static ItemViewToolTipManager* make(ItemViewToolTipType tipType, const QRect& itemRect);

protected:
    QRect itemRect_;
};

class TableItemToolTipManager : public ItemViewToolTipManager {
public:
    explicit TableItemToolTipManager(const QRect& itemRect = QRect());

    QPoint _pos(ToolTip* tooltip, QWidget* view) override;
};

class ToolTipFilter : public QObject {
    Q_OBJECT

public:
    explicit ToolTipFilter(QWidget* parent, int showDelay = 300, ToolTipPosition position = ToolTipPosition::Top);

    bool eventFilter(QObject* obj, QEvent* e) override;

    void setToolTipDelay(int delay);
    int toolTipDelay() const;

    void showToolTip();
    void hideToolTip();

protected:
    virtual ToolTip* createToolTip();
    virtual bool canShowToolTip();

    bool isEnter_ = false;
    ToolTip* tooltip_ = nullptr;
    int tooltipDelay_ = 300;
    ToolTipPosition position_;
    QTimer* timer_ = nullptr;
};

class ItemViewToolTip : public ToolTip {
    Q_OBJECT

public:
    using ToolTip::ToolTip;

    void adjustPos(QAbstractItemView* view, const QRect& itemRect, ItemViewToolTipType tooltipType);
};

class ItemViewToolTipDelegate : public ToolTipFilter {
    Q_OBJECT

public:
    explicit ItemViewToolTipDelegate(QAbstractItemView* parent, int showDelay = 300,
                                     ItemViewToolTipType tooltipType = ItemViewToolTipType::Table);

    bool eventFilter(QObject* obj, QEvent* e) override;

    void setText(const QString& text);
    QString text() const;

    void setToolTipDuration(int duration);
    int toolTipDuration() const;

    bool helpEvent(QHelpEvent* event, QAbstractItemView* view, const QStyleOptionViewItem& option,
                   const QModelIndex& index);

protected:
    ToolTip* createToolTip() override;
    bool canShowToolTip() override;

private:
    QString text_;
    QModelIndex currentIndex_;
    int tooltipDuration_ = -1;
    ItemViewToolTipType tooltipType_;
    QWidget* viewport_ = nullptr;
};

}  // namespace qfw
