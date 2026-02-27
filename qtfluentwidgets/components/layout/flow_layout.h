#pragma once

#include <QEasingCurve>
#include <QEvent>
#include <QLayout>
#include <QList>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QRect>
#include <QTimer>

namespace qfw {

class FlowLayout : public QLayout {
    Q_OBJECT

public:
    explicit FlowLayout(QWidget* parent = nullptr, bool needAni = false, bool isTight = false);
    ~FlowLayout() override;

    void addItem(QLayoutItem* item) override;
    void insertItem(int index, QLayoutItem* item);

    void addWidget(QWidget* w);
    void insertWidget(int index, QWidget* w);

    void setAnimation(int duration, QEasingCurve::Type ease = QEasingCurve::Linear);

    int count() const override;
    QLayoutItem* itemAt(int index) const override;
    QLayoutItem* takeAt(int index) override;

    void removeWidget(QWidget* widget);
    void removeAllWidgets();
    void takeAllWidgets();

    Qt::Orientations expandingDirections() const override;
    bool hasHeightForWidth() const override;
    int heightForWidth(int width) const override;

    void setGeometry(const QRect& rect) override;
    QSize sizeHint() const override;
    QSize minimumSize() const override;

    void setVerticalSpacing(int spacing);
    int verticalSpacing() const;

    void setHorizontalSpacing(int spacing);
    int horizontalSpacing() const;

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void onDebounceTimeout();

private:
    void onWidgetAdded(QWidget* w, int index = -1);
    int doLayout(const QRect& rect, bool move) const;

    QList<QLayoutItem*> items_;
    QList<QPropertyAnimation*> anis_;
    QParallelAnimationGroup* aniGroup_;
    QTimer* debounceTimer_;

    int verticalSpacing_ = 10;
    int horizontalSpacing_ = 10;
    int duration_ = 300;
    QEasingCurve::Type ease_ = QEasingCurve::Linear;
    bool needAni_ = false;
    bool isTight_ = false;
    QWidget* wParent_ = nullptr;
    bool isInstalledEventFilter_ = false;
};

}  // namespace qfw
