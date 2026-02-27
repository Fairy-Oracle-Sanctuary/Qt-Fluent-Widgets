#pragma once

#include <QAbstractAnimation>
#include <QColor>
#include <QEvent>
#include <QFrame>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QMap>
#include <QObject>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWeakPointer>
#include <QWidget>

#include "common/icon.h"

namespace qfw {

class InfoBar;
class InfoBarManager;

// ============================================================================
// InfoBarIcon
// ============================================================================

enum class InfoBarIconEnum { Information, Success, Warning, Error };

class InfoBarIcon : public FluentIconBase {
public:
    explicit InfoBarIcon(InfoBarIconEnum icon);

    QString path(Theme theme = Theme::Auto) const override;
    void render(QPainter* painter, const QRect& rect, Theme theme = Theme::Auto,
                const QVariantMap& attributes = {}) const override;
    FluentIconBase* clone() const override;

private:
    InfoBarIconEnum icon_;
};

// ============================================================================
// InfoBarPosition
// ============================================================================

enum class InfoBarPosition { Top, Bottom, TopLeft, TopRight, BottomLeft, BottomRight, None };

// ============================================================================
// InfoIconWidget
// ============================================================================

class InfoIconWidget : public QWidget {
    Q_OBJECT

public:
    explicit InfoIconWidget(InfoBarIconEnum icon, QWidget* parent = nullptr);

    void setIcon(InfoBarIconEnum icon);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    InfoBarIconEnum icon_;
};

// ============================================================================
// InfoBar
// ============================================================================

class InfoBar : public QFrame {
    Q_OBJECT

public:
    explicit InfoBar(InfoBarIconEnum icon, const QString& title, const QString& content,
                     Qt::Orientation orient = Qt::Horizontal, bool isClosable = true,
                     int duration = 1000, InfoBarPosition position = InfoBarPosition::TopRight,
                     QWidget* parent = nullptr);

    void addWidget(QWidget* widget, int stretch = 0);
    void setCustomBackgroundColor(const QColor& light, const QColor& dark);

    QString title() const { return title_; }
    QString content() const { return content_; }
    InfoBarIconEnum iconType() const { return iconType_; }
    InfoBarPosition position() const { return position_; }
    int duration() const { return duration_; }

    static InfoBar* create(InfoBarIconEnum icon, const QString& title, const QString& content,
                           Qt::Orientation orient = Qt::Horizontal, bool isClosable = true,
                           int duration = 1000,
                           InfoBarPosition position = InfoBarPosition::TopRight,
                           QWidget* parent = nullptr);

    static InfoBar* info(const QString& title, const QString& content,
                         Qt::Orientation orient = Qt::Horizontal, bool isClosable = true,
                         int duration = 1000, InfoBarPosition position = InfoBarPosition::TopRight,
                         QWidget* parent = nullptr);

    static InfoBar* success(const QString& title, const QString& content,
                            Qt::Orientation orient = Qt::Horizontal, bool isClosable = true,
                            int duration = 1000,
                            InfoBarPosition position = InfoBarPosition::TopRight,
                            QWidget* parent = nullptr);

    static InfoBar* warning(const QString& title, const QString& content,
                            Qt::Orientation orient = Qt::Horizontal, bool isClosable = true,
                            int duration = 1000,
                            InfoBarPosition position = InfoBarPosition::TopRight,
                            QWidget* parent = nullptr);

    static InfoBar* error(const QString& title, const QString& content,
                          Qt::Orientation orient = Qt::Horizontal, bool isClosable = true,
                          int duration = 1000, InfoBarPosition position = InfoBarPosition::TopRight,
                          QWidget* parent = nullptr);

signals:
    void closedSignal();

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    void initWidget();
    void initLayout();
    void setQss();
    void fadeOut();
    void adjustText();

    QString title_;
    QString content_;
    Qt::Orientation orient_;
    InfoBarIconEnum iconType_;
    int duration_;
    bool isClosable_;
    InfoBarPosition position_;

    QLabel* titleLabel_ = nullptr;
    QLabel* contentLabel_ = nullptr;
    QToolButton* closeButton_ = nullptr;
    InfoIconWidget* iconWidget_ = nullptr;

