#pragma once

#include <QColor>
#include <QPointer>
#include <QTimer>
#include <QVBoxLayout>
#include <QVariantAnimation>
#include <QWidget>

namespace qfw {
class SubtitleLabel;
class TransparentToolButton;
enum class DrawerPosition { LEFT, RIGHT, TOP, BOTTOM };

// Parent-scoped overlay. Content belongs to the drawer; no native window or WebView is created.
class Drawer : public QWidget {
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle)
public:
    explicit Drawer(const QString& title, QWidget* parent,
                    DrawerPosition position = DrawerPosition::RIGHT);
    explicit Drawer(QWidget* parent, DrawerPosition position = DrawerPosition::RIGHT);
    QString title() const;
    void setTitle(const QString& title);
    void addWidget(QWidget* widget, int stretch = 0, Qt::Alignment alignment = {});
    DrawerPosition position() const;
    void setPosition(DrawerPosition position);
    int drawerSize() const;
    void setDrawerSize(int size);
    int animationDuration() const;
    void setAnimationDuration(int duration);
    QColor maskColor() const;
    void setMaskColor(const QColor& color);
    bool isClosableOnMaskClicked() const;
    void setClosableOnMaskClicked(bool enabled);
    bool isEscClosable() const;
    void setEscClosable(bool enabled);
    bool isOpen() const;
    QWidget* panelWidget() const;
    QWidget* contentWidget;
    QVBoxLayout* viewLayout;
public slots:
    void open();
    void open(DrawerPosition position);
    void show();
    bool close();
    void hide();
signals:
    void opened();
    void closed();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
    bool focusNextPrevChild(bool next) override;

private:
    void updatePanelGeometry();
    void animateTo(qreal value);
    void finishClosed();
    QWidget* panel_;
    SubtitleLabel* titleLabel_;
    TransparentToolButton* closeButton_;
    QPointer<QWidget> previousFocus_;
    QVariantAnimation animation_;
    QTimer raiseTimer_;
    DrawerPosition position_;
    QColor mask_{0, 0, 0, 45};
    int size_ = 320;
    int duration_ = 240;
    qreal progress_ = 0;
    bool open_ = false;
    bool maskClosable_ = true;
    bool escClosable_ = true;
    bool finishing_ = false;
};
}  // namespace qfw
