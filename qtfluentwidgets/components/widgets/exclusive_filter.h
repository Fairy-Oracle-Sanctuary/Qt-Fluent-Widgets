#pragma once

#include <QHash>
#include <QStringList>
#include <QWidget>

class QButtonGroup;
class QHBoxLayout;
class QPropertyAnimation;
class QScrollArea;

namespace qfw {
class FluentPushButton;
class TransparentToolButton;

class LiteFilterBase : public QWidget {
    Q_OBJECT
public:
    ~LiteFilterBase() override;
    FluentPushButton* addItem(const QString& text);
    void addItems(const QStringList& texts);
    void removeItem(const QString& text);
    void clear();
    QStringList items() const;
    FluentPushButton* button(const QString& text) const;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    QHBoxLayout* hBoxLayout;
    QScrollArea* scrollArea;
    QWidget* container;
    QHBoxLayout* itemsLayout;
    TransparentToolButton* rightArrowBtn;
    QWidget* spacer;

protected:
    LiteFilterBase(bool multiSelection, bool outlined, QWidget* parent);
    QStringList selection() const;
    void applySelection(const QStringList& texts);
    virtual void notifySelectionChanged() = 0;
    bool eventFilter(QObject* watched, QEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    void scrollBy(int distance);
    void updateArrowVisibility();
    void resizeItem(FluentPushButton* button);
    void ensureItemVisible(FluentPushButton* button);
    bool multiSelection_;
    bool outlined_;
    bool updating_ = false;
    bool clearing_ = false;
    QStringList items_;
    QStringList selection_;
    QHash<QString, FluentPushButton*> buttons_;
    QButtonGroup* group_;
    QPropertyAnimation* scrollAnimation_;
};

class ExclusiveLiteFilterBase : public LiteFilterBase {
    Q_OBJECT
public:
    QString currentItem() const;
    void setCurrentItem(const QString& text);
signals:
    void currentTextChanged(const QString& text);

protected:
    ExclusiveLiteFilterBase(bool outlined, QWidget* parent);
    void notifySelectionChanged() override;
};
class ExclusiveLiteFilter : public ExclusiveLiteFilterBase {
    Q_OBJECT
public:
    explicit ExclusiveLiteFilter(QWidget* parent = nullptr);
};
class OutlinedExclusiveLiteFilter : public ExclusiveLiteFilterBase {
    Q_OBJECT
public:
    explicit OutlinedExclusiveLiteFilter(QWidget* parent = nullptr);
};
class MultiSelectionLiteFilterBase : public LiteFilterBase {
    Q_OBJECT
public:
    QStringList currentItems() const;
    void setCurrentItems(const QStringList& texts);
    void clearSelection();
signals:
    void currentItemsChanged(const QStringList& texts);

protected:
    MultiSelectionLiteFilterBase(bool outlined, QWidget* parent);
    void notifySelectionChanged() override;
};
class MultiSelectionLiteFilter : public MultiSelectionLiteFilterBase {
    Q_OBJECT
public:
    explicit MultiSelectionLiteFilter(QWidget* parent = nullptr);
};
class OutlinedMultiSelectionLiteFilter : public MultiSelectionLiteFilterBase {
    Q_OBJECT
public:
    explicit OutlinedMultiSelectionLiteFilter(QWidget* parent = nullptr);
};
}  // namespace qfw
