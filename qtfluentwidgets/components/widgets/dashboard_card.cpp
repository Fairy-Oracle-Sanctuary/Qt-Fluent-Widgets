#include "components/widgets/dashboard_card.h"

#include <stdexcept>

#include "common/config.h"
#include "common/font.h"
#include "common/style_sheet.h"

namespace qfw {
DashboardCardWidget::DashboardCardWidget(QWidget* parent) : SimpleCardWidget(parent) {
    setBorderRadius(8);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    vBoxLayout = new QVBoxLayout(this);
    vBoxLayout->setContentsMargins(16, 16, 16, 16);
    vBoxLayout->setSpacing(12);
    headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(12);
    iconWidget = new IconWidget(this);
    iconWidget->setFixedSize(24, 24);
    iconWidget->hide();
    titleLabel = new BodyLabel(this);
    titleLabel->setTextFormat(Qt::PlainText);
    titleLabel->setWordWrap(true);
    qfw::setFont(titleLabel, 14, QFont::DemiBold);
    qfw::setCustomStyleSheet(titleLabel, "QLabel {color:black; font-weight:600;}",
                             "QLabel {color:white; font-weight:600;}");
    switchButton = new SwitchButton(this);
    switchButton->setOnText(QString());
    switchButton->setOffText(QString());
    switchButton->setSpacing(0);
    switchButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    for (auto* label : switchButton->findChildren<QLabel*>(QString(), Qt::FindDirectChildrenOnly)) {
        label->hide();
    }
    headerLayout->addWidget(iconWidget, 0, Qt::AlignTop);
    headerLayout->addWidget(titleLabel, 1);
    headerLayout->addWidget(switchButton, 0, Qt::AlignTop);
    contentWidget = new QWidget(this);
    viewLayout = new QVBoxLayout(contentWidget);
    viewLayout->setContentsMargins(0, 0, 0, 0);
    viewLayout->setSpacing(8);
    contentLabel = new CaptionLabel(contentWidget);
    contentLabel->setTextFormat(Qt::PlainText);
    contentLabel->setWordWrap(true);
    contentLabel->setTextColor(QColor("#606060"), QColor("#cecece"));
    qfw::setFont(contentLabel, 12);
    qfw::setCustomStyleSheet(contentLabel, "QLabel {color:#606060; font-size:12px;}",
                             "QLabel {color:#cecece; font-size:12px;}");
    contentLabel->hide();
    viewLayout->addWidget(contentLabel);
    vBoxLayout->addLayout(headerLayout);
    vBoxLayout->addWidget(contentWidget);
    contentWidget->hide();
    connect(switchButton, &SwitchButton::checkedChanged, this,
            &DashboardCardWidget::checkedChanged);
    // The base theme handler updates the fill, but an in-flight hover animation
    // may still hold the previous theme's color. Stop it after each theme change.
    connect(&QConfig::instance(), &QConfig::themeChanged, this,
            &DashboardCardWidget::refreshBackground);
}
DashboardCardWidget::DashboardCardWidget(const QIcon& icon, const QString& title,
                                         const QString& content, QWidget* parent)
    : DashboardCardWidget(parent) {
    setIcon(icon);
    setTitle(title);
    setContent(content);
}
DashboardCardWidget::DashboardCardWidget(const FluentIconBase& icon, const QString& title,
                                         const QString& content, QWidget* parent)
    : DashboardCardWidget(parent) {
    setIcon(icon);
    setTitle(title);
    setContent(content);
}
QIcon DashboardCardWidget::icon() const { return iconWidget->icon(); }
void DashboardCardWidget::setIcon(const QIcon& icon) {
    iconWidget->setIcon(icon);
    iconWidget->setVisible(!icon.isNull());
}
void DashboardCardWidget::setIcon(const FluentIconBase& icon) {
    iconWidget->setIcon(icon);
    iconWidget->show();
}
void DashboardCardWidget::setIcon(const QString& path) { setIcon(QIcon(path)); }
void DashboardCardWidget::setIconSize(const QSize& size) {
    if (size.width() <= 0 || size.height() <= 0) {
        throw std::invalid_argument("Invalid icon size");
    }
    iconWidget->setFixedSize(size);
}
QString DashboardCardWidget::title() const { return titleLabel->text(); }
void DashboardCardWidget::setTitle(const QString& title) {
    titleLabel->setText(title);
    setAccessibleName(title);
    switchButton->setAccessibleName(title);
    if (auto* indicator = switchButton->findChild<Indicator*>()) {
        indicator->setAccessibleName(title);
    }
}
QString DashboardCardWidget::content() const { return contentLabel->text(); }
void DashboardCardWidget::setContent(const QString& content) {
    contentLabel->setText(content);
    contentLabel->setVisible(!content.isEmpty());
    updateContentVisibility();
}
void DashboardCardWidget::addWidget(QWidget* widget, int stretch, Qt::Alignment alignment) {
    if (!widget || widget == this || widget == contentWidget || widget->isAncestorOf(this)) {
        throw std::invalid_argument("Invalid dashboard content");
    }
    viewLayout->addWidget(widget, stretch, alignment);
    updateContentVisibility();
}
void DashboardCardWidget::addLayout(QLayout* layout, int stretch) {
    if (!layout || layout->parent()) {
        throw std::invalid_argument("Dashboard requires an unowned layout");
    }
    viewLayout->addLayout(layout, stretch);
    updateContentVisibility();
}
void DashboardCardWidget::updateContentVisibility() {
    contentWidget->setVisible(!content().isEmpty() || viewLayout->count() > 1);
}
bool DashboardCardWidget::isChecked() const { return switchButton->isChecked(); }
void DashboardCardWidget::setChecked(bool checked) { switchButton->setChecked(checked); }
void DashboardCardWidget::setSwitchVisible(bool visible) { switchButton->setVisible(visible); }
QColor DashboardCardWidget::normalBackgroundColor() const {
    if (!light_.isValid()) {
        return SimpleCardWidget::normalBackgroundColor();
    }
    return isDarkTheme() ? dark_ : light_;
}
void DashboardCardWidget::refreshBackground() {
    for (auto* animation : findChildren<QPropertyAnimation*>()) {
        if (animation->targetObject() == this && animation->propertyName() == "backgroundColor") {
            animation->stop();
        }
    }
    setBackgroundColor(normalBackgroundColor());
}
void DashboardCardWidget::setCardBackgroundColor(const QColor& light, const QColor& dark) {
    if (!light.isValid()) {
        throw std::invalid_argument("Invalid dashboard color");
    }
    light_ = light;
    dark_ = dark.isValid() ? dark : light;
    refreshBackground();
}
void DashboardCardWidget::resetCardBackgroundColor() {
    light_ = QColor();
    dark_ = QColor();
    refreshBackground();
}
}  // namespace qfw
