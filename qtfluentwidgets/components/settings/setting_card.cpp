#include "components/settings/setting_card.h"

#include <QFont>
#include <QHBoxLayout>
#include <QPainter>
#include <QVBoxLayout>

#include "common/font.h"
#include "common/style_sheet.h"
#include "components/dialog_box/color_dialog.h"
#include "components/widgets/button.h"
#include "components/widgets/combo_box.h"
#include "components/widgets/slider.h"
#include "components/widgets/switch_button.h"

namespace qfw {

// ==========================================================================
// SettingIconWidget
// ==========================================================================
SettingIconWidget::SettingIconWidget(QWidget* parent) : IconWidget(parent) {}

SettingIconWidget::SettingIconWidget(const QVariant& icon, QWidget* parent) : IconWidget(parent) {
    setIcon(icon);
}

void SettingIconWidget::setIcon(const QVariant& icon) {
    // Handle FluentIconBase* wrapped in QVariant (used by ExpandSettingCard subclasses)
    if (icon.canConvert<const FluentIconBase*>()) {
        const auto* fluentIcon = icon.value<const FluentIconBase*>();
        if (fluentIcon) {
            IconWidget::setIcon(*fluentIcon);
            return;
        }
    }
    // Fallback to base class handling for QIcon, QString, etc.
    IconWidget::setIcon(icon);
}

void SettingIconWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);

    if (!isEnabled()) {
        painter.setOpacity(0.36);
    }

    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    const QIcon qicon = icon();
    if (!qicon.isNull()) {
        qicon.paint(&painter, rect(), Qt::AlignCenter, QIcon::Normal, QIcon::Off);
        return;
    }

    // fallback to IconWidget painting for FluentIconBase
    IconWidget::paintEvent(event);
}

// ==========================================================================
// SettingCard
// ==========================================================================
SettingCard::SettingCard(const QVariant& icon, const QString& title, const QString& content,
                         QWidget* parent)
    : QFrame(parent),
      iconLabel_(new SettingIconWidget(icon, this)),
      titleLabel_(new QLabel(title, this)),
      contentLabel_(new QLabel(content, this)),
      hBoxLayout_(new QHBoxLayout(this)),
      vBoxLayout_(new QVBoxLayout()) {
    setProperty("qssClass", "SettingCard");

    if (content.isEmpty()) {
        contentLabel_->hide();
    }

    setFixedHeight(content.isEmpty() ? 50 : 70);
    iconLabel_->setFixedSize(16, 16);

    hBoxLayout_->setSpacing(0);
    hBoxLayout_->setContentsMargins(16, 0, 0, 0);
    hBoxLayout_->setAlignment(Qt::AlignVCenter);

    vBoxLayout_->setSpacing(0);
    vBoxLayout_->setContentsMargins(0, 0, 0, 0);
    vBoxLayout_->setAlignment(Qt::AlignVCenter);

    hBoxLayout_->addWidget(iconLabel_, 0, Qt::AlignLeft);
    hBoxLayout_->addSpacing(16);

    hBoxLayout_->addLayout(vBoxLayout_);
    vBoxLayout_->addWidget(titleLabel_, 0, Qt::AlignLeft);
    vBoxLayout_->addWidget(contentLabel_, 0, Qt::AlignLeft);

    hBoxLayout_->addSpacing(16);
    hBoxLayout_->addStretch(1);

    titleLabel_->setObjectName(QStringLiteral("titleLabel"));
    contentLabel_->setObjectName(QStringLiteral("contentLabel"));

    qfw::setStyleSheet(this, FluentStyleSheet::SettingCard);
}

void SettingCard::setTitle(const QString& title) { titleLabel_->setText(title); }

void SettingCard::setContent(const QString& content) {
    contentLabel_->setText(content);
    contentLabel_->setVisible(!content.isEmpty());
    setFixedHeight(content.isEmpty() ? 50 : 70);
}

void SettingCard::setValue(const QVariant& value) { Q_UNUSED(value); }

void SettingCard::setIconSize(int width, int height) { iconLabel_->setFixedSize(width, height); }

void SettingCard::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);

    if (isDarkTheme()) {
        painter.setBrush(QColor(255, 255, 255, 13));
        painter.setPen(QColor(0, 0, 0, 50));
    } else {
        painter.setBrush(QColor(255, 255, 255, 170));
        painter.setPen(QColor(0, 0, 0, 19));
    }

    painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 6, 6);
}

