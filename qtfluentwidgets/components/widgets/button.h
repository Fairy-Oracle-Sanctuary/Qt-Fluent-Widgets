#pragma once

#include <QColor>
#include <QEnterEvent>
#include <QEvent>
#include <QIcon>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPoint>
#include <QPushButton>
#include <QRadioButton>
#include <QString>
#include <QToolButton>
#include <QUrl>
#include <QVariant>

#include "common/icon.h"

class QMenu;
class QPainter;
class QRectF;
class QHBoxLayout;

namespace qfw {

class FluentPushButton : public QPushButton {
    Q_OBJECT

public:
    explicit FluentPushButton(QWidget* parent = nullptr);
    explicit FluentPushButton(const QString& text, QWidget* parent = nullptr);
    explicit FluentPushButton(const QIcon& icon, const QString& text, QWidget* parent = nullptr);
    explicit FluentPushButton(const FluentIconBase& icon, const QString& text,
                              QWidget* parent = nullptr);
    ~FluentPushButton();

    void setIcon(const QIcon& icon);
    void setIcon(const FluentIconBase& icon);
    void setIcon(const QVariant& icon);

    QIcon icon() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;

    void paintContent(QPainter& painter, QStyleOptionButton& opt);

    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

    virtual void drawIcon(QPainter* painter, const QRectF& rect, QIcon::State state = QIcon::Off);

    QVariant icon_;
    bool isPressed = false;
    bool isHover = false;

private:
    void init();
    FluentIconBase* ownedIcon_ = nullptr;  // owned icon for lifetime management
};

class PrimaryPushButton : public FluentPushButton {
    Q_OBJECT

public:
    explicit PrimaryPushButton(QWidget* parent = nullptr);
    explicit PrimaryPushButton(const QString& text, QWidget* parent = nullptr);
    explicit PrimaryPushButton(const QIcon& icon, const QString& text, QWidget* parent = nullptr);
    explicit PrimaryPushButton(const FluentIconBase& icon, const QString& text,
                               QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
    void drawIcon(QPainter* painter, const QRectF& rect, QIcon::State state = QIcon::Off) override;
};

class TransparentPushButton : public FluentPushButton {
    Q_OBJECT

public:
    explicit TransparentPushButton(QWidget* parent = nullptr);
    explicit TransparentPushButton(const QString& text, QWidget* parent = nullptr);
    explicit TransparentPushButton(const QIcon& icon, const QString& text,
                                   QWidget* parent = nullptr);
    explicit TransparentPushButton(const FluentIconBase& icon, const QString& text,
                                   QWidget* parent = nullptr);
};

class PushButton : public FluentPushButton {
    Q_OBJECT

public:
    explicit PushButton(QWidget* parent = nullptr);
    explicit PushButton(const QString& text, QWidget* parent = nullptr);
    explicit PushButton(const QIcon& icon, const QString& text, QWidget* parent = nullptr);
    explicit PushButton(const FluentIconBase& icon, const QString& text, QWidget* parent = nullptr);
};

class ToggleButton : public FluentPushButton {
    Q_OBJECT

public:
    explicit ToggleButton(QWidget* parent = nullptr);
    explicit ToggleButton(const QString& text, QWidget* parent = nullptr);
    explicit ToggleButton(const QIcon& icon, const QString& text, QWidget* parent = nullptr);
    explicit ToggleButton(const FluentIconBase& icon, const QString& text,
                          QWidget* parent = nullptr);

protected:
    void drawIcon(QPainter* painter, const QRectF& rect, QIcon::State state = QIcon::Off) override;
};

class TransparentTogglePushButton : public ToggleButton {
    Q_OBJECT

public:
    explicit TransparentTogglePushButton(QWidget* parent = nullptr);
    explicit TransparentTogglePushButton(const QString& text, QWidget* parent = nullptr);
    explicit TransparentTogglePushButton(const QIcon& icon, const QString& text,
                                         QWidget* parent = nullptr);
    explicit TransparentTogglePushButton(const FluentIconBase& icon, const QString& text,
                                         QWidget* parent = nullptr);
};

class HyperlinkButton : public FluentPushButton {
    Q_OBJECT
    Q_PROPERTY(QUrl url READ url WRITE setUrl)

public:
    explicit HyperlinkButton(QWidget* parent = nullptr);
    explicit HyperlinkButton(const QString& url, const QString& text, QWidget* parent = nullptr,
                             const QVariant& icon = QVariant());
    explicit HyperlinkButton(const QIcon& icon, const QString& url, const QString& text,
                             QWidget* parent = nullptr);
    explicit HyperlinkButton(const FluentIconBase& icon, const QString& url, const QString& text,
                             QWidget* parent = nullptr);

