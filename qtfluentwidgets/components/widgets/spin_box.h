#pragma once

#include <QDoubleSpinBox>
#include <QRectF>
#include <QSpinBox>
#include <QToolButton>
#include <QVBoxLayout>
#include <memory>

#include "common/icon.h"
#include "components/widgets/button.h"
#include "components/widgets/flyout.h"

namespace qfw {

/**
 * @brief Spin icon types
 */
enum class SpinIconType { UP, DOWN };

/**
 * @brief SpinIcon class that implements FluentIconBase
 */
class SpinIcon : public FluentIconBase {
public:
    explicit SpinIcon(SpinIconType type);

    QString path(Theme theme = Theme::Auto) const override;
    FluentIconBase* clone() const override;

    static void render(SpinIconType type, QPainter* painter, const QRectF& rect,
                       Theme theme = Theme::Auto);

private:
    SpinIconType type_;
};

/**
 * @brief Spin button for InlineSpinBox
 */
class SpinButton : public QToolButton {
    Q_OBJECT

public:
    explicit SpinButton(SpinIconType icon, QWidget* parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void paintEvent(QPaintEvent* e) override;

private:
    bool isPressed_ = false;
    SpinIconType iconType_;
};

/**
 * @brief Compact spin button for CompactSpinBox
 */
class CompactSpinButton : public QToolButton {
    Q_OBJECT

public:
    explicit CompactSpinButton(QWidget* parent = nullptr);

signals:
    void clicked();

protected:
    void paintEvent(QPaintEvent* e) override;
};

/**
 * @brief Spin flyout view
 */
class SpinFlyoutView : public FlyoutViewBase {
    Q_OBJECT

public:
    explicit SpinFlyoutView(QWidget* parent = nullptr);

    void addWidget(QWidget* widget, int stretch = 0, Qt::Alignment align = Qt::AlignLeft) override;

    TransparentToolButton* upButton;
    TransparentToolButton* downButton;

protected:
    void paintEvent(QPaintEvent* e) override;

private:
    QVBoxLayout* vBoxLayout_;
    std::unique_ptr<SpinIcon> upIcon_;
    std::unique_ptr<SpinIcon> downIcon_;
};

/**
 * @brief Base class for spin boxes
 */
class SpinBoxBase {
public:
    virtual ~SpinBoxBase() = default;

    virtual bool isError() const = 0;
    virtual void setError(bool isError) = 0;

    virtual void setReadOnly(bool isReadOnly) = 0;
    virtual void setSymbolVisible(bool isVisible) = 0;

    virtual void setCustomFocusedBorderColor(const QColor& light, const QColor& dark) = 0;
    virtual QColor focusedBorderColor() const = 0;

protected:
    virtual void drawBorderBottom(QWidget* widget, QPainter* painter) = 0;
};

/**
 * @brief Inline spin box base
 */
class InlineSpinBoxBase : public SpinBoxBase {
public:
    explicit InlineSpinBoxBase(QWidget* widget);

    void setSymbolVisible(bool isVisible) override;
    void setAccelerated(bool on);

    bool isError() const override;
    void setError(bool isError) override;
    void setReadOnly(bool isReadOnly) override;
    void setCustomFocusedBorderColor(const QColor& light, const QColor& dark) override;
    QColor focusedBorderColor() const override;

    SpinButton* upButton;
    SpinButton* downButton;

protected:
    void drawBorderBottom(QWidget* widget, QPainter* painter) override;
    void updateButtonPosition();
    virtual void stepUp() = 0;
    virtual void stepDown() = 0;

    QWidget* widget_;
    bool isError_ = false;
    QColor lightFocusedBorderColor_;
    QColor darkFocusedBorderColor_;
};

/**
 * @brief Compact spin box base
 */
class CompactSpinBoxBase : public SpinBoxBase {
public:
    explicit CompactSpinBoxBase(QWidget* widget);

    void setSymbolVisible(bool isVisible) override;
    void setAccelerated(bool on);

    bool isError() const override;
    void setError(bool isError) override;
    void setReadOnly(bool isReadOnly) override;
    void setCustomFocusedBorderColor(const QColor& light, const QColor& dark) override;
    QColor focusedBorderColor() const override;

protected:
    void drawBorderBottom(QWidget* widget, QPainter* painter) override;
    void updateButtonPosition();
    void focusInEvent(QWidget* widget, QFocusEvent* e);
    virtual void stepUp() = 0;
    virtual void stepDown() = 0;

    void showFlyout();

private:
    QWidget* widget_;
    bool isError_ = false;
    QColor lightFocusedBorderColor_;
    QColor darkFocusedBorderColor_;

    CompactSpinButton* compactSpinButton;
    SpinFlyoutView* spinFlyoutView;
    Flyout* spinFlyout;
};

/**
 * @brief Spin box
 */
class SpinBox : public QSpinBox, public InlineSpinBoxBase {
    Q_OBJECT

public:
    explicit SpinBox(QWidget* parent = nullptr);

    void setReadOnly(bool isReadOnly) override;
    void setSymbolVisible(bool isVisible) override;

protected:
    void stepUp() override;
    void stepDown() override;
    void resizeEvent(QResizeEvent* e) override;
    void paintEvent(QPaintEvent* e) override;
};

/**
 * @brief Compact spin box
 */
class CompactSpinBox : public QSpinBox, public CompactSpinBoxBase {
    Q_OBJECT

public:
    explicit CompactSpinBox(QWidget* parent = nullptr);

    void setReadOnly(bool isReadOnly) override;
    void setSymbolVisible(bool isVisible) override;

protected:
    void focusInEvent(QFocusEvent* e) override;
    void stepUp() override;
    void stepDown() override;
    void resizeEvent(QResizeEvent* e) override;
    void paintEvent(QPaintEvent* e) override;
};

/**
 * @brief Double spin box
 */
class DoubleSpinBox : public QDoubleSpinBox, public InlineSpinBoxBase {
    Q_OBJECT

public:
    explicit DoubleSpinBox(QWidget* parent = nullptr);

    void setReadOnly(bool isReadOnly) override;
    void setSymbolVisible(bool isVisible) override;

protected:
    void stepUp() override;
    void stepDown() override;
    void resizeEvent(QResizeEvent* e) override;
    void paintEvent(QPaintEvent* e) override;
};

/**
 * @brief Compact double spin box
 */
class CompactDoubleSpinBox : public QDoubleSpinBox, public CompactSpinBoxBase {
    Q_OBJECT

public:
    explicit CompactDoubleSpinBox(QWidget* parent = nullptr);

    void setReadOnly(bool isReadOnly) override;
    void setSymbolVisible(bool isVisible) override;

protected:
    void focusInEvent(QFocusEvent* e) override;
    void stepUp() override;
    void stepDown() override;
    void resizeEvent(QResizeEvent* e) override;
    void paintEvent(QPaintEvent* e) override;
};

}  // namespace qfw
