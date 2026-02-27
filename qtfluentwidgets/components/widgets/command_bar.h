#pragma once

#include <QAction>
#include <QFrame>
#include <QList>
#include <QPointer>
#include <QSize>
#include <QToolButton>
#include <QWidget>

#include "components/widgets/button.h"
#include "components/widgets/flyout.h"
#include "components/widgets/menu.h"
#include "components/widgets/tool_tip.h"

namespace qfw {

class CommandButton : public TransparentToggleToolButton {
    Q_OBJECT

public:
    explicit CommandButton(QWidget* parent = nullptr);

    void setTight(bool tight);
    bool isTight() const;

    QSize sizeHint() const override;

    bool isIconOnly() const;

    QString text() const;
    void setText(const QString& text);

    void setAction(QAction* action);
    QAction* action() const;

protected:
    void paintEvent(QPaintEvent* e) override;
    void drawIcon(QPainter* painter, const QRectF& rect, QIcon::State state = QIcon::Off) override;

private slots:
    void onActionChanged();

private:
    QString text_;
    QPointer<QAction> action_;
    bool tight_ = false;
};

class CommandToolTipFilter : public ToolTipFilter {
    Q_OBJECT

public:
    using ToolTipFilter::ToolTipFilter;

protected:
    bool canShowToolTip() override;
};

class MoreActionsButton : public CommandButton {
    Q_OBJECT

public:
    explicit MoreActionsButton(QWidget* parent = nullptr);

    QSize sizeHint() const override;

    void clearState();
};

class CommandSeparator : public QWidget {
    Q_OBJECT

public:
    explicit CommandSeparator(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* e) override;
};

class CommandMenu : public RoundMenu {
    Q_OBJECT

public:
    explicit CommandMenu(QWidget* parent = nullptr);
};

class CommandBar : public QFrame {
    Q_OBJECT

public:
    explicit CommandBar(QWidget* parent = nullptr);

    void setSpacing(int spacing);
    int spacing() const;

    CommandButton* addAction(QAction* action);
    void addActions(const QList<QAction*>& actions);

    void addHiddenAction(QAction* action);
    void addHiddenActions(const QList<QAction*>& actions);

    CommandButton* insertAction(QAction* before, QAction* action);

    void addSeparator();
    void insertSeparator(int index);

    void addWidget(QWidget* widget);

    void removeAction(QAction* action);
    void removeWidget(QWidget* widget);
    void removeHiddenAction(QAction* action);

    void setToolButtonStyle(Qt::ToolButtonStyle style);
    Qt::ToolButtonStyle toolButtonStyle() const;

    void setButtonTight(bool tight);
    bool isButtonTight() const;

    void setIconSize(const QSize& size);
    QSize iconSize() const;

    void resizeToSuitableWidth();
    int suitableWidth() const;

    void setMenuDropDown(bool down);
    bool isMenuDropDown() const;

    QList<CommandButton*> commandButtons() const;

protected:
    QSize minimumSizeHint() const override;
    void resizeEvent(QResizeEvent* e) override;

    void updateGeometry();

protected slots:
    virtual void showMoreActionsMenu();

private:
    CommandButton* createButton(QAction* action);
    void insertWidgetToLayout(int index, QWidget* widget);

    QList<QWidget*> visibleWidgets() const;

    QList<QWidget*> widgets_;

protected:
    QList<QWidget*> hiddenWidgets_;
    QList<QPointer<QAction>> hiddenActions_;

    MenuAnimationType menuAnimation_ = MenuAnimationType::DropDown;
    Qt::ToolButtonStyle toolButtonStyle_ = Qt::ToolButtonIconOnly;
    QSize iconSize_ = QSize(16, 16);
    bool buttonTight_ = false;
    int spacing_ = 4;

    MoreActionsButton* moreButton_ = nullptr;
};

class CommandViewMenu : public CommandMenu {
    Q_OBJECT

public:
    explicit CommandViewMenu(QWidget* parent = nullptr);

    void setDropDown(bool down, bool isLong = false);
};

class CommandViewBar : public CommandBar {
    Q_OBJECT

public:
    explicit CommandViewBar(QWidget* parent = nullptr);

    void setMenuDropDown(bool down);
    bool isMenuDropDown() const;

protected:
    void showMoreActionsMenu() override;
};

class CommandBarView : public FlyoutViewBase {
    Q_OBJECT

public:
    explicit CommandBarView(QWidget* parent = nullptr);

    void setMenuVisible(bool visible);

    void addWidget(QWidget* widget, int stretch = 0, Qt::Alignment align = Qt::AlignLeft) override;

    void setSpacing(int spacing);
    int spacing() const;

    CommandButton* addAction(QAction* action);
    void addActions(const QList<QAction*>& actions);

    void addHiddenAction(QAction* action);
    void addHiddenActions(const QList<QAction*>& actions);

    CommandButton* insertAction(QAction* before, QAction* action);

    void addSeparator();
    void insertSeparator(int index);

    void removeAction(QAction* action);
    void removeWidget(QWidget* widget);
    void removeHiddenAction(QAction* action);

    void setToolButtonStyle(Qt::ToolButtonStyle style);
    Qt::ToolButtonStyle toolButtonStyle() const;

    void setButtonTight(bool tight);
    bool isButtonTight() const;

    void setIconSize(const QSize& size);
    QSize iconSize() const;

    int suitableWidth() const;
    void resizeToSuitableWidth();

    QList<QAction*> actions() const;

protected:
    void paintEvent(QPaintEvent* e) override;

private:
    CommandViewBar* bar_ = nullptr;
    QHBoxLayout* hBoxLayout_ = nullptr;
    bool menuVisible_ = false;
};

}  // namespace qfw
