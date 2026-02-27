#ifndef SEPARATOR_H
#define SEPARATOR_H

#include <QWidget>

namespace qfw {

class HorizontalSeparator : public QWidget {
    Q_OBJECT

public:
    explicit HorizontalSeparator(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
};

class VerticalSeparator : public QWidget {
    Q_OBJECT

public:
    explicit VerticalSeparator(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
};

}  // namespace qfw

#endif  // SEPARATOR_H