    QUrl url() const;
    void setUrl(const QUrl& url);
    void setUrl(const QString& url);

protected:
    void drawIcon(QPainter* painter, const QRectF& rect, QIcon::State state = QIcon::Off) override;

private:
    void onClicked();

    QUrl url_;
};

class RadioButton : public QRadioButton {
    Q_OBJECT
    Q_PROPERTY(QColor lightTextColor READ lightTextColor WRITE setLightTextColor)
    Q_PROPERTY(QColor darkTextColor READ darkTextColor WRITE setDarkTextColor)

public:
    explicit RadioButton(QWidget* parent = nullptr);
    explicit RadioButton(const QString& text, QWidget* parent = nullptr);

    QColor lightTextColor() const;
    QColor darkTextColor() const;
    void setLightTextColor(const QColor& color);
    void setDarkTextColor(const QColor& color);

    void setIndicatorColor(const QColor& light, const QColor& dark);
    void setTextColor(const QColor& light, const QColor& dark);

protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    void drawText(QPainter* painter);
    void drawIndicator(QPainter* painter);
    void drawCircle(QPainter* painter, const QPoint& center, int radius, int thickness,
                    const QColor& borderColor, const QColor& filledColor);
    QColor textColor() const;

    QColor lightTextColor_;
    QColor darkTextColor_;
    QColor lightIndicatorColor_;
    QColor darkIndicatorColor_;
    QPoint indicatorPos_;
    bool isHover_;
};

class ToolButton : public QToolButton {
    Q_OBJECT

public:
    explicit ToolButton(QWidget* parent = nullptr);
    explicit ToolButton(const QIcon& icon, QWidget* parent = nullptr);
    explicit ToolButton(const FluentIconBase& icon, QWidget* parent = nullptr);
    ~ToolButton();

    void setIcon(const QIcon& icon);
    void setIcon(const FluentIconBase& icon);
    void setIcon(const QVariant& icon);

    QIcon icon() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    virtual void drawIcon(QPainter* painter, const QRectF& rect, QIcon::State state = QIcon::Off);
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

    QVariant icon_;
    bool isPressed = false;
    bool isHover = false;

private:
    void init();
    FluentIconBase* ownedIcon_ = nullptr;  // owned icon for lifetime management
};

class TransparentToolButton : public ToolButton {
    Q_OBJECT

public:
    explicit TransparentToolButton(QWidget* parent = nullptr);
    explicit TransparentToolButton(const QIcon& icon, QWidget* parent = nullptr);
    explicit TransparentToolButton(const FluentIconBase& icon, QWidget* parent = nullptr);
    explicit TransparentToolButton(FluentIconEnum icon, QWidget* parent = nullptr);
};

class PrimaryToolButton : public ToolButton {
    Q_OBJECT

public:
    explicit PrimaryToolButton(QWidget* parent = nullptr);
    explicit PrimaryToolButton(const QIcon& icon, QWidget* parent = nullptr);
    explicit PrimaryToolButton(const FluentIconBase& icon, QWidget* parent = nullptr);

protected:
    void drawIcon(QPainter* painter, const QRectF& rect, QIcon::State state = QIcon::Off) override;
};

class ToggleToolButton : public ToolButton {
    Q_OBJECT

public:
    explicit ToggleToolButton(QWidget* parent = nullptr);
    explicit ToggleToolButton(const QIcon& icon, QWidget* parent = nullptr);
    explicit ToggleToolButton(const FluentIconBase& icon, QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
    void drawIcon(QPainter* painter, const QRectF& rect, QIcon::State state = QIcon::Off) override;
};

class TransparentToggleToolButton : public ToggleToolButton {
    Q_OBJECT

public:
    explicit TransparentToggleToolButton(QWidget* parent = nullptr);
    explicit TransparentToggleToolButton(const QIcon& icon, QWidget* parent = nullptr);
    explicit TransparentToggleToolButton(const FluentIconBase& icon, QWidget* parent = nullptr);
};

class TranslateYAnimation;

class DropDownPushButton : public FluentPushButton {
    Q_OBJECT

public:
    explicit DropDownPushButton(QWidget* parent = nullptr);
    explicit DropDownPushButton(const QString& text, QWidget* parent = nullptr);
    explicit DropDownPushButton(const QIcon& icon, const QString& text, QWidget* parent = nullptr);
    explicit DropDownPushButton(const FluentIconBase& icon, const QString& text,
                                QWidget* parent = nullptr);

