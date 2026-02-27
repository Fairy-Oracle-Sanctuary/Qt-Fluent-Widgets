#pragma once

#include <QListWidget>
#include <QStyledItemDelegate>
#include <Qt>

#include "common/icon.h"

namespace qfw {

class SmoothScrollBar;

enum class PipsScrollButtonDisplayMode { Always = 0, OnHover = 1, Never = 2 };

class PipsScrollButton : public QWidget {
    Q_OBJECT

public:
    explicit PipsScrollButton(FluentIconEnum iconEnum, QWidget* parent = nullptr);

    void setHover(bool hover);
    void setPressed(bool pressed);
    bool isHover() const;
    bool isPressed() const;

signals:
    void clicked();

protected:
    void paintEvent(QPaintEvent* e) override;
    void enterEvent(QEnterEvent* e) override;
    void leaveEvent(QEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;

private:
    FluentIconEnum iconEnum_;
    bool isHover_ = false;
    bool isPressed_ = false;
};

class PipsDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit PipsDelegate(QObject* parent = nullptr);

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

    void setHoveredRow(int row);
    void setPressedRow(int row);

    int hoveredRow() const;
    int pressedRow() const;

private:
    int hoveredRow_ = -1;
    int pressedRow_ = -1;
};

class PipsPager : public QListWidget {
    Q_OBJECT
    Q_PROPERTY(int visibleNumber READ visibleNumber WRITE setVisibleNumber)
    Q_PROPERTY(int pageNumber READ pageNumber WRITE setPageNumber)

public:
    explicit PipsPager(QWidget* parent = nullptr);
    explicit PipsPager(Qt::Orientation orientation, QWidget* parent = nullptr);

    void setPageNumber(int n);
    int pageNumber() const;

    void setVisibleNumber(int n);
    int visibleNumber() const;

    void setCurrentIndex(int index);
    int currentIndex() const;

    void setPreviousButtonDisplayMode(PipsScrollButtonDisplayMode mode);
    void setNextButtonDisplayMode(PipsScrollButtonDisplayMode mode);

    PipsScrollButtonDisplayMode previousButtonDisplayMode() const;
    PipsScrollButtonDisplayMode nextButtonDisplayMode() const;

    void scrollNext();
    void scrollPrevious();

    bool isHorizontal() const;
    Qt::Orientation orientation() const;

signals:
    void currentIndexChanged(int index);

protected:
    void mouseReleaseEvent(QMouseEvent* e) override;
    void enterEvent(QEnterEvent* e) override;
    void leaveEvent(QEvent* e) override;
    void wheelEvent(QWheelEvent* e) override;
    void resizeEvent(QResizeEvent* e) override;

private:
    void init(Qt::Orientation orientation);
    void adjustSize();
    void scrollToItem(QListWidgetItem* item);
    void updateScrollButtonVisibility();
    bool isPreviousButtonVisible() const;
    bool isNextButtonVisible() const;
    void setPressedItem(QListWidgetItem* item);
    void setHoveredItem(QListWidgetItem* item);

    Qt::Orientation orientation_ = Qt::Horizontal;
    int visibleNumber_ = 5;
    bool isHover_ = false;

    PipsDelegate* delegate_ = nullptr;
    SmoothScrollBar* scrollBar_ = nullptr;
    PipsScrollButton* preButton_ = nullptr;
    PipsScrollButton* nextButton_ = nullptr;

    PipsScrollButtonDisplayMode previousButtonDisplayMode_ = PipsScrollButtonDisplayMode::Never;
    PipsScrollButtonDisplayMode nextButtonDisplayMode_ = PipsScrollButtonDisplayMode::Never;
};

class HorizontalPipsPager : public PipsPager {
    Q_OBJECT

public:
    explicit HorizontalPipsPager(QWidget* parent = nullptr);
};

class VerticalPipsPager : public PipsPager {
    Q_OBJECT

public:
    explicit VerticalPipsPager(QWidget* parent = nullptr);
};

}  // namespace qfw
