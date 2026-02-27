#pragma once

#include <QColor>
#include <QFrame>
#include <QHBoxLayout>
#include <QIcon>
#include <QPointer>
#include <QPropertyAnimation>
#include <QVBoxLayout>
#include <QVariant>

#include "components/navigation/navigation_panel.h"
#include "components/widgets/frameless_window.h"
#include "components/window/title_bar.h"

class QLabel;

namespace qfw {

class FluentIconBase;
enum class FluentIconEnum;
class NavigationBar;
class NavigationBarPushButton;
class NavigationInterface;
class NavigationTreeWidget;
class NavigationWidget;
class Router;
class StackedWidget;

class FluentWidgetTitleBar;

class FluentWidget : public FluentMainWindow {
    Q_OBJECT
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor)

public:
    explicit FluentWidget(QWidget* parent = nullptr);

    void setCustomBackgroundColor(const QColor& light, const QColor& dark);

    void setMicaEffectEnabled(bool enabled);
    bool isMicaEffectEnabled() const { return isMicaEnabled_; }

    QColor backgroundColor() const { return backgroundColor_; }
    void setBackgroundColor(const QColor& c);

    QRect systemTitleBarRect(const QSize& size) const;

    void setTitleBar(TitleBarBase* titleBar);
    TitleBarBase* titleBar() const { return titleBar_; }

    void setContentWidget(QWidget* widget);

protected:
    void paintEvent(QPaintEvent* e) override;
    void showEvent(QShowEvent* e) override;

protected slots:
    void onThemeChangedFinished();

protected:
    QColor normalBackgroundColor() const;

private:
    void applyMica();

    bool micaApplied_ = false;
    bool isMicaEnabled_ = false;

    QColor lightBackgroundColor_ = QColor(240, 244, 249);
    QColor darkBackgroundColor_ = QColor(32, 32, 32);

    QColor backgroundColor_;

    QPointer<QPropertyAnimation> bgAni_;

    QPointer<QVBoxLayout> rootLayout_;
    QPointer<TitleBarBase> titleBar_;
    QPointer<QFrame> contentFrame_;
};

class FluentWindowBase : public FluentWidget {
    Q_OBJECT

public:
    explicit FluentWindowBase(QWidget* parent = nullptr);

    virtual NavigationWidget* addSubInterface(QWidget* subInterface, const QVariant& icon,
                                              const QString& text,
                                              NavigationItemPosition position) = 0;

    virtual void removeInterface(QWidget* subInterface, bool isDelete = false) = 0;

    void switchTo(QWidget* subInterface);

protected slots:
    void onCurrentInterfaceChanged(int index);

protected:
    void updateStackedBackground();
    QRect systemTitleBarRect(const QSize& size) const;

protected:
    QPointer<QHBoxLayout> hBoxLayout_;
    QPointer<StackedWidget> stackedWidget_;
    QPointer<NavigationInterface> navigationInterface_;
};

class FluentTitleBar : public TitleBar {
    Q_OBJECT

public:
    explicit FluentTitleBar(QWidget* parent);

    void setTitle(const QString& title);
    void setIcon(const QIcon& icon);

protected:
    void updateWindowTitle();
    void updateWindowIcon();

protected:
    QLabel* iconLabel_ = nullptr;
    QLabel* titleLabel_ = nullptr;
};

class FluentWidgetTitleBar : public FluentTitleBar {
    Q_OBJECT

public:
    explicit FluentWidgetTitleBar(QWidget* parent);
};

class FluentWindow : public FluentWindowBase {
    Q_OBJECT

public:
    explicit FluentWindow(QWidget* parent = nullptr);

    NavigationWidget* addSubInterface(QWidget* subInterface, const QVariant& icon,
                                      const QString& text,
                                      NavigationItemPosition position) override;

    NavigationWidget* addSubInterface(QWidget* subInterface, const QIcon& icon, const QString& text,
                                      NavigationItemPosition position) {
        return addSubInterface(subInterface, QVariant(icon), text, position);
    }

    NavigationWidget* addSubInterface(QWidget* subInterface, FluentIconEnum icon,
                                      const QString& text, NavigationItemPosition position);

    NavigationWidget* addSubInterface(QWidget* subInterface, const FluentIconBase& icon,
                                      const QString& text, NavigationItemPosition position);

    NavigationWidget* addSubInterface(QWidget* subInterface, const QVariant& icon,
                                      const QString& text, NavigationItemPosition position,
                                      QWidget* parent, bool isTransparent);

    NavigationWidget* addSubInterface(QWidget* subInterface, const QVariant& icon,
                                      const QString& text);

    NavigationWidget* addSubInterface(QWidget* subInterface, const QIcon& icon,
                                      const QString& text) {
        return addSubInterface(subInterface, QVariant(icon), text);
    }

    NavigationWidget* addSubInterface(QWidget* subInterface, FluentIconEnum icon,
                                      const QString& text);

    NavigationWidget* addSubInterface(QWidget* subInterface, const FluentIconBase& icon,
                                      const QString& text);

    void removeInterface(QWidget* subInterface, bool isDelete = false) override;

protected:
    void resizeEvent(QResizeEvent* e) override;
    QPointer<QHBoxLayout> widgetLayout_;

private:
    Router* router_ = nullptr;
};

class MSFluentTitleBar : public FluentTitleBar {
    Q_OBJECT

public:
    explicit MSFluentTitleBar(QWidget* parent);
};

class MSFluentWindow : public FluentWindowBase {
    Q_OBJECT

public:
    explicit MSFluentWindow(QWidget* parent = nullptr);

    NavigationWidget* addSubInterface(QWidget* subInterface, const QVariant& icon,
                                      const QString& text,
                                      NavigationItemPosition position) override;

    NavigationWidget* addSubInterface(QWidget* subInterface, const QIcon& icon, const QString& text,
                                      NavigationItemPosition position) {
        return addSubInterface(subInterface, QVariant(icon), text, position);
    }

    NavigationWidget* addSubInterface(QWidget* subInterface, FluentIconEnum icon,
                                      const QString& text, NavigationItemPosition position);

    NavigationWidget* addSubInterface(QWidget* subInterface, const FluentIconBase& icon,
                                      const QString& text, NavigationItemPosition position);

    NavigationWidget* addSubInterface(QWidget* subInterface, const QVariant& icon,
                                      const QString& text, const QVariant& selectedIcon,
                                      NavigationItemPosition position, bool isTransparent);

    void removeInterface(QWidget* subInterface, bool isDelete = false) override;

protected:
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onCurrentInterfaceChangedMs(int index);

private:
    QPointer<NavigationBar> navigationBar_;
};

class SplitTitleBar : public TitleBar {
    Q_OBJECT

public:
    explicit SplitTitleBar(QWidget* parent);

    void setTitle(const QString& title);
    void setIcon(const QIcon& icon);

private:
    QLabel* iconLabel_ = nullptr;
    QLabel* titleLabel_ = nullptr;
};

class SplitFluentWindow : public FluentWindow {
    Q_OBJECT

public:
    explicit SplitFluentWindow(QWidget* parent = nullptr);
};

}  // namespace qfw
