#pragma once
#include <QColor>
#include <QWidget>
#include <optional>

namespace qfw {
class RatingWidget : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(int decimals READ decimals WRITE setDecimals)
public:
    explicit RatingWidget(qreal value = 0, QWidget* parent = nullptr);
    explicit RatingWidget(QWidget* parent);
    qreal value() const;
    virtual void setValue(qreal value);
    int decimals() const;
    void setDecimals(int decimals);
    QString text() const;
    void setStarColor(const QColor& light, const QColor& dark = {});
    QColor starColor() const;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
signals:
    void valueChanged(qreal value);

protected:
    void paintEvent(QPaintEvent*) override;
    void changeEvent(QEvent* event) override;
    static qreal validateValue(qreal value);

private:
    qreal value_;
    int decimals_ = 1;
    QColor light_{QStringLiteral("#f88a00")}, dark_{QStringLiteral("#f88a00")};
};

class InteractiveRatingWidget : public RatingWidget {
    Q_OBJECT
    Q_PROPERTY(bool readOnly READ isReadOnly WRITE setReadOnly)
public:
    explicit InteractiveRatingWidget(qreal value = 0, QWidget* parent = nullptr);
    explicit InteractiveRatingWidget(QWidget* parent);
    void setValue(qreal value) override;
    qreal displayValue() const;
    bool isReadOnly() const;
    void setReadOnly(bool readOnly);
    QSize sizeHint() const override;
signals:
    void hovered(qreal value);

protected:
    void paintEvent(QPaintEvent*) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    qreal valueAt(qreal x) const;
    qreal fillPosition(qreal value) const;
    void preview(qreal x);
    void clearPreview();
    std::optional<qreal> hover_;
    bool readOnly_ = false;
    bool pressed_ = false;
};
}  // namespace qfw