    void setMenu(QMenu* menu);
    QMenu* menu() const;

protected:
    void mouseReleaseEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    void showMenu();
    void drawDropDownIcon(QPainter* painter, const QRectF& rect);

    QMenu* menu_ = nullptr;
    TranslateYAnimation* arrowAni_ = nullptr;
};

class TransparentDropDownPushButton : public DropDownPushButton {
    Q_OBJECT

public:
    using DropDownPushButton::DropDownPushButton;
    explicit TransparentDropDownPushButton(QWidget* parent = nullptr);
    explicit TransparentDropDownPushButton(const QString& text, QWidget* parent = nullptr);
    explicit TransparentDropDownPushButton(const QIcon& icon, const QString& text,
                                           QWidget* parent = nullptr);
    explicit TransparentDropDownPushButton(const FluentIconBase& icon, const QString& text,
                                           QWidget* parent = nullptr);
};

class PrimaryDropDownButtonBase {
protected:
    void drawPrimaryDropDownIcon(QPainter* painter, const QRectF& rect);
};

class PrimaryDropDownPushButton : public PrimaryPushButton, protected PrimaryDropDownButtonBase {
    Q_OBJECT

public:
    explicit PrimaryDropDownPushButton(QWidget* parent = nullptr);
    explicit PrimaryDropDownPushButton(const QString& text, QWidget* parent = nullptr);
    explicit PrimaryDropDownPushButton(const QIcon& icon, const QString& text,
                                       QWidget* parent = nullptr);
    explicit PrimaryDropDownPushButton(const FluentIconBase& icon, const QString& text,
                                       QWidget* parent = nullptr);

    void setMenu(QMenu* menu);
    QMenu* menu() const;

protected:
    void mouseReleaseEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    void showMenu();

    QMenu* menu_ = nullptr;
    TranslateYAnimation* arrowAni_ = nullptr;
};

class SplitDropButton : public ToolButton {
    Q_OBJECT

public:
    explicit SplitDropButton(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    TranslateYAnimation* arrowAni_ = nullptr;
};

class PrimarySplitDropButton : public PrimaryToolButton {
    Q_OBJECT

public:
    explicit PrimarySplitDropButton(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    TranslateYAnimation* arrowAni_ = nullptr;
};

class SplitWidgetBase : public QWidget {
    Q_OBJECT

public:
    explicit SplitWidgetBase(QWidget* parent = nullptr);

    void setWidget(QWidget* widget);
    void setDropButton(ToolButton* button);
    ToolButton* dropButton() const;

    void setFlyout(QMenu* menu);
    QMenu* flyout() const;

signals:
    void dropDownClicked();

protected slots:
    void showFlyout();

protected:
    QMenu* flyout_ = nullptr;
    ToolButton* dropButton_ = nullptr;
    QHBoxLayout* hBoxLayout_ = nullptr;
};

class SplitPushButton : public SplitWidgetBase {
    Q_OBJECT

public:
    explicit SplitPushButton(QWidget* parent = nullptr);
    explicit SplitPushButton(const QString& text, QWidget* parent = nullptr,
                             const QVariant& icon = QVariant());
    explicit SplitPushButton(const QIcon& icon, const QString& text, QWidget* parent = nullptr);
    explicit SplitPushButton(const FluentIconBase& icon, const QString& text,
                             QWidget* parent = nullptr);

    void setText(const QString& text);
    QString text() const;

    void setIcon(const QIcon& icon);
    void setIcon(const FluentIconBase& icon);
    void setIcon(const QVariant& icon);