    QHBoxLayout* hBoxLayout_ = nullptr;
    QBoxLayout* textLayout_ = nullptr;
    QBoxLayout* widgetLayout_ = nullptr;

    QGraphicsOpacityEffect* opacityEffect_ = nullptr;
    QPropertyAnimation* opacityAni_ = nullptr;

    QColor lightBackgroundColor_;
    QColor darkBackgroundColor_;
};

// ============================================================================
// InfoBarManager
// ============================================================================

class InfoBarManager : public QObject {
    Q_OBJECT

public:
    static InfoBarManager* make(InfoBarPosition position);

    void add(InfoBar* infoBar);
    void remove(InfoBar* infoBar);

protected:
    explicit InfoBarManager(QObject* parent = nullptr);

    virtual QPoint pos(InfoBar* infoBar, const QSize& parentSize = QSize()) const = 0;
    virtual QPoint slideStartPos(InfoBar* infoBar) const = 0;

    bool eventFilter(QObject* obj, QEvent* event) override;

    void createSlideAni(InfoBar* infoBar);
    void updateDropAni(QWidget* parent);
    QPropertyAnimation* createDropAni(InfoBar* infoBar);

    int spacing_ = 16;
    int margin_ = 24;

    QMap<QWidget*, QList<InfoBar*>> infoBars_;
    QMap<QWidget*, QParallelAnimationGroup*> aniGroups_;
    QList<QPropertyAnimation*> slideAnis_;
    QList<QPropertyAnimation*> dropAnis_;

private:
    static QMap<InfoBarPosition, InfoBarManager*>& managers();
};

// ============================================================================
// TopInfoBarManager
// ============================================================================

class TopInfoBarManager : public InfoBarManager {
public:
    static InfoBarManager* create() { return new TopInfoBarManager(); }

protected:
    QPoint pos(InfoBar* infoBar, const QSize& parentSize = QSize()) const override;
    QPoint slideStartPos(InfoBar* infoBar) const override;
};

// ============================================================================
// TopRightInfoBarManager
// ============================================================================

class TopRightInfoBarManager : public InfoBarManager {
public:
    static InfoBarManager* create() { return new TopRightInfoBarManager(); }

protected:
    QPoint pos(InfoBar* infoBar, const QSize& parentSize = QSize()) const override;
    QPoint slideStartPos(InfoBar* infoBar) const override;
};

// ============================================================================
// BottomRightInfoBarManager
// ============================================================================

class BottomRightInfoBarManager : public InfoBarManager {
public:
    static InfoBarManager* create() { return new BottomRightInfoBarManager(); }

protected:
    QPoint pos(InfoBar* infoBar, const QSize& parentSize = QSize()) const override;
    QPoint slideStartPos(InfoBar* infoBar) const override;
};

// ============================================================================
// TopLeftInfoBarManager
// ============================================================================

class TopLeftInfoBarManager : public InfoBarManager {
public:
    static InfoBarManager* create() { return new TopLeftInfoBarManager(); }

protected:
    QPoint pos(InfoBar* infoBar, const QSize& parentSize = QSize()) const override;
    QPoint slideStartPos(InfoBar* infoBar) const override;
};

// ============================================================================
// BottomLeftInfoBarManager
// ============================================================================

class BottomLeftInfoBarManager : public InfoBarManager {
public:
    static InfoBarManager* create() { return new BottomLeftInfoBarManager(); }

protected:
    QPoint pos(InfoBar* infoBar, const QSize& parentSize = QSize()) const override;
    QPoint slideStartPos(InfoBar* infoBar) const override;
};

// ============================================================================
// BottomInfoBarManager
// ============================================================================

class BottomInfoBarManager : public InfoBarManager {
public:
    static InfoBarManager* create() { return new BottomInfoBarManager(); }

protected:
    QPoint pos(InfoBar* infoBar, const QSize& parentSize = QSize()) const override;
    QPoint slideStartPos(InfoBar* infoBar) const override;
};

// ============================================================================
// DesktopInfoBarView
// ============================================================================

class DesktopInfoBarView : public QWidget {
    Q_OBJECT

public:
    explicit DesktopInfoBarView(QWidget* parent = nullptr);

    static DesktopInfoBarView* instance();
};

}  // namespace qfw
