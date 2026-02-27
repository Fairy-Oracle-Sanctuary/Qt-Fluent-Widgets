#pragma once

#include <QHBoxLayout>
#include <QIcon>
#include <QImage>
#include <QMargins>
#include <QObject>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QPoint>
#include <QPointF>
#include <QPolygonF>
#include <QString>
#include <QWidget>

#include "common/icon.h"
#include "components/widgets/flyout.h"

namespace qfw {

class TeachingTipManager;
class TeachingTip;

enum class TeachingTipTailPosition {
    Top = 0,
    Bottom = 1,
    Left = 2,
    Right = 3,
    TopLeft = 4,
    TopRight = 5,
    BottomLeft = 6,
    BottomRight = 7,
    LeftTop = 8,
    LeftBottom = 9,
    RightTop = 10,
    RightBottom = 11,
    None = 12
};

enum class ImagePosition { Top = 0, Bottom = 1, Left = 2, Right = 3 };

class TeachingTipView : public FlyoutView {
    Q_OBJECT

public:
    explicit TeachingTipView(const QString& title, const QString& content,
                             QWidget* parent = nullptr);
    explicit TeachingTipView(const QString& title, const QString& content, const QIcon& icon,
                             QWidget* parent = nullptr);
    explicit TeachingTipView(const QString& title, const QString& content,
                             const FluentIconBase& icon, QWidget* parent = nullptr);
    explicit TeachingTipView(const QString& title, const QString& content, const QIcon& icon,
                             const QVariant& image, bool isClosable = true,
                             TeachingTipTailPosition tailPosition = TeachingTipTailPosition::Bottom,
                             QWidget* parent = nullptr);
    explicit TeachingTipView(const QString& title, const QString& content,
                             const FluentIconBase& icon, const QVariant& image,
                             bool isClosable = true,
                             TeachingTipTailPosition tailPosition = TeachingTipTailPosition::Bottom,
                             QWidget* parent = nullptr);

    TeachingTipManager* manager() const;

protected:
    void paintEvent(QPaintEvent* e) override;

private:
    void init(TeachingTipTailPosition tailPosition);
    void adjustImage();
    void addImageToLayout();

    TeachingTipManager* manager_ = nullptr;
    QHBoxLayout* hBoxLayout_ = nullptr;
};

class TeachTipBubble : public QWidget {
    Q_OBJECT

public:
    explicit TeachTipBubble(FlyoutViewBase* view,
                            TeachingTipTailPosition tailPosition = TeachingTipTailPosition::Bottom,
                            QWidget* parent = nullptr);

    void setView(QWidget* view);
    FlyoutViewBase* view() const;

    TeachingTipManager* manager() const;
    QHBoxLayout* hBoxLayout() const;

protected:
    void paintEvent(QPaintEvent* e) override;

private:
    TeachingTipManager* manager_ = nullptr;
    QHBoxLayout* hBoxLayout_ = nullptr;
    FlyoutViewBase* view_ = nullptr;
};

class TeachingTipManager : public QObject {
    Q_OBJECT

public:
    explicit TeachingTipManager(QObject* parent = nullptr);

    virtual void doLayout(TeachTipBubble* tip);
    virtual ImagePosition imagePosition() const;
    virtual QPoint position(TeachingTip* tip) const;
    virtual void draw(TeachTipBubble* tip, QPainter& painter);

    static TeachingTipManager* make(TeachingTipTailPosition position);

protected:
    virtual QPoint calculatePosition(TeachingTip* tip) const;
};

class TopTailTeachingTipManager : public TeachingTipManager {
    Q_OBJECT

public:
    explicit TopTailTeachingTipManager(QObject* parent = nullptr);

    void doLayout(TeachTipBubble* tip) override;
    ImagePosition imagePosition() const override;
    void draw(TeachTipBubble* tip, QPainter& painter) override;

protected:
    QPoint calculatePosition(TeachingTip* tip) const override;
};

class BottomTailTeachingTipManager : public TeachingTipManager {
    Q_OBJECT

public:
    explicit BottomTailTeachingTipManager(QObject* parent = nullptr);

    void doLayout(TeachTipBubble* tip) override;
    void draw(TeachTipBubble* tip, QPainter& painter) override;

protected:
    QPoint calculatePosition(TeachingTip* tip) const override;
};

class LeftTailTeachingTipManager : public TeachingTipManager {
    Q_OBJECT

public:
    explicit LeftTailTeachingTipManager(QObject* parent = nullptr);

    void doLayout(TeachTipBubble* tip) override;
    ImagePosition imagePosition() const override;
    void draw(TeachTipBubble* tip, QPainter& painter) override;

protected:
    QPoint calculatePosition(TeachingTip* tip) const override;
};

class RightTailTeachingTipManager : public TeachingTipManager {
    Q_OBJECT

public:
    explicit RightTailTeachingTipManager(QObject* parent = nullptr);

    void doLayout(TeachTipBubble* tip) override;
    ImagePosition imagePosition() const override;
    void draw(TeachTipBubble* tip, QPainter& painter) override;

protected:
    QPoint calculatePosition(TeachingTip* tip) const override;
};

class TopLeftTailTeachingTipManager : public TopTailTeachingTipManager {
    Q_OBJECT

public:
    explicit TopLeftTailTeachingTipManager(QObject* parent = nullptr);

