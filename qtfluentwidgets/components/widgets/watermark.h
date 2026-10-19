#pragma once

#include <QColor>
#include <QPixmap>
#include <QPointer>
#include <QSizeF>
#include <QTimer>
#include <QWidget>

namespace qfw {
class Watermark : public QWidget {
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(qreal angle READ angle WRITE setAngle)
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
public:
    explicit Watermark(const QString& text = QString(), QWidget* target = nullptr);
    QWidget* targetWidget() const;
    void setTargetWidget(QWidget* target);
    QString text() const;
    void setText(const QString& text);
    qreal angle() const;
    void setAngle(qreal angle);
    qreal opacity() const;
    void setOpacity(qreal opacity);
    QSizeF spacing() const;
    void setSpacing(qreal horizontal, qreal vertical);
    void setSpacing(qreal spacing);
    QColor color() const;
    void setColor(const QColor& light, const QColor& dark = QColor());
    QSize sizeHint() const override;

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void changeEvent(QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    void syncTarget();
    void invalidateTile();
    void createTile();
    QPointer<QWidget> target_;
    QList<QPointer<QWidget>> siblings_;
    QString text_;
    qreal angle_ = -15, opacity_ = 0.1, tileDpr_ = 0;
    QSizeF spacing_{60, 30};
    QColor light_{Qt::black}, dark_{Qt::white};
    QPixmap tile_;
    QTimer syncTimer_;
};
}  // namespace qfw