// ==========================================================================
// SwitchSettingCard
// ==========================================================================
SwitchSettingCard::SwitchSettingCard(const QVariant& icon, const QString& title,
                                     const QString& content, ConfigItem* configItem,
                                     QWidget* parent)
    : SettingCard(icon, title, content, parent),
      configItem_(configItem),
      switchButton_(new SwitchButton(tr("Off"), this, IndicatorPosition::RIGHT)) {
    setProperty("qssClass", "SwitchSettingCard");

    if (configItem_) {
        setValue(QConfig::instance().get(*configItem_));
        QObject::connect(configItem_, &ConfigItem::valueChanged, this,
                         [this](const QVariant& v) { setValue(v); });
    }

    hBoxLayout_->addWidget(switchButton_, 0, Qt::AlignRight);
    hBoxLayout_->addSpacing(16);

    QObject::connect(switchButton_, &SwitchButton::checkedChanged, this,
                     &SwitchSettingCard::onCheckedChanged);
}

void SwitchSettingCard::onCheckedChanged(bool isChecked) {
    setValue(isChecked);
    emit checkedChanged(isChecked);
}

void SwitchSettingCard::setValue(const QVariant& value) {
    const bool isChecked = value.toBool();

    if (configItem_) {
        QConfig::instance().set(*configItem_, isChecked);
    }

    switchButton_->setChecked(isChecked);
    switchButton_->setText(isChecked ? tr("On") : tr("Off"));
}

void SwitchSettingCard::setChecked(bool isChecked) { setValue(isChecked); }

bool SwitchSettingCard::isChecked() const { return switchButton_->isChecked(); }

// ==========================================================================
// RangeSettingCard
// ==========================================================================
RangeSettingCard::RangeSettingCard(RangeConfigItem* configItem, const QVariant& icon,
                                   const QString& title, const QString& content, QWidget* parent)
    : SettingCard(icon, title, content, parent),
      configItem_(configItem),
      slider_(new Slider(Qt::Horizontal, this)),
      valueLabel_(new QLabel(this)) {
    setProperty("qssClass", "RangeSettingCard");

    slider_->setMinimumWidth(268);
    slider_->setSingleStep(1);

    if (configItem_ && configItem_->rangeValidator()) {
        const auto rv = configItem_->rangeValidator();
        slider_->setRange(static_cast<int>(rv->minimum()), static_cast<int>(rv->maximum()));
        slider_->setValue(QConfig::instance().get(*configItem_).toInt());
        valueLabel_->setNum(slider_->value());

        QObject::connect(configItem_, &ConfigItem::valueChanged, this,
                         [this](const QVariant& v) { setValue(v); });
    }

    hBoxLayout_->addStretch(1);
    hBoxLayout_->addWidget(valueLabel_, 0, Qt::AlignRight);
    hBoxLayout_->addSpacing(6);
    hBoxLayout_->addWidget(slider_, 0, Qt::AlignRight);
    hBoxLayout_->addSpacing(16);

    valueLabel_->setObjectName(QStringLiteral("valueLabel"));

    QObject::connect(slider_, &QSlider::valueChanged, this,
                     &RangeSettingCard::onSliderValueChanged);
}

void RangeSettingCard::onSliderValueChanged(int value) {
    setValue(value);
    emit valueChanged(value);
}

void RangeSettingCard::setValue(const QVariant& value) {
    if (!configItem_) {
        return;
    }

    const int v = value.toInt();
    QConfig::instance().set(*configItem_, v);

    valueLabel_->setNum(v);
    valueLabel_->adjustSize();
    slider_->setValue(v);
}

// ==========================================================================
// PushSettingCard
// ==========================================================================
PushSettingCard::PushSettingCard(const QString& text, const QVariant& icon, const QString& title,
                                 const QString& content, QWidget* parent)
    : SettingCard(icon, title, content, parent), button_(new QPushButton(text, this)) {
    setProperty("qssClass", "PushSettingCard");

    button_->setFont(QFont("Microsoft YaHei", 10, QFont::Normal));
    hBoxLayout_->addWidget(button_, 0, Qt::AlignRight);
    hBoxLayout_->addSpacing(16);

    QObject::connect(button_, &QPushButton::clicked, this, &PushSettingCard::clicked);
}

// ==========================================================================
// PrimaryPushSettingCard
// ==========================================================================
PrimaryPushSettingCard::PrimaryPushSettingCard(const QString& text, const QVariant& icon,
                                               const QString& title, const QString& content,
                                               QWidget* parent)
    : PushSettingCard(text, icon, title, content, parent) {
    button_->setObjectName(QStringLiteral("primaryButton"));
    setProperty("qssClass", "PrimaryPushSettingCard");
}

// ==========================================================================
// HyperlinkCard
// ==========================================================================
HyperlinkCard::HyperlinkCard(const QString& url, const QString& text, const QVariant& icon,
                             const QString& title, const QString& content, QWidget* parent)
    : SettingCard(icon, title, content, parent), linkButton_(new HyperlinkButton(url, text, this)) {
    setProperty("qssClass", "HyperlinkCard");

    hBoxLayout_->addWidget(linkButton_, 0, Qt::AlignRight);
    hBoxLayout_->addSpacing(16);
}