    void draw(TeachTipBubble* tip, QPainter& painter) override;

protected:
    QPoint calculatePosition(TeachingTip* tip) const override;
};

class TopRightTailTeachingTipManager : public TopTailTeachingTipManager {
    Q_OBJECT

public:
    explicit TopRightTailTeachingTipManager(QObject* parent = nullptr);

    void draw(TeachTipBubble* tip, QPainter& painter) override;

protected:
    QPoint calculatePosition(TeachingTip* tip) const override;
};

class BottomLeftTailTeachingTipManager : public BottomTailTeachingTipManager {
    Q_OBJECT

public:
    explicit BottomLeftTailTeachingTipManager(QObject* parent = nullptr);

    void draw(TeachTipBubble* tip, QPainter& painter) override;

protected:
    QPoint calculatePosition(TeachingTip* tip) const override;
};

class BottomRightTailTeachingTipManager : public BottomTailTeachingTipManager {
    Q_OBJECT

public:
    explicit BottomRightTailTeachingTipManager(QObject* parent = nullptr);

    void draw(TeachTipBubble* tip, QPainter& painter) override;

protected:
    QPoint calculatePosition(TeachingTip* tip) const override;
};

class LeftTopTailTeachingTipManager : public LeftTailTeachingTipManager {
    Q_OBJECT

public:
    explicit LeftTopTailTeachingTipManager(QObject* parent = nullptr);

    ImagePosition imagePosition() const override;
    void draw(TeachTipBubble* tip, QPainter& painter) override;

protected:
    QPoint calculatePosition(TeachingTip* tip) const override;
};

class LeftBottomTailTeachingTipManager : public LeftTailTeachingTipManager {
    Q_OBJECT

public:
    explicit LeftBottomTailTeachingTipManager(QObject* parent = nullptr);

    ImagePosition imagePosition() const override;
    void draw(TeachTipBubble* tip, QPainter& painter) override;

protected:
    QPoint calculatePosition(TeachingTip* tip) const override;
};

class RightTopTailTeachingTipManager : public RightTailTeachingTipManager {
    Q_OBJECT

public:
    explicit RightTopTailTeachingTipManager(QObject* parent = nullptr);

    ImagePosition imagePosition() const override;
    void draw(TeachTipBubble* tip, QPainter& painter) override;

protected:
    QPoint calculatePosition(TeachingTip* tip) const override;
};

class RightBottomTailTeachingTipManager : public RightTailTeachingTipManager {
    Q_OBJECT

public:
    explicit RightBottomTailTeachingTipManager(QObject* parent = nullptr);

    ImagePosition imagePosition() const override;
    void draw(TeachTipBubble* tip, QPainter& painter) override;

protected:
    QPoint calculatePosition(TeachingTip* tip) const override;
};

class TeachingTip : public QWidget {
    Q_OBJECT

public:
    explicit TeachingTip(FlyoutViewBase* view, QWidget* target, int duration = 1000,
                         TeachingTipTailPosition tailPosition = TeachingTipTailPosition::Bottom,
                         QWidget* parent = nullptr, bool isDeleteOnClose = true);

    void addWidget(QWidget* widget, int stretch = 0, Qt::Alignment align = Qt::AlignLeft);
    void setView(FlyoutViewBase* view);
    FlyoutViewBase* view() const;
    QWidget* target() const;

    static TeachingTip* make(FlyoutViewBase* view, QWidget* target, int duration = 1000,
                             TeachingTipTailPosition tailPosition = TeachingTipTailPosition::Bottom,
                             QWidget* parent = nullptr, bool isDeleteOnClose = true);

    static TeachingTip* create(
        QWidget* target, const QString& title, const QString& content,
        const QVariant& icon = QVariant(), const QVariant& image = QVariant(),
        bool isClosable = true, int duration = 1000,
        TeachingTipTailPosition tailPosition = TeachingTipTailPosition::Bottom,
        QWidget* parent = nullptr, bool isDeleteOnClose = true);

protected:
    void showEvent(QShowEvent* e) override;
    void closeEvent(QCloseEvent* e) override;
    bool eventFilter(QObject* obj, QEvent* e) override;

private:
    void setShadowEffect(int blurRadius = 35, const QPoint& offset = QPoint(0, 8));
    void fadeOut();

    QWidget* target_ = nullptr;
    int duration_ = 1000;
    bool isDeleteOnClose_ = true;
    TeachingTipManager* manager_ = nullptr;
    QHBoxLayout* hBoxLayout_ = nullptr;
    QPropertyAnimation* opacityAni_ = nullptr;
    TeachTipBubble* bubble_ = nullptr;
    QGraphicsDropShadowEffect* shadowEffect_ = nullptr;
};

class PopupTeachingTip : public TeachingTip {
    Q_OBJECT

public:
    explicit PopupTeachingTip(
        FlyoutViewBase* view, QWidget* target, int duration = 1000,
        TeachingTipTailPosition tailPosition = TeachingTipTailPosition::Bottom,
        QWidget* parent = nullptr, bool isDeleteOnClose = true);
};

}  // namespace qfw
