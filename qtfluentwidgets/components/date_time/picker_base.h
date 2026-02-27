#pragma once

#include <QColor>
#include <QGraphicsDropShadowEffect>
#include <QList>
#include <QListWidgetItem>
#include <QObject>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QStringList>
#include <QVariant>
#include <QWidget>

#include "components/widgets/button.h"

class QPainter;
class QHBoxLayout;
class QFrame;
class QVBoxLayout;
class QGraphicsDropShadowEffect;

namespace qfw {

class CycleListWidget;

class SeparatorWidget : public QWidget {
    Q_OBJECT

public:
    explicit SeparatorWidget(Qt::Orientation orient, QWidget* parent = nullptr);
};

class ItemMaskWidget : public QWidget {
    Q_OBJECT

public:
    explicit ItemMaskWidget(QList<CycleListWidget*>* listWidgets, QWidget* parent = nullptr);

    void setCustomBackgroundColor(const QColor& light, const QColor& dark);

protected:
    void paintEvent(QPaintEvent* e) override;

private:
    void drawText(QListWidgetItem* item, QPainter* painter, int y);

    QList<CycleListWidget*>* listWidgets_;
    QColor lightBackgroundColor_;
    QColor darkBackgroundColor_;
};

class PickerColumnFormatter : public QObject {
    Q_OBJECT

public:
    explicit PickerColumnFormatter(QObject* parent = nullptr);
    ~PickerColumnFormatter() override = default;

    virtual QString encode(const QVariant& value) const;
    virtual QVariant decode(const QString& value) const;
};

class DigitFormatter : public PickerColumnFormatter {
    Q_OBJECT

public:
    using PickerColumnFormatter::PickerColumnFormatter;

    QVariant decode(const QString& value) const override;
};

class PickerColumnButton : public QPushButton {
    Q_OBJECT

public:
    explicit PickerColumnButton(const QString& name, const QVariantList& items, int width,
                                Qt::Alignment align = Qt::AlignLeft,
                                PickerColumnFormatter* formatter = nullptr,
                                QWidget* parent = nullptr);

    Qt::Alignment align() const;
    void setAlignment(Qt::Alignment align = Qt::AlignCenter);

    QString value() const;
    void setValue(const QVariant& v);

    QStringList items() const;
    void setItems(const QVariantList& items);

    PickerColumnFormatter* formatter() const;
    void setFormatter(PickerColumnFormatter* formatter);

    QString name() const;
    void setName(const QString& name);

protected:
    QString name_;
    QVariant value_;
    QVariantList items_;
    Qt::Alignment align_ = Qt::AlignLeft;
    PickerColumnFormatter* formatter_ = nullptr;
};

class PickerToolButton : public TransparentToolButton {
    Q_OBJECT

public:
    using TransparentToolButton::TransparentToolButton;

protected:
    void drawIcon(QPainter* painter, const QRectF& rect, QIcon::State state = QIcon::Off) override;
};

class PickerPanel : public QWidget {
    Q_OBJECT

public:
    explicit PickerPanel(QWidget* parent = nullptr);

    void setShadowEffect(int blurRadius = 30, const QPoint& offset = QPoint(0, 8),
                         const QColor& color = QColor(0, 0, 0, 30));

    void setResetEnabled(bool enabled);
    void setScrollButtonRepeatEnabled(bool enabled);
    void setSelectedBackgroundColor(const QColor& light, const QColor& dark);

    bool isResetEnabled() const;

    void addColumn(const QStringList& items, int width, Qt::Alignment align = Qt::AlignCenter);

    QStringList value() const;
    void setValue(const QStringList& value);

    QString columnValue(int index) const;
    void setColumnValue(int index, const QString& value);

    CycleListWidget* column(int index) const;

    void exec(const QPoint& pos, bool ani = true);

signals:
    void confirmed(const QStringList& value);
    void resetted();
    void columnValueChanged(int index, const QString& value);

protected:
    void resizeEvent(QResizeEvent* e) override;

private slots:
    void onAniValueChanged(const QVariant& opacity);
    void fadeOut();

private:
    void initWidget();

    int itemHeight_ = 37;
    QList<CycleListWidget*> listWidgets_;

    QFrame* view_ = nullptr;
    ItemMaskWidget* itemMaskWidget_ = nullptr;
    SeparatorWidget* hSeparatorWidget_ = nullptr;
    PickerToolButton* yesButton_ = nullptr;
    PickerToolButton* resetButton_ = nullptr;
    PickerToolButton* cancelButton_ = nullptr;

    QHBoxLayout* hBoxLayout_ = nullptr;
    QHBoxLayout* listLayout_ = nullptr;
    QHBoxLayout* buttonLayout_ = nullptr;
    QVBoxLayout* vBoxLayout_ = nullptr;

    bool scrollButtonRepeatEnabled_ = true;
    bool isExpanded_ = false;
    QPropertyAnimation* ani_ = nullptr;
    QGraphicsDropShadowEffect* shadowEffect_ = nullptr;
};

class PickerBase : public QPushButton {
    Q_OBJECT

public:
    explicit PickerBase(QWidget* parent = nullptr);

    void setSelectedBackgroundColor(const QColor& light, const QColor& dark);

    void addColumn(const QString& name, const QVariantList& items, int width,
                   Qt::Alignment align = Qt::AlignCenter,
                   PickerColumnFormatter* formatter = nullptr);

    void setColumnAlignment(int index, Qt::Alignment align = Qt::AlignCenter);
    void setColumnWidth(int index, int width);
    void setColumnTight(int index);
    void setColumnVisible(int index, bool isVisible);

    QStringList value() const;

    void setColumnValue(int index, const QVariant& value);
    void setColumnFormatter(int index, PickerColumnFormatter* formatter);
    void setColumnItems(int index, const QVariantList& items);

    QString encodeValue(int index, const QVariant& value) const;
    QVariant decodeValue(int index, const QString& value) const;

    void setColumn(int index, const QString& name, const QVariantList& items, int width,
                   Qt::Alignment align = Qt::AlignCenter);

    void clearColumns();

    virtual QStringList panelInitialValue() const;

    void setScrollButtonRepeatEnabled(bool enabled);

    bool isResetEnabled() const;
    void setResetEnabled(bool enabled);

public slots:
    void reset();

protected:
    void enterEvent(QEnterEvent* e) override;
    void leaveEvent(QEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;

    virtual void showPanel();
    virtual void onColumnValueChanged(int index, const QString& value);
    virtual void onConfirmed(const QStringList& value);

protected:
    bool isValidColumnIndex(int index) const;
    void setButtonProperty(const char* name, bool value);

    QList<PickerColumnButton*> columns_;
    QColor lightSelectedBackgroundColor_;
    QColor darkSelectedBackgroundColor_;

    bool resetEnabled_ = false;
    QHBoxLayout* hBoxLayout_ = nullptr;
    bool scrollButtonRepeatEnabled_ = true;
};

}  // namespace qfw
