#pragma once

#include <QAbstractButton>
#include <QList>
#include <QTimer>
#include <QVBoxLayout>

#include "common/icon.h"

namespace qfw {
class ToolBoxItem;
// At most one body is visible, including during animated transitions.
class ToolBox : public QWidget {
    Q_OBJECT
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentChanged)
public:
    explicit ToolBox(QWidget* parent = nullptr);
    int count() const;
    int currentIndex() const;
    QWidget* currentWidget() const;
    QWidget* widget(int index) const;
    int indexOf(QWidget* widget) const;
    int addItem(QWidget* widget, const QString& text);
    int addItem(QWidget* widget, const QIcon& icon, const QString& text);
    int addItem(QWidget* widget, const FluentIconBase& icon, const QString& text);
    int insertItem(int index, QWidget* widget, const QString& text);
    void removeItem(int index);
    QString itemText(int index) const;
    void setItemText(int index, const QString& text);
    QIcon itemIcon(int index) const;
    void setItemIcon(int index, const QIcon& icon);
    void setItemIcon(int index, const FluentIconBase& icon);
    QAbstractButton* itemHeader(int index) const;
    bool isItemEnabled(int index) const;
    void setItemEnabled(int index, bool enabled);
    int animationDuration() const;
    void setAnimationDuration(int duration);
public slots:
    void setCurrentIndex(int index);
signals:
    void currentChanged(int index);

protected:
    void hideEvent(QHideEvent* event) override;

private:
    ToolBoxItem* item(int index) const;
    void syncPanels();
    QList<ToolBoxItem*> items_;
    QVBoxLayout* layout_;
    QTimer syncTimer_;
    int current_ = -1;
    int duration_ = 200;
};
}  // namespace qfw