    FluentPushButton* button() const;

private:
    FluentPushButton* button_ = nullptr;
};

class PrimarySplitPushButton : public SplitWidgetBase {
    Q_OBJECT

public:
    explicit PrimarySplitPushButton(QWidget* parent = nullptr);
    explicit PrimarySplitPushButton(const QString& text, QWidget* parent = nullptr,
                                    const QVariant& icon = QVariant());
    explicit PrimarySplitPushButton(const QIcon& icon, const QString& text,
                                    QWidget* parent = nullptr);
    explicit PrimarySplitPushButton(const FluentIconBase& icon, const QString& text,
                                    QWidget* parent = nullptr);

    void setText(const QString& text);
    QString text() const;

    void setIcon(const QIcon& icon);
    void setIcon(const FluentIconBase& icon);
    void setIcon(const QVariant& icon);

    PrimaryPushButton* button() const;

private:
    PrimaryPushButton* button_ = nullptr;
};

class SplitToolButton : public SplitWidgetBase {
    Q_OBJECT

public:
    explicit SplitToolButton(QWidget* parent = nullptr);
    explicit SplitToolButton(const QVariant& icon, QWidget* parent = nullptr);
    explicit SplitToolButton(const QIcon& icon, QWidget* parent = nullptr);
    explicit SplitToolButton(const FluentIconBase& icon, QWidget* parent = nullptr);

    void setIcon(const QIcon& icon);
    void setIcon(const FluentIconBase& icon);
    void setIcon(const QVariant& icon);
    QIcon icon() const;

    void setIconSize(const QSize& size);
    QSize iconSize() const;

signals:
    void clicked();

protected:
    ToolButton* button_ = nullptr;
};

class PrimarySplitToolButton : public SplitToolButton {
    Q_OBJECT

public:
    explicit PrimarySplitToolButton(QWidget* parent = nullptr);
    explicit PrimarySplitToolButton(const QVariant& icon, QWidget* parent = nullptr);
    explicit PrimarySplitToolButton(const QIcon& icon, QWidget* parent = nullptr);
    explicit PrimarySplitToolButton(const FluentIconBase& icon, QWidget* parent = nullptr);

private:
    void initPrimary();
};

class PillButtonBase {
protected:
    void paintPill(QPainter* painter, const QRect& rect, bool checked, bool enabled, bool pressed,
                   bool hover);
};

class PillPushButton : public ToggleButton, protected PillButtonBase {
    Q_OBJECT

public:
    explicit PillPushButton(QWidget* parent = nullptr);
    explicit PillPushButton(const QString& text, QWidget* parent = nullptr);
    explicit PillPushButton(const QIcon& icon, const QString& text, QWidget* parent = nullptr);
    explicit PillPushButton(const FluentIconBase& icon, const QString& text,
                            QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
};

class PillToolButton : public ToggleToolButton, protected PillButtonBase {
    Q_OBJECT

public:
    explicit PillToolButton(QWidget* parent = nullptr);
    explicit PillToolButton(const QIcon& icon, QWidget* parent = nullptr);
    explicit PillToolButton(const FluentIconBase& icon, QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
};

class DropDownToolButton : public ToolButton {
    Q_OBJECT

public:
    explicit DropDownToolButton(QWidget* parent = nullptr);
    explicit DropDownToolButton(const QIcon& icon, QWidget* parent = nullptr);
    explicit DropDownToolButton(const FluentIconBase& icon, QWidget* parent = nullptr);

    void setMenu(QMenu* menu);
    QMenu* menu() const;

protected:
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    void paintEvent(QPaintEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void drawIcon(QPainter* painter, const QRectF& rect, QIcon::State state = QIcon::Off) override;

private:
    void showMenu();
    void drawDropDownIcon(QPainter* painter, const QRectF& rect);

    QMenu* menu_ = nullptr;
    TranslateYAnimation* arrowAni_ = nullptr;
};

class TransparentDropDownToolButton : public DropDownToolButton {
    Q_OBJECT

public:
    using DropDownToolButton::DropDownToolButton;
    explicit TransparentDropDownToolButton(QWidget* parent = nullptr);
    explicit TransparentDropDownToolButton(const QIcon& icon, QWidget* parent = nullptr);
    explicit TransparentDropDownToolButton(const FluentIconBase& icon, QWidget* parent = nullptr);
};

}  // namespace qfw
