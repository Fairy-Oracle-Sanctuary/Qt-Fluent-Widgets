#include "color_dialog.h"

#include <QApplication>
#include <QBrush>
#include <QFont>
#include <QFontMetrics>
#include <QIntValidator>
#include <QPainter>
#include <QPen>
#include <QRegularExpressionValidator>
#include <QShowEvent>

#include "../../common/config.h"
#include "../../common/style_sheet.h"

namespace qfw {

HuePanel::HuePanel(const QColor& color, QWidget* parent)
    : QWidget(parent),
      huePixmap_(QStringLiteral(":/qfluentwidgets/images/color_dialog/HuePanel.png")) {
    setFixedSize(256, 256);
    setColor(color);
}

void HuePanel::mousePressEvent(QMouseEvent* e) { setPickerPosition(e->pos()); }

void HuePanel::mouseMoveEvent(QMouseEvent* e) { setPickerPosition(e->pos()); }

void HuePanel::setPickerPosition(const QPoint& pos) {
    pickerPos_ = pos;

    const double x = qBound(0.0, static_cast<double>(pos.x()) / width(), 1.0);
    const double y = qBound(0.0, static_cast<double>(height() - pos.y()) / height(), 1.0);

    color_.setHsv(static_cast<int>(x * 359), static_cast<int>(y * 255), 255);
    update();
    emit colorChanged(color_);
}

void HuePanel::setColor(const QColor& color) {
    color_ = QColor(color);
    color_.setHsv(color_.hue(), color_.saturation(), 255);

    pickerPos_ = QPoint(static_cast<int>(static_cast<double>(hue()) / 359 * width()),
                        static_cast<int>(static_cast<double>(255 - saturation()) / 255 * height()));
    update();
}

int HuePanel::hue() const { return color_.hue(); }

int HuePanel::saturation() const { return color_.saturation(); }

void HuePanel::paintEvent(QPaintEvent* e) {
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    painter.setBrush(QBrush(huePixmap_));
    painter.setPen(QPen(QColor(0, 0, 0, 15), 2.4));
    painter.drawRoundedRect(rect(), 5.6, 5.6);

    QColor c;
    if (saturation() > 153 || (40 < hue() && hue() < 180)) {
        c = Qt::black;
    } else {
        c = QColor(255, 253, 254);
    }

    painter.setPen(QPen(c, 3));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(pickerPos_.x() - 8, pickerPos_.y() - 8, 16, 16);
}

BrightnessSlider::BrightnessSlider(const QColor& color, QWidget* parent)
    : ClickableSlider(Qt::Horizontal, parent) {
    setRange(0, 255);
    setSingleStep(1);
    setColor(color);

    connect(this, &QSlider::valueChanged, this, &BrightnessSlider::onValueChanged);
}

void BrightnessSlider::setColor(const QColor& color) {
    color_ = QColor(color);
    setValue(color_.value());

    QString qss = qfw::getStyleSheet(FluentStyleSheet::ColorDialog);
    qss.replace(QStringLiteral("--slider-hue"), QString::number(color_.hue()));
    qss.replace(QStringLiteral("--slider-saturation"), QString::number(color_.saturation()));
    setStyleSheet(qss);
}

void BrightnessSlider::onValueChanged(int value) {
    color_.setHsv(color_.hue(), color_.saturation(), value, color_.alpha());
    setColor(color_);
    emit colorChanged(color_);
}

ColorCard::ColorCard(const QColor& color, QWidget* parent, bool enableAlpha)
    : QWidget(parent), enableAlpha_(enableAlpha) {
    setFixedSize(44, 128);
    setColor(color);
    tiledPixmap_ = createTiledBackground();
}

QPixmap ColorCard::createTiledBackground() {
    QPixmap pixmap(8, 8);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    const int c = isDarkTheme() ? 255 : 0;
    QColor color(c, c, c, 26);
    painter.fillRect(4, 0, 4, 4, color);
    painter.fillRect(0, 4, 4, 4, color);
    return pixmap;
}

void ColorCard::setColor(const QColor& color) {
    color_ = QColor(color);
    update();
}

void ColorCard::paintEvent(QPaintEvent* e) {
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);

    if (enableAlpha_) {
        painter.setBrush(QBrush(tiledPixmap_));
        painter.setPen(QColor(0, 0, 0, 13));
        painter.drawRoundedRect(rect(), 4, 4);
    }

    painter.setBrush(color_);
    painter.setPen(QColor(0, 0, 0, 13));
    painter.drawRoundedRect(rect(), 4, 4);
}

