#pragma once

#include "components/widgets/card_widget.h"
#include "components/widgets/label.h"
#include "components/widgets/switch_button.h"

namespace qfw {
class DashboardCardWidget : public SimpleCardWidget {
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(QString content READ content WRITE setContent)
    Q_PROPERTY(bool checked READ isChecked WRITE setChecked)
public:
    explicit DashboardCardWidget(QWidget* parent = nullptr);
    DashboardCardWidget(const QIcon& icon, const QString& title, const QString& content = {},
                        QWidget* parent = nullptr);
    DashboardCardWidget(const FluentIconBase& icon, const QString& title,
                        const QString& content = {}, QWidget* parent = nullptr);
    QIcon icon() const;
    void setIcon(const QIcon& icon);
    void setIcon(const FluentIconBase& icon);
    void setIcon(const QString& path);
    void setIconSize(const QSize& size);
    QString title() const;
    void setTitle(const QString& title);
    QString content() const;
    void setContent(const QString& content);
    void addWidget(QWidget* widget, int stretch = 0, Qt::Alignment alignment = {});
    void addLayout(QLayout* layout, int stretch = 0);
    bool isChecked() const;
    void setChecked(bool checked);
    void setSwitchVisible(bool visible);
    void setCardBackgroundColor(const QColor& light, const QColor& dark = {});
    void resetCardBackgroundColor();

    QVBoxLayout* vBoxLayout;
    QHBoxLayout* headerLayout;
    IconWidget* iconWidget;
    BodyLabel* titleLabel;
    SwitchButton* switchButton;
    QWidget* contentWidget;
    CaptionLabel* contentLabel;
    QVBoxLayout* viewLayout;
signals:
    void checkedChanged(bool checked);

protected:
    QColor normalBackgroundColor() const override;

private:
    void updateContentVisibility();
    void refreshBackground();
    QColor light_, dark_;
};
}  // namespace qfw
