#pragma once

#include <QColor>
#include <QFrame>
#include <QMargins>
#include <QPointer>
#include <QList>
#include <functional>

class QBoxLayout;
class QPropertyAnimation;
class QTimer;

namespace qfw {
class BodyLabel;
class TransparentToolButton;
class ToastStack;
enum class ToastPosition { Top, Bottom, TopLeft, TopRight, BottomLeft, BottomRight };
enum class ToastColor { Success, Error, Warning };
class Toast;
class ToastManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(int margin READ margin WRITE setMargin)
public:
    using Factory = std::function<ToastManager*()>;
    explicit ToastManager(ToastPosition position = ToastPosition::TopRight, QObject* parent = nullptr);
    static ToastManager* get(ToastPosition position);
    // C++ 'register' is a keyword; registerFactory is the equivalent extension hook.
    static void registerFactory(ToastPosition position, Factory factory);
    void add(Toast* toast);
    void remove(Toast* toast);
    int margin() const { return margin_; }
    void setMargin(int margin);
    virtual QPoint slideStartPos(Toast* toast) const;
    virtual QPoint slideEndPos(Toast* toast) const;
protected:
    QList<Toast*> precedingBars(Toast* toast) const;
private:
    void reflow();
    ToastPosition position_;
    int margin_ = 24;
    QList<QPointer<Toast>> bars_;
};

// Parent-owned notification with a top status stripe and six stacking positions.
class Toast : public QFrame {
    Q_OBJECT
public:
    explicit Toast(const QString& title, const QString& content, int duration = 2000,
                   bool isClosable = true, ToastPosition position = ToastPosition::TopRight,
                   Qt::Orientation orient = Qt::Vertical,
                   const QColor& toastColor = QColor("#0F7B0F"), QWidget* parent = nullptr,
                   const QColor& backgroundColor = QColor(), bool useThemeColor = false);
    ~Toast() override;
    static QColor color(ToastColor color);
    static Toast* newToast(const QString& title, const QString& content, int duration,
                           bool isClosable, ToastPosition position, Qt::Orientation orient,
                           const QColor& toastColor, QWidget* parent,
                           const QColor& backgroundColor = QColor(), bool useThemeColor = false);
    static Toast* info(const QString& title, const QString& content, int duration = 2000,
                       bool isClosable = true, ToastPosition position = ToastPosition::TopRight,
                       Qt::Orientation orient = Qt::Vertical, QWidget* parent = nullptr);
    static Toast* success(const QString& title, const QString& content, int duration = 2000,
                          bool isClosable = true, ToastPosition position = ToastPosition::TopRight,
                          Qt::Orientation orient = Qt::Vertical, QWidget* parent = nullptr);
    static Toast* warning(const QString& title, const QString& content, int duration = 2000,
                          bool isClosable = true, ToastPosition position = ToastPosition::TopRight,
                          Qt::Orientation orient = Qt::Vertical, QWidget* parent = nullptr);
    static Toast* error(const QString& title, const QString& content, int duration = -1,
                        bool isClosable = true, ToastPosition position = ToastPosition::TopRight,
                        Qt::Orientation orient = Qt::Vertical, QWidget* parent = nullptr);
    static Toast* custom(const QString& title, const QString& content, const QColor& toastColor,
                         const QColor& backgroundColor, QWidget* parent, int duration = 2000,
                         bool isClosable = true, ToastPosition position = ToastPosition::TopRight,
                         Qt::Orientation orient = Qt::Vertical);
    QString title() const;
    void setTitle(const QString& title);
    QString content() const;
    void setContent(const QString& content);
    int duration() const;
    void setDuration(int duration);
    bool isClosable() const;
    void setClosable(bool closable);
    ToastPosition position() const;
    void setPosition(ToastPosition position);
    Qt::Orientation orientation() const;
    virtual QColor statusColor() const;
    void setToastColor(const QColor& color);
    QColor backgroundColor() const;
    void setBackgroundColor(const QColor& color);
    void setUseThemeColor(bool enabled);
    bool usesThemeColor() const;
    void addWidget(QWidget* widget, int stretch = 0, Qt::Alignment alignment = Qt::AlignLeft);
    void setAnimationDuration(int duration);
    int animationDuration() const;
    QPoint targetPosition() const;
    BodyLabel* titleLabel = nullptr;
    BodyLabel* contentLabel = nullptr;
    TransparentToolButton* closeButton = nullptr;
    QBoxLayout* widgetLayout = nullptr;

signals:
    void closed();

protected:
    // Content-only variants retain the same stack, lifetime and text measuring.
    void configureCompact(QWidget* leadingWidget, const QMargins& margins,
                          const QSize& preferredMinimum, bool reserveCloseGap = true);
    virtual QColor defaultBackgroundColor() const;
    bool event(QEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    friend class ToastStack;
    friend class ToastManager;
    void updateSize();
    void animateTo(const QPoint& target, bool animate);
    QString title_;
    QString content_;
    QColor toastColor_;
    QColor backgroundColor_;
    ToastPosition position_;
    Qt::Orientation orient_;
    QBoxLayout* row_ = nullptr;
    QBoxLayout* textLayout_ = nullptr;
    QPropertyAnimation* positionAnimation_ = nullptr;
    QTimer* closeTimer_ = nullptr;
    QPointer<ToastStack> stack_;
    int duration_;
    int animationDuration_ = 200;
    bool closable_;
    bool useThemeColor_;
    bool closing_ = false;
    QWidget* leadingWidget_ = nullptr;
    QSize compactMinimum_;
    QMargins compactMargins_;
    bool reserveCloseGap_ = true;
};
}  // namespace qfw