ColorLineEdit::ColorLineEdit(int value, QWidget* parent) : LineEdit(parent) {
    setText(QString::number(value));
    setFixedSize(136, 33);
    setClearButtonEnabled(true);
    setValidator(new QIntValidator(0, 255, this));

    connect(this, &QLineEdit::textEdited, this, &ColorLineEdit::onTextEdited);
}

void ColorLineEdit::onTextEdited(const QString& text) {
    int pos = 0;
    const auto* v = validator();
    if (!v) {
        return;
    }

    QString copy = text;
    const auto state = v->validate(copy, pos);
    if (state == QValidator::Acceptable) {
        emit valueChanged(text);
    }
}

HexColorLineEdit::HexColorLineEdit(const QColor& color, QWidget* parent, bool enableAlpha)
    : ColorLineEdit(0, parent) {
    colorFormat_ = enableAlpha ? QColor::HexArgb : QColor::HexRgb;

    setText(QColor(color).name(colorFormat_).mid(1));
    setFixedSize(136, 33);

    if (enableAlpha) {
        setValidator(new QRegularExpressionValidator(
            QRegularExpression(QStringLiteral("[A-Fa-f0-9]{8}")), this));
    } else {
        setValidator(new QRegularExpressionValidator(
            QRegularExpression(QStringLiteral("[A-Fa-f0-9]{6}")), this));
    }

    setTextMargins(4, 0, 33, 0);
    prefixLabel_ = new QLabel(QStringLiteral("#"), this);
    prefixLabel_->move(7, 2);
    prefixLabel_->setObjectName(QStringLiteral("prefixLabel"));
}

void HexColorLineEdit::setColor(const QColor& color) { setText(color.name(colorFormat_).mid(1)); }

ColorDialog::ColorDialog(const QColor& color, const QString& title, QWidget* parent,
                         bool enableAlpha)
    : MaskDialogBase(parent), enableAlpha_(enableAlpha) {
    QColor initColor = QColor(color);
    if (!enableAlpha_) {
        initColor.setAlpha(255);
    }

    oldColor_ = initColor;
    color_ = initColor;

    scrollArea_ = new SingleDirectionScrollArea(widget);
    scrollWidget_ = new QWidget(scrollArea_);

    buttonGroup_ = new QFrame(widget);
    yesButton = new PrimaryPushButton(tr("OK"), buttonGroup_);
    cancelButton = new QPushButton(tr("Cancel"), buttonGroup_);

    titleLabel_ = new QLabel(title, scrollWidget_);
    huePanel_ = new HuePanel(initColor, scrollWidget_);
    newColorCard_ = new ColorCard(initColor, scrollWidget_, enableAlpha_);
    oldColorCard_ = new ColorCard(initColor, scrollWidget_, enableAlpha_);
    brightSlider_ = new BrightnessSlider(initColor, scrollWidget_);

    editLabel_ = new QLabel(tr("Edit Color"), scrollWidget_);
    redLabel_ = new QLabel(tr("Red"), scrollWidget_);
    blueLabel_ = new QLabel(tr("Blue"), scrollWidget_);
    greenLabel_ = new QLabel(tr("Green"), scrollWidget_);

    hexLineEdit_ = new HexColorLineEdit(initColor, scrollWidget_, enableAlpha_);
    redLineEdit_ = new ColorLineEdit(color_.red(), scrollWidget_);
    greenLineEdit_ = new ColorLineEdit(color_.green(), scrollWidget_);
    blueLineEdit_ = new ColorLineEdit(color_.blue(), scrollWidget_);

    vBoxLayout_ = new QVBoxLayout(widget);

    initWidget();
}

void ColorDialog::initWidget() {
    scrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea_->setWidget(scrollWidget_);

    widget->setMaximumSize(488, 726 + 40 * (enableAlpha_ ? 1 : 0));
    widget->resize(488, 726 + 40 * (enableAlpha_ ? 1 : 0));

    // We can't call QAbstractScrollArea::setViewportMargins (protected in Qt C++).
    // Instead we offset all child positions by (48, 24) during layout.
    scrollWidget_->resize(488, 590 + 40 * (enableAlpha_ ? 1 : 0));

    buttonGroup_->setFixedSize(486, 81);
    yesButton->setFixedWidth(216);
    cancelButton->setFixedWidth(216);

    setShadowEffect(60, QPoint(0, 10), QColor(0, 0, 0, 80));
    setMaskColor(QColor(0, 0, 0, 76));

    setQss();
    initLayout();
    connectSignalToSlot();
}