// ==========================================================================
// ColorPickerButton
// ==========================================================================
ColorPickerButton::ColorPickerButton(const QColor& color, const QString& title, QWidget* parent,
                                     bool enableAlpha)
    : QToolButton(parent), color_(color), title_(title), enableAlpha_(enableAlpha) {
    setProperty("qssClass", "ColorPickerButton");

    setFixedSize(96, 32);
    setAttribute(Qt::WA_TranslucentBackground);
    setCursor(Qt::PointingHandCursor);

    QObject::connect(this, &QToolButton::clicked, this, &ColorPickerButton::showColorDialog);
}

void ColorPickerButton::showColorDialog() {
    auto* dlg = new ColorDialog(color_, tr("Choose ") + title_, window(), enableAlpha_);
    dlg->setAttribute(Qt::WA_DeleteOnClose);

    QObject::connect(dlg, &ColorDialog::colorChanged, this, &ColorPickerButton::onColorChanged);
    dlg->exec();
}

void ColorPickerButton::onColorChanged(const QColor& color) {
    setColor(color);
    emit colorChanged(color);
}

void ColorPickerButton::setColor(const QColor& color) {
    color_ = QColor(color);
    update();
}

void ColorPickerButton::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);

    const QColor pc = isDarkTheme() ? QColor(255, 255, 255, 10) : QColor(234, 234, 234);
    painter.setPen(pc);

    QColor drawColor = color_;
    if (!enableAlpha_) {
        drawColor.setAlpha(255);
    }

    painter.setBrush(drawColor);
    painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 5, 5);
}

// ==========================================================================
// ColorSettingCard
// ==========================================================================
ColorSettingCard::ColorSettingCard(ConfigItem* configItem, const QVariant& icon,
                                   const QString& title, const QString& content, QWidget* parent,
                                   bool enableAlpha)
    : SettingCard(icon, title, content, parent),
      configItem_(configItem),
      colorPicker_(new ColorPickerButton(QColor(), title, this, enableAlpha)) {
    setProperty("qssClass", "ColorSettingCard");

    if (configItem_) {
        const QColor c = QConfig::instance().get(*configItem_).value<QColor>();
        colorPicker_->setColor(c);

        QObject::connect(configItem_, &ConfigItem::valueChanged, this,
                         [this](const QVariant& v) { setValue(v); });
    }

    hBoxLayout_->addWidget(colorPicker_, 0, Qt::AlignRight);
    hBoxLayout_->addSpacing(16);

    QObject::connect(colorPicker_, &ColorPickerButton::colorChanged, this,
                     &ColorSettingCard::onColorPicked);
}

void ColorSettingCard::onColorPicked(const QColor& color) {
    if (configItem_) {
        QConfig::instance().set(*configItem_, color);
    }

    emit colorChanged(color);
}

void ColorSettingCard::setValue(const QVariant& value) {
    const QColor c = value.value<QColor>();
    colorPicker_->setColor(c);

    if (configItem_) {
        QConfig::instance().set(*configItem_, c);
    }
}

// ==========================================================================
// ComboBoxSettingCard
// ==========================================================================
ComboBoxSettingCard::ComboBoxSettingCard(OptionsConfigItem* configItem, const QVariant& icon,
                                         const QString& title, const QString& content,
                                         const QStringList& texts, QWidget* parent)
    : SettingCard(icon, title, content, parent),
      configItem_(configItem),
      comboBox_(new ComboBox(this)),
      texts_(texts) {
    setProperty("qssClass", "ComboBoxSettingCard");

    hBoxLayout_->addWidget(comboBox_, 0, Qt::AlignRight);
    hBoxLayout_->addSpacing(16);

    if (configItem_ && configItem_->optionsValidator()) {
        options_ = configItem_->optionsValidator()->options();
        const int n = qMin(options_.size(), texts_.size());

        for (int i = 0; i < n; ++i) {
            comboBox_->addItem(texts_[i], QVariant(), options_[i]);
        }

        setValue(QConfig::instance().get(*configItem_));

        QObject::connect(comboBox_, &ComboBox::currentIndexChanged, this,
                         &ComboBoxSettingCard::onCurrentIndexChanged);
        QObject::connect(configItem_, &ConfigItem::valueChanged, this,
                         [this](const QVariant& v) { setValue(v); });
    }
}

void ComboBoxSettingCard::onCurrentIndexChanged(int index) {
    if (!configItem_) {
        return;
    }

    QConfig::instance().set(*configItem_, comboBox_->itemData(index));
}

void ComboBoxSettingCard::setValue(const QVariant& value) {
    if (!configItem_) {
        return;
    }

    const int n = qMin(options_.size(), texts_.size());
    int idx = -1;
    for (int i = 0; i < n; ++i) {
        if (options_[i] == value) {
            idx = i;
            break;
        }
    }

    if (idx < 0) {
        return;
    }

    comboBox_->setCurrentText(texts_[idx]);
    QConfig::instance().set(*configItem_, value);
}

}  // namespace qfw
