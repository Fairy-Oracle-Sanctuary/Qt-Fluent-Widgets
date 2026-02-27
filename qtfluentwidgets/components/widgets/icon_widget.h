#pragma once

#include <QIcon>
#include <QVariant>
#include <QWidget>
#include <memory>

#include "common/icon.h"

namespace qfw {

class IconWidget : public QWidget {
    Q_OBJECT
    Q_PROPERTY(QIcon icon READ icon WRITE setIcon)

public:
    explicit IconWidget(QWidget* parent = nullptr);
    explicit IconWidget(const FluentIconBase& icon, QWidget* parent = nullptr);
    explicit IconWidget(const FluentIconBase* icon, QWidget* parent = nullptr);
    explicit IconWidget(const QIcon& icon, QWidget* parent = nullptr);
    explicit IconWidget(const QString& icon, QWidget* parent = nullptr);

    QIcon icon() const;

public slots:
    void setIcon(const FluentIconBase& icon);
    void setIcon(const FluentIconBase* icon);
    void setIcon(const QIcon& icon);
    void setIcon(const QString& icon);
    void setIcon(const QVariant& icon);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QVariant icon_;
    std::unique_ptr<FluentIconBase> fluentIcon_;
};

}  // namespace qfw
