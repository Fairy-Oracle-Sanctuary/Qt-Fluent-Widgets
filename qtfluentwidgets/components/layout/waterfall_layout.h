#pragma once

#include <QEasingCurve>
#include <QLayout>
#include <QPointer>
#include <QVector>

class QParallelAnimationGroup;
class QPropertyAnimation;
class QTimer;

namespace qfw {
// Equal-width, variable-height items go into the currently shortest column.
// takeAt() returns QLayoutItem*, as required by native QLayout ownership rules.
class WaterfallLayout : public QLayout {
    Q_OBJECT
public:
    explicit WaterfallLayout(QWidget* parent = nullptr, bool needAni = false, bool isTight = false);
    ~WaterfallLayout() override;
    void addItem(QLayoutItem* item) override;
    void insertItem(int index, QLayoutItem* item);
    void addWidget(QWidget* widget);
    void insertWidget(int index, QWidget* widget);
    int count() const override;
    QLayoutItem* itemAt(int index) const override;
    QLayoutItem* takeAt(int index) override;
    void removeWidget(QWidget* widget);
    void removeAllWidgets();
    void takeAllWidgets();
    Qt::Orientations expandingDirections() const override;
    bool hasHeightForWidth() const override;
    int heightForWidth(int width) const override;
    QSize sizeHint() const override;
    QSize minimumSize() const override;
    void setGeometry(const QRect& rect) override;
    void invalidate() override;
    void setVerticalSpacing(int spacing);
    int verticalSpacing() const;
    void setHorizontalSpacing(int spacing);
    int horizontalSpacing() const;
    void setColumnCount(int count);
    int columnCount() const;
    void setColumnWidth(int width);
    int columnWidth() const;
    void setAnimation(int duration, QEasingCurve::Type easing = QEasingCurve::Linear);
    bool isAnimationEnabled() const;
    bool isTight() const;

protected:
    bool eventFilter(QObject* object, QEvent* event) override;

private:
    struct Entry {
        QLayoutItem* item = nullptr;
        QPointer<QPropertyAnimation> animation;
    };
    struct Placement {
        int index;
        QRect rect;
    };
    int calculate(const QRect& rect, QVector<Placement>* placements = nullptr) const;
    void applyGeometry();
    void scheduleGeometry();
    QVector<Entry> entries_;
    QParallelAnimationGroup* animations_ = nullptr;
    QTimer* debounce_ = nullptr;
    int verticalSpacing_ = 5;
    int horizontalSpacing_ = 10;
    int columns_ = 3;
    int columnWidth_ = 0;
    int duration_ = 300;
    QEasingCurve::Type easing_ = QEasingCurve::Linear;
    bool animated_ = false;
    bool tight_ = false;
    bool applying_ = false;
};
}  // namespace qfw
