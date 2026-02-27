#pragma once

#include <QCheckBox>
#include <QColor>
#include <QIcon>

#include "common/icon.h"

namespace qfw {

class CheckBoxIcon : public FluentIconBase {
public:
    enum Icon { Accept, PartialAccept };

    explicit CheckBoxIcon(Icon icon) : icon_(icon) {}

    QString path(Theme theme = Theme::Auto) const override {
        const QString c = getIconColor(theme, true);
        const QString name =
            (icon_ == Accept) ? QStringLiteral("Accept") : QStringLiteral("PartialAccept");
        return QStringLiteral(":/qfluentwidgets/images/check_box/%1_%2.svg").arg(name, c);
    }

    FluentIconBase* clone() const override { return new CheckBoxIcon(*this); }

private:
    Icon icon_;
};

class CheckBox : public QCheckBox {
    Q_OBJECT

public:
    explicit CheckBox(QWidget* parent = nullptr);
    explicit CheckBox(const QString& text, QWidget* parent = nullptr);

    void setCheckedColor(const QColor& light, const QColor& dark);
    void setTextColor(const QColor& light, const QColor& dark);

protected:
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void enterEvent(QEnterEvent* e) override;
    void leaveEvent(QEvent* e) override;
    void paintEvent(QPaintEvent* e) override;

private:
    enum class State {
        Normal,
        Hover,
        Pressed,
        Checked,
        CheckedHover,
        CheckedPressed,
        Disabled,
        CheckedDisabled
    };

    State state() const;
    QColor borderColor() const;
    QColor backgroundColor() const;

    static CheckBoxIcon stateIcon(Qt::CheckState state);

    bool isPressed_ = false;
    bool isHover_ = false;

    QColor lightCheckedColor_;
    QColor darkCheckedColor_;
    QColor lightTextColor_ = QColor(0, 0, 0);
    QColor darkTextColor_ = QColor(255, 255, 255);
};

}  // namespace qfw
