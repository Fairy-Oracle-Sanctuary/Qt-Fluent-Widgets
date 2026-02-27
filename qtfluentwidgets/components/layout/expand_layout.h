#pragma once

#include <QLayout>
#include <QList>
#include <QRect>
#include <QSize>
#include <QWidget>

namespace qfw {

class ExpandLayout : public QLayout {
    Q_OBJECT

public:
    explicit ExpandLayout(QWidget* parent = nullptr);
    ~ExpandLayout() override;

    void addWidget(QWidget* widget);
    void addItem(QLayoutItem* item) override;

    int count() const override;
    QLayoutItem* itemAt(int index) const override;
    QLayoutItem* takeAt(int index) override;

    Qt::Orientations expandingDirections() const override;
    bool hasHeightForWidth() const override;
    int heightForWidth(int width) const override;

    void setGeometry(const QRect& rect) override;
    QSize sizeHint() const override;
    QSize minimumSize() const override;

protected:
    bool eventFilter(QObject* obj, QEvent* e) override;

private:
    int doLayout(const QRect& rect, bool move) const;

    QList<QLayoutItem*> items_;
    QList<QWidget*> widgets_;
};

} // namespace qfw
