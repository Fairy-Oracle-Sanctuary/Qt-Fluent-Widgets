#pragma once

#include <QAbstractButton>
#include <memory>

#include "components/widgets/extended_button.h"

class QVariantAnimation;

namespace qfw {

// Selectable token with an independent close action. Emitting closed() does
// not hide/delete the token; its owner controls removal.
class Chip : public QAbstractButton {
    Q_OBJECT
public:
    explicit Chip(QWidget* parent = nullptr);
    explicit Chip(const QString& text, QWidget* parent = nullptr);
    explicit Chip(const QIcon& icon, const QString& text, QWidget* parent = nullptr);
    explicit Chip(const FluentIconBase& icon, const QString& text, QWidget* parent = nullptr);
    ~Chip() override;
    void setIcon(const QIcon& icon);
    void setIcon(const FluentIconBase& icon);
    void setIcon(const QVariant& icon);
    QIcon icon() const;
    RoundToolButton* closeButton() const;
    QColor backgroundColor() const;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

signals:
    void closed();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void enterEvent(enterEvent_QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void changeEvent(QEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    void refreshBackground(bool animate = true);
    void updateCloseIcon();
    void positionCloseButton();
    bool hovered_ = false;
    QColor background_;
    RoundToolButton* closeButton_ = nullptr;
    QVariantAnimation* backgroundAnimation_ = nullptr;
    std::unique_ptr<FluentIconBase> fluentIcon_;
};

}  // namespace qfw
