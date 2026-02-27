#pragma once

#include <QList>
#include <QVBoxLayout>
#include <QWidget>

namespace qfw {

class VBoxLayout : public QVBoxLayout {
    Q_OBJECT

public:
    explicit VBoxLayout(QWidget* parent = nullptr);

    void addWidgets(const QList<QWidget*>& widgets, int stretch = 0,
                    Qt::Alignment alignment = Qt::AlignTop);
    void addWidget(QWidget* widget, int stretch = 0,
                   Qt::Alignment alignment = Qt::AlignTop);

    void removeWidget(QWidget* widget);
    void deleteWidget(QWidget* widget);
    void removeAllWidgets();

private:
    QList<QWidget*> widgets_;
};

} // namespace qfw
