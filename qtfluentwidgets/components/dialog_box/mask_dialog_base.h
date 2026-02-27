#pragma once

#include <QDialog>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPointer>
#include <QPropertyAnimation>
#include <QResizeEvent>

namespace qfw {

class MaskDialogBase : public QDialog {
    Q_OBJECT

public:
    explicit MaskDialogBase(QWidget* parent = nullptr);
    ~MaskDialogBase() override;

    void setShadowEffect(int blurRadius = 60, const QPoint& offset = QPoint(0, 10),
                         const QColor& color = QColor(0, 0, 0, 100));
    void setMaskColor(const QColor& color);

    bool isClosableOnMaskClicked() const;
    void setClosableOnMaskClicked(bool isClosable);

    void setDraggable(bool draggable);
    bool isDraggable() const;

protected:
    void showEvent(QShowEvent* e) override;
    void resizeEvent(QResizeEvent* e) override;
    bool eventFilter(QObject* obj, QEvent* e) override;

    void done(int code) override;

private slots:
    void onDone(int code);

protected:
    QWidget* windowMask;
    QFrame* widget;
    QHBoxLayout* hBoxLayout;

private:
    bool isClosableOnMaskClicked_ = false;
    bool isDraggable_ = false;
    QPoint dragPos_;
};

}  // namespace qfw
