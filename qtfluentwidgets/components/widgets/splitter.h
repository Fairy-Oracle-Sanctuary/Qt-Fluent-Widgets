#pragma once

#include <QSplitter>
#include <QSplitterHandle>

#include "common/qtcompat.h"

namespace qfw {
class SplitterHandle : public QSplitterHandle {
    Q_OBJECT
public:
    explicit SplitterHandle(Qt::Orientation orientation, QSplitter* parent);
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(enterEvent_QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    bool hovered_ = false;
};

// Retains QSplitter's native drag, sizes, stretch factors and state persistence.
class Splitter : public QSplitter {
    Q_OBJECT
public:
    explicit Splitter(Qt::Orientation orientation, QWidget* parent = nullptr);
    explicit Splitter(QWidget* parent = nullptr);

protected:
    QSplitterHandle* createHandle() override;
};
}  // namespace qfw
