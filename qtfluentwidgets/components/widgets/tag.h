#pragma once

#include <QIcon>
#include <QWidget>
#include <array>
#include <memory>

#include "common/icon.h"

namespace qfw {

// Decorative status/category label. It is deliberately not a button.
class Tag : public QWidget {
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(QString type READ type WRITE setType)
public:
    explicit Tag(QWidget* parent = nullptr);
    explicit Tag(const QString& text, QWidget* parent = nullptr);
    explicit Tag(const QIcon& icon, const QString& text, QWidget* parent = nullptr);
    explicit Tag(const FluentIconBase& icon, const QString& text, QWidget* parent = nullptr);
    ~Tag() override;
    QString text() const;
    void setText(const QString& text);
    QIcon icon() const;
    void setIcon(const QIcon& icon);
    void setIcon(const FluentIconBase& icon);
    void setIcon(const QVariant& icon);
    QString type() const;
    void setType(const QString& type);
    QColor backgroundColor() const;
    QColor borderColor() const;
    QColor textColor() const;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    std::array<QColor, 3> colors() const;
    QString text_;
    QString type_ = QStringLiteral("information");
    QIcon icon_;
    std::unique_ptr<FluentIconBase> fluentIcon_;
};

}  // namespace qfw