void ColorDialog::initLayout() {
    const int ox = 48;
    const int oy = 24;

    titleLabel_->move(ox, oy);

    huePanel_->move(ox + 0, oy + 46);
    newColorCard_->move(ox + 288, oy + 46);
    oldColorCard_->move(ox + 288, newColorCard_->geometry().bottom() + 1);
    brightSlider_->move(ox + 0, oy + 324);

    editLabel_->move(ox + 0, oy + 385);
    redLineEdit_->move(ox + 0, oy + 426);
    greenLineEdit_->move(ox + 0, oy + 470);
    blueLineEdit_->move(ox + 0, oy + 515);

    redLabel_->move(ox + 144, oy + 434);
    greenLabel_->move(ox + 144, oy + 478);
    blueLabel_->move(ox + 144, oy + 524);

    hexLineEdit_->move(ox + 196, oy + 381);

    vBoxLayout_->setSpacing(0);
    vBoxLayout_->setAlignment(Qt::AlignTop);
    vBoxLayout_->setContentsMargins(0, 0, 0, 0);
    vBoxLayout_->addWidget(scrollArea_, 1);
    vBoxLayout_->addWidget(buttonGroup_, 0, Qt::AlignBottom);

    yesButton->move(24, 25);
    cancelButton->move(250, 25);
}

void ColorDialog::setQss() {
    editLabel_->setObjectName(QStringLiteral("editLabel"));
    titleLabel_->setObjectName(QStringLiteral("titleLabel"));
    yesButton->setObjectName(QStringLiteral("yesButton"));
    cancelButton->setObjectName(QStringLiteral("cancelButton"));
    buttonGroup_->setObjectName(QStringLiteral("buttonGroup"));

    qfw::setStyleSheet(this, FluentStyleSheet::ColorDialog);

    titleLabel_->adjustSize();
    editLabel_->adjustSize();
}

void ColorDialog::setColor(const QColor& color, bool movePicker) {
    color_ = QColor(color);
    brightSlider_->setColor(color_);
    newColorCard_->setColor(color_);
    hexLineEdit_->setColor(color_);

    redLineEdit_->setText(QString::number(color_.red()));
    blueLineEdit_->setText(QString::number(color_.blue()));
    greenLineEdit_->setText(QString::number(color_.green()));

    if (movePicker) {
        huePanel_->setColor(color_);
    }
}

void ColorDialog::onHueChanged(const QColor& color) {
    color_.setHsv(color.hue(), color.saturation(), color_.value(), color_.alpha());
    setColor(color_);
}

void ColorDialog::onBrightnessChanged(const QColor& color) {
    color_.setHsv(color_.hue(), color_.saturation(), color.value(), color.alpha());
    setColor(color_, false);
}

void ColorDialog::onRedChanged(const QString& red) {
    color_.setRed(red.toInt());
    setColor(color_);
}

void ColorDialog::onBlueChanged(const QString& blue) {
    color_.setBlue(blue.toInt());
    setColor(color_);
}

void ColorDialog::onGreenChanged(const QString& green) {
    color_.setGreen(green.toInt());
    setColor(color_);
}

void ColorDialog::onHexColorChanged(const QString& color) {
    color_.setNamedColor(QStringLiteral("#") + color);
    setColor(color_);
}

void ColorDialog::onYesButtonClicked() {
    accept();
    if (color_ != oldColor_) {
        emit colorChanged(color_);
    }
}

void ColorDialog::updateStyle() {
    setStyle(QApplication::style());
    titleLabel_->adjustSize();
    editLabel_->adjustSize();
    redLabel_->adjustSize();
    greenLabel_->adjustSize();
    blueLabel_->adjustSize();
}

void ColorDialog::showEvent(QShowEvent* e) {
    updateStyle();
    MaskDialogBase::showEvent(e);
}

void ColorDialog::connectSignalToSlot() {
    connect(cancelButton, &QPushButton::clicked, this, &ColorDialog::reject);
    connect(yesButton, &QPushButton::clicked, this, &ColorDialog::onYesButtonClicked);

    connect(huePanel_, &HuePanel::colorChanged, this, &ColorDialog::onHueChanged);
    connect(brightSlider_, &BrightnessSlider::colorChanged, this,
            &ColorDialog::onBrightnessChanged);

    connect(redLineEdit_, &ColorLineEdit::valueChanged, this, &ColorDialog::onRedChanged);
    connect(blueLineEdit_, &ColorLineEdit::valueChanged, this, &ColorDialog::onBlueChanged);
    connect(greenLineEdit_, &ColorLineEdit::valueChanged, this, &ColorDialog::onGreenChanged);
    connect(hexLineEdit_, &ColorLineEdit::valueChanged, this, &ColorDialog::onHexColorChanged);
}

}  // namespace qfw
