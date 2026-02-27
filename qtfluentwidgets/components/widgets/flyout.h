#pragma once

#include <QColor>
#include <QEvent>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QIcon>
#include <QImage>
#include <QLabel>
#include <QMargins>
#include <QObject>
#include <QParallelAnimationGroup>
#include <QPixmap>
#include <QPoint>
#include <QPropertyAnimation>
#include <QSize>
#include <QString>
#include <QVBoxLayout>
#include <QVariant>
#include <QWidget>

#include "common/icon.h"

namespace qfw {

class ImageLabel;
class TransparentToolButton;

enum class FlyoutAnimationType {
    PullUp = 0,
    DropDown = 1,
    SlideLeft = 2,
    SlideRight = 3,
    FadeIn = 4,
    None = 5
};

class FlyoutViewBase : public QWidget {
    Q_OBJECT

public:
    explicit FlyoutViewBase(QWidget* parent = nullptr);

    virtual void addWidget(QWidget* widget, int stretch = 0,
                           Qt::Alignment align = Qt::AlignLeft) = 0;

protected:
    QColor backgroundColor() const;
    QColor borderColor() const;

    void paintEvent(QPaintEvent* e) override;
};

class FlyoutView : public FlyoutViewBase {
    Q_OBJECT

public:
    explicit FlyoutView(const QString& title, const QString& content, QWidget* parent = nullptr);
    explicit FlyoutView(const QString& title, const QString& content, const QIcon& icon,
                        QWidget* parent = nullptr);
    explicit FlyoutView(const QString& title, const QString& content, const FluentIconBase& icon,
                        QWidget* parent = nullptr);
    explicit FlyoutView(const QString& title, const QString& content, const QString& image,
                        QWidget* parent = nullptr);

    void setIcon(const QIcon& icon);
    void setImage(const QString& imagePath);
    void setImage(const QImage& image);
    void setImage(const QPixmap& pixmap);

    void setClosable(bool closable);
    bool isClosable() const;

    void addWidget(QWidget* widget, int stretch = 0, Qt::Alignment align = Qt::AlignLeft) override;

    QVBoxLayout* widgetLayout() const;

signals:
    void closed();

protected:
    void showEvent(QShowEvent* e) override;

private:
    void initWidgets();
    void initLayout();
    void adjustText();
    void adjustImage();
    void addImageToLayout();

    QString title_;
    QString content_;

    QIcon icon_;

    QPointer<QVBoxLayout> vBoxLayout_;
    QPointer<QHBoxLayout> viewLayout_;
    QPointer<QVBoxLayout> widgetLayout_;

    QPointer<QLabel> titleLabel_;
    QPointer<QLabel> contentLabel_;
    QWidget* iconWidget_ = nullptr;
    QPointer<ImageLabel> imageLabel_;
    QPointer<TransparentToolButton> closeButton_;

    bool closable_ = false;
};

class FlyoutAnimationManager;

class Flyout : public QWidget {
    Q_OBJECT

public:
    explicit Flyout(FlyoutViewBase* view, QWidget* parent = nullptr, bool deleteOnClose = true);

    void exec(const QPoint& pos, FlyoutAnimationType aniType = FlyoutAnimationType::PullUp);

    static Flyout* make(FlyoutViewBase* view, const QVariant& target = QVariant(),
                        QWidget* parent = nullptr,
                        FlyoutAnimationType aniType = FlyoutAnimationType::PullUp,
                        bool deleteOnClose = true);

    static Flyout* create(const QString& title, const QString& content, const QVariant& icon = {},
                          const QVariant& image = {}, bool closable = false,
                          const QVariant& target = QVariant(), QWidget* parent = nullptr,
                          FlyoutAnimationType aniType = FlyoutAnimationType::PullUp,
                          bool deleteOnClose = true);

    void fadeOut();

signals:
    void closed();

protected:
    void closeEvent(QCloseEvent* e) override;
    void showEvent(QShowEvent* e) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void setShadowEffect(int blurRadius = 35, const QPoint& offset = QPoint(0, 8));

    QPointer<FlyoutViewBase> view_;
    QPointer<QHBoxLayout> hBoxLayout_;

    FlyoutAnimationManager* aniManager_ = nullptr;
    bool deleteOnClose_ = true;

    QPointer<QPropertyAnimation> fadeOutAni_;
};

class FlyoutAnimationManager : public QObject {
    Q_OBJECT

public:
    explicit FlyoutAnimationManager(Flyout* flyout);
    ~FlyoutAnimationManager() override = default;

    virtual void exec(const QPoint& pos) = 0;
    virtual QPoint position(QWidget* target) const = 0;

    static FlyoutAnimationManager* make(FlyoutAnimationType type, Flyout* flyout);

protected:
    QPoint adjustPosition(const QPoint& pos) const;

    QPointer<Flyout> flyout_;

    QParallelAnimationGroup* group_ = nullptr;
    QPropertyAnimation* slideAni_ = nullptr;
    QPropertyAnimation* opacityAni_ = nullptr;
};

class PullUpFlyoutAnimationManager : public FlyoutAnimationManager {
    Q_OBJECT

public:
    using FlyoutAnimationManager::FlyoutAnimationManager;

    QPoint position(QWidget* target) const override;
    void exec(const QPoint& pos) override;
};

class DropDownFlyoutAnimationManager : public FlyoutAnimationManager {
    Q_OBJECT

public:
    using FlyoutAnimationManager::FlyoutAnimationManager;

    QPoint position(QWidget* target) const override;
    void exec(const QPoint& pos) override;
};

class SlideLeftFlyoutAnimationManager : public FlyoutAnimationManager {
    Q_OBJECT

public:
    using FlyoutAnimationManager::FlyoutAnimationManager;

    QPoint position(QWidget* target) const override;
    void exec(const QPoint& pos) override;
};

class SlideRightFlyoutAnimationManager : public FlyoutAnimationManager {
    Q_OBJECT

public:
    using FlyoutAnimationManager::FlyoutAnimationManager;

    QPoint position(QWidget* target) const override;
    void exec(const QPoint& pos) override;
};

class FadeInFlyoutAnimationManager : public FlyoutAnimationManager {
    Q_OBJECT

public:
    explicit FadeInFlyoutAnimationManager(Flyout* flyout);

    QPoint position(QWidget* target) const override;
    void exec(const QPoint& pos) override;
};

class DummyFlyoutAnimationManager : public FlyoutAnimationManager {
    Q_OBJECT

public:
    using FlyoutAnimationManager::FlyoutAnimationManager;

    QPoint position(QWidget* target) const override;
    void exec(const QPoint& pos) override;
};

}  // namespace qfw
