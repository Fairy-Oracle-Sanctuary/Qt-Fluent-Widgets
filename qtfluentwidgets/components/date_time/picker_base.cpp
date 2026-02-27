#include "picker_base.h"

#include <QApplication>
#include <QEasingCurve>
#include <QEnterEvent>
#include <QFontMetrics>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QListWidgetItem>
#include <QMouseEvent>
#include <QPainter>
#include <QPropertyAnimation>
#include <QRegion>
#include <QResizeEvent>
#include <QScrollBar>
#include <QSizePolicy>
#include <QVBoxLayout>

#include "common/color.h"
#include "common/screen.h"
#include "common/style_sheet.h"
#include "components/widgets/cycle_list_widget.h"

namespace qfw {

SeparatorWidget::SeparatorWidget(Qt::Orientation orient, QWidget* parent) : QWidget(parent) {
    if (orient == Qt::Horizontal) {
        setFixedHeight(1);
    } else {
        setFixedWidth(1);
    }

    setAttribute(Qt::WA_StyledBackground);
    setProperty("qssClass", QStringLiteral("SeparatorWidget"));
    qfw::setStyleSheet(this, FluentStyleSheet::TimePicker);
}

ItemMaskWidget::ItemMaskWidget(QList<CycleListWidget*>* listWidgets, QWidget* parent)
    : QWidget(parent), listWidgets_(listWidgets) {
    setFixedHeight(37);
    setProperty("qssClass", QStringLiteral("ItemMaskWidget"));
    qfw::setStyleSheet(this, FluentStyleSheet::TimePicker);
}

void ItemMaskWidget::setCustomBackgroundColor(const QColor& light, const QColor& dark) {
    lightBackgroundColor_ = light;
    darkBackgroundColor_ = dark;
    update();
}

void ItemMaskWidget::paintEvent(QPaintEvent* e) {
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    // draw background
    painter.setPen(Qt::NoPen);

    // Use themeColorLight1 in dark theme as default background
    QColor bgColor;
    if (isDarkTheme()) {
        bgColor = darkBackgroundColor_.isValid()
                      ? darkBackgroundColor_
                      : themedColor(themeColor(), true, QStringLiteral("ThemeColorLight1"));
    } else {
        bgColor = lightBackgroundColor_.isValid() ? lightBackgroundColor_ : themeColor();
    }
    painter.setBrush(bgColor);
    painter.drawRoundedRect(rect().adjusted(4, 0, -3, 0), 5, 5);

    // draw text - draw the currently visible center item text
    painter.setPen(isDarkTheme() ? Qt::black : Qt::white);
    painter.setFont(font());

    int xOffset = 0;
    if (!listWidgets_) return;

    for (int i = 0; i < listWidgets_->count(); ++i) {
        auto* listWidget = (*listWidgets_)[i];
        if (!listWidget || !listWidget->currentItem()) {
            continue;
        }

        // Get the current selected item
        auto* item = listWidget->currentItem();
        int itemWidth = item->sizeHint().width();

        // Draw the item text centered in the mask area
        // The mask is positioned at the center row of the list widget
        Qt::Alignment align = static_cast<Qt::Alignment>(item->textAlignment());
        QRectF textRect;

        if (align & Qt::AlignLeft) {
            textRect = QRectF(xOffset + 15, 0, itemWidth, height());
        } else if (align & Qt::AlignRight) {
            textRect = QRectF(xOffset + 4, 0, itemWidth - 15, height());
        } else {
            textRect = QRectF(xOffset + 4, 0, itemWidth, height());
        }

        // Center vertically
        painter.drawText(textRect, align | Qt::AlignVCenter, item->text());

        xOffset += (itemWidth + 8);  // margin: 0 4px;
    }
}

void ItemMaskWidget::drawText(QListWidgetItem* item, QPainter* painter, int y) {
    if (!item || !painter) return;

    auto align = item->textAlignment();
    int w = item->sizeHint().width();
    int h = item->sizeHint().height();

    QRectF rect;
    if (align & Qt::AlignLeft) {
        rect = QRectF(15, y, w, h);  // padding-left: 11px
    } else if (align & Qt::AlignRight) {
        rect = QRectF(4, y, w - 15, h);  // padding-right: 11px
    } else if (align & Qt::AlignCenter) {
        rect = QRectF(4, y, w, h);
    }

    painter->drawText(rect, align, item->text());
}

PickerColumnFormatter::PickerColumnFormatter(QObject* parent) : QObject(parent) {}

QString PickerColumnFormatter::encode(const QVariant& value) const { return value.toString(); }

QVariant PickerColumnFormatter::decode(const QString& value) const { return value; }

QVariant DigitFormatter::decode(const QString& value) const {
    bool ok = false;
    const int v = value.toInt(&ok);
    if (ok) {
        return v;
    }
    return value;
}

PickerColumnButton::PickerColumnButton(const QString& name, const QVariantList& items, int width,
                                       Qt::Alignment align, PickerColumnFormatter* formatter,
                                       QWidget* parent)
    : QPushButton(name, parent), name_(name) {
    setItems(items);
    setAlignment(align);
    setFormatter(formatter);
    setFixedSize(width, 30);
    setObjectName(QStringLiteral("pickerButton"));
    setProperty("hasBorder", false);
    setAttribute(Qt::WA_TransparentForMouseEvents);
}

Qt::Alignment PickerColumnButton::align() const { return align_; }

void PickerColumnButton::setAlignment(Qt::Alignment align) {
    if (align == Qt::AlignLeft) {
        setProperty("align", "left");
    } else if (align == Qt::AlignRight) {
        setProperty("align", "right");
    } else {
        setProperty("align", "center");
    }

    align_ = align;
    setStyle(QApplication::style());
}

QString PickerColumnButton::value() const {
    if (!value_.isValid() || value_.isNull()) {
        return QString();
    }

    if (!formatter_) {
        return value_.toString();
    }

    return formatter_->encode(value_);
}

void PickerColumnButton::setValue(const QVariant& v) {
    value_ = v;
    if (!value_.isValid() || value_.isNull()) {
        setText(name());
        setProperty("hasValue", false);
    } else {
        setText(value());
        setProperty("hasValue", true);
    }

    setStyle(QApplication::style());
}

QStringList PickerColumnButton::items() const {
    QStringList out;
    if (!formatter_) {
        for (const auto& v : items_) {
            out.append(v.toString());
        }
        return out;
    }

    for (const auto& v : items_) {
        out.append(formatter_->encode(v));
    }
    return out;
}

void PickerColumnButton::setItems(const QVariantList& items) { items_ = items; }

PickerColumnFormatter* PickerColumnButton::formatter() const { return formatter_; }

void PickerColumnButton::setFormatter(PickerColumnFormatter* formatter) {
    formatter_ = formatter;
    if (!formatter_) {
        formatter_ = new PickerColumnFormatter(this);
    } else if (!formatter_->parent()) {
        formatter_->setParent(this);
    }
}

QString PickerColumnButton::name() const { return name_; }

void PickerColumnButton::setName(const QString& name) {
    if (text() == name_) {
        setText(name);
    }
    name_ = name;
}

void PickerToolButton::drawIcon(QPainter* painter, const QRectF& rect, QIcon::State state) {
    if (isPressed) {
        painter->setOpacity(1);
    }
    TransparentToolButton::drawIcon(painter, rect, state);
}

PickerBase::PickerBase(QWidget* parent) : QPushButton(parent) {
    setProperty("qssClass", QStringLiteral("PickerBase"));
    hBoxLayout_ = new QHBoxLayout(this);
    hBoxLayout_->setSpacing(0);
    hBoxLayout_->setContentsMargins(0, 0, 0, 0);
    hBoxLayout_->setSizeConstraint(QHBoxLayout::SetFixedSize);

    qfw::setStyleSheet(this, FluentStyleSheet::TimePicker);
    connect(this, &QPushButton::clicked, this, &PickerBase::showPanel);
}

void PickerBase::setSelectedBackgroundColor(const QColor& light, const QColor& dark) {
    lightSelectedBackgroundColor_ = light;
    darkSelectedBackgroundColor_ = dark;
}

void PickerBase::addColumn(const QString& name, const QVariantList& items, int width,
                           Qt::Alignment align, PickerColumnFormatter* formatter) {
    auto* button = new PickerColumnButton(name, items, width, align, formatter, this);
    columns_.append(button);
    hBoxLayout_->addWidget(button, 0, Qt::AlignLeft);

    for (int i = 0; i < columns_.count() - 1; ++i) {
        columns_[i]->setProperty("hasBorder", true);
        columns_[i]->setStyle(QApplication::style());
    }
}

bool PickerBase::isValidColumnIndex(int index) const {
    return index >= 0 && index < columns_.count();
}

void PickerBase::setColumnAlignment(int index, Qt::Alignment align) {
    if (!isValidColumnIndex(index)) {
        return;
    }
    columns_[index]->setAlignment(align);
}

void PickerBase::setColumnWidth(int index, int width) {
    if (!isValidColumnIndex(index)) {
        return;
    }
    columns_[index]->setFixedWidth(width);
}

void PickerBase::setColumnTight(int index) {
    if (!isValidColumnIndex(index)) {
        return;
    }

    const QFontMetrics fm(font());
    int w = 0;
    const QStringList its = columns_[index]->items();
    for (const auto& s : its) {
        w = qMax(w, fm.horizontalAdvance(s));
    }
    setColumnWidth(index, w + 30);
}

void PickerBase::setColumnVisible(int index, bool isVisible) {
    if (!isValidColumnIndex(index)) {
        return;
    }
    columns_[index]->setVisible(isVisible);
}

QStringList PickerBase::value() const {
    QStringList out;
    for (auto* c : columns_) {
        if (c && c->isVisible()) {
            out.append(c->value());
        }
    }
    return out;
}

void PickerBase::setColumnValue(int index, const QVariant& value) {
    if (!isValidColumnIndex(index)) {
        return;
    }
    columns_[index]->setValue(value);
}

void PickerBase::setColumnFormatter(int index, PickerColumnFormatter* formatter) {
    if (!isValidColumnIndex(index)) {
        return;
    }
    columns_[index]->setFormatter(formatter);
}

void PickerBase::setColumnItems(int index, const QVariantList& items) {
    if (!isValidColumnIndex(index)) {
        return;
    }
    columns_[index]->setItems(items);
}

QString PickerBase::encodeValue(int index, const QVariant& value) const {
    if (!isValidColumnIndex(index)) {
        return QString();
    }
    auto* fmt = columns_[index]->formatter();
    return fmt ? fmt->encode(value) : value.toString();
}

QVariant PickerBase::decodeValue(int index, const QString& value) const {
    if (!isValidColumnIndex(index)) {
        return QVariant();
    }
    auto* fmt = columns_[index]->formatter();
    return fmt ? fmt->decode(value) : QVariant(value);
}

void PickerBase::setColumn(int index, const QString& name, const QVariantList& items, int width,
                           Qt::Alignment align) {
    if (!isValidColumnIndex(index)) {
        return;
    }

    auto* button = columns_[index];
    button->setText(name);
    button->setItems(items);
    button->setFixedWidth(width);
    button->setAlignment(align);
}

void PickerBase::clearColumns() {
    while (!columns_.isEmpty()) {
        auto* btn = columns_.takeLast();
        if (!btn) {
            continue;
        }
        hBoxLayout_->removeWidget(btn);
        btn->setParent(nullptr);
        btn->deleteLater();
    }
}

void PickerBase::setButtonProperty(const char* name, bool value) {
    for (auto* button : columns_) {
        if (!button) {
            continue;
        }
        button->setProperty(name, value);
        button->setStyle(QApplication::style());
    }
}

QStringList PickerBase::panelInitialValue() const { return this->value(); }

void PickerBase::setScrollButtonRepeatEnabled(bool enabled) {
    scrollButtonRepeatEnabled_ = enabled;
}

bool PickerBase::isResetEnabled() const { return resetEnabled_; }

void PickerBase::setResetEnabled(bool enabled) { resetEnabled_ = enabled; }

void PickerBase::reset() {
    for (int i = 0; i < columns_.count(); ++i) {
        setColumnValue(i, QVariant());
    }
}

void PickerBase::enterEvent(QEnterEvent* e) {
    setButtonProperty("enter", true);
    QPushButton::enterEvent(e);
}

void PickerBase::leaveEvent(QEvent* e) {
    setButtonProperty("enter", false);
    QPushButton::leaveEvent(e);
}

void PickerBase::mousePressEvent(QMouseEvent* e) {
    setButtonProperty("pressed", true);
    QPushButton::mousePressEvent(e);
}

void PickerBase::mouseReleaseEvent(QMouseEvent* e) {
    setButtonProperty("pressed", false);
    QPushButton::mouseReleaseEvent(e);
}

void PickerBase::showPanel() {
    auto* panel = new PickerPanel(this);

    for (auto* column : columns_) {
        if (column && column->isVisible()) {
            panel->addColumn(column->items(), column->width(), column->align());
        }
    }

    panel->setValue(panelInitialValue());
    panel->setResetEnabled(isResetEnabled());
    panel->setScrollButtonRepeatEnabled(scrollButtonRepeatEnabled_);
    panel->setSelectedBackgroundColor(lightSelectedBackgroundColor_, darkSelectedBackgroundColor_);

    connect(panel, &PickerPanel::confirmed, this, [this](const QStringList& v) { onConfirmed(v); });
    connect(panel, &PickerPanel::resetted, this, &PickerBase::reset);
    connect(panel, &PickerPanel::columnValueChanged, this,
            [this](int i, const QString& v) { onColumnValueChanged(i, v); });

    const int w = panel->sizeHint().width() - width();
    panel->exec(mapToGlobal(QPoint(-w / 2, -37 * 4)));
}

void PickerBase::onColumnValueChanged(int index, const QString& value) {
    Q_UNUSED(index);
    Q_UNUSED(value);
}

void PickerBase::onConfirmed(const QStringList& value) {
    for (int i = 0; i < value.size(); ++i) {
        setColumnValue(i, value[i]);
    }
}

PickerPanel::PickerPanel(QWidget* parent) : QWidget(parent) {
    view_ = new QFrame(this);
    itemMaskWidget_ = new ItemMaskWidget(&listWidgets_, this);  // pass pointer, not copy
    hSeparatorWidget_ = new SeparatorWidget(Qt::Horizontal, view_);
    yesButton_ = new PickerToolButton(FluentIconEnum::Accept, view_);
    resetButton_ = new PickerToolButton(FluentIconEnum::Cancel, view_);
    cancelButton_ = new PickerToolButton(FluentIconEnum::Close, view_);

    hBoxLayout_ = new QHBoxLayout(this);
    listLayout_ = new QHBoxLayout();
    buttonLayout_ = new QHBoxLayout();
    vBoxLayout_ = new QVBoxLayout(view_);

    initWidget();
}

void PickerPanel::initWidget() {
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    setShadowEffect();

    yesButton_->setIconSize(QSize(16, 16));
    resetButton_->setIconSize(QSize(16, 16));
    cancelButton_->setIconSize(QSize(13, 13));

    yesButton_->setFixedHeight(33);
    cancelButton_->setFixedHeight(33);
    resetButton_->setFixedHeight(33);

    hBoxLayout_->setContentsMargins(12, 8, 12, 20);
    hBoxLayout_->addWidget(view_, 1, Qt::AlignCenter);
    hBoxLayout_->setSizeConstraint(QHBoxLayout::SetMinimumSize);

    vBoxLayout_->setSpacing(0);
    vBoxLayout_->setContentsMargins(0, 0, 0, 0);
    vBoxLayout_->addLayout(listLayout_, 1);
    vBoxLayout_->addWidget(hSeparatorWidget_);
    vBoxLayout_->addLayout(buttonLayout_, 1);
    vBoxLayout_->setSizeConstraint(QVBoxLayout::SetMinimumSize);

    buttonLayout_->setSpacing(6);
    buttonLayout_->setContentsMargins(3, 3, 3, 3);
    buttonLayout_->addWidget(yesButton_);
    buttonLayout_->addWidget(resetButton_);
    buttonLayout_->addWidget(cancelButton_);

    yesButton_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    resetButton_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    cancelButton_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(yesButton_, &QToolButton::clicked, this, &PickerPanel::fadeOut);
    connect(yesButton_, &QToolButton::clicked, this, [this]() { emit confirmed(value()); });
    connect(cancelButton_, &QToolButton::clicked, this, &PickerPanel::fadeOut);

    connect(resetButton_, &QToolButton::clicked, this, &PickerPanel::resetted);
    connect(resetButton_, &QToolButton::clicked, this, &PickerPanel::fadeOut);

    setResetEnabled(false);

    view_->setObjectName(QStringLiteral("view"));
    setProperty("qssClass", QStringLiteral("PickerPanel"));
    qfw::setStyleSheet(this, FluentStyleSheet::TimePicker);
}

void PickerPanel::setShadowEffect(int blurRadius, const QPoint& offset, const QColor& color) {
    if (!shadowEffect_) {
        shadowEffect_ = new QGraphicsDropShadowEffect(view_);
    }

    shadowEffect_->setBlurRadius(blurRadius);
    shadowEffect_->setOffset(offset);
    shadowEffect_->setColor(color);
    view_->setGraphicsEffect(nullptr);
    view_->setGraphicsEffect(shadowEffect_);
}

void PickerPanel::setResetEnabled(bool enabled) { resetButton_->setVisible(enabled); }

void PickerPanel::setScrollButtonRepeatEnabled(bool enabled) {
    scrollButtonRepeatEnabled_ = enabled;
    for (auto* widget : listWidgets_) {
        if (widget) {
            widget->setScrollButtonRepeatEnabled(enabled);
        }
    }
}

void PickerPanel::setSelectedBackgroundColor(const QColor& light, const QColor& dark) {
    itemMaskWidget_->setCustomBackgroundColor(light, dark);
}

bool PickerPanel::isResetEnabled() const { return resetButton_->isVisible(); }

void PickerPanel::addColumn(const QStringList& items, int width, Qt::Alignment align) {
    if (!listWidgets_.isEmpty()) {
        listLayout_->addWidget(new SeparatorWidget(Qt::Vertical, view_));
    }

    auto* w = new CycleListWidget(items, QSize(width, itemHeight_), align, this);
    w->setScrollButtonRepeatEnabled(scrollButtonRepeatEnabled_);

    connect(w, &CycleListWidget::cycleCurrentItemChanged, itemMaskWidget_,
            QOverload<>::of(&QWidget::update));
    connect(w->verticalScrollBar(), &QScrollBar::valueChanged, itemMaskWidget_,
            QOverload<>::of(&QWidget::update));

    const int idx = listWidgets_.size();
    connect(w, &CycleListWidget::cycleCurrentItemChanged, this, [this, idx](QListWidgetItem* item) {
        if (item) {
            emit columnValueChanged(idx, item->text());
        }
    });

    listWidgets_.append(w);
    listLayout_->addWidget(w);
}

QStringList PickerPanel::value() const {
    QStringList out;
    for (auto* w : listWidgets_) {
        if (w && w->currentItem()) {
            out.append(w->currentItem()->text());
        }
    }
    return out;
}

void PickerPanel::setValue(const QStringList& value) {
    if (value.size() != listWidgets_.size()) {
        return;
    }

    for (int i = 0; i < value.size(); ++i) {
        listWidgets_[i]->setSelectedItem(value[i]);
    }
}

QString PickerPanel::columnValue(int index) const {
    if (index < 0 || index >= listWidgets_.size()) {
        return QString();
    }

    auto* item = listWidgets_[index]->currentItem();
    return item ? item->text() : QString();
}

void PickerPanel::setColumnValue(int index, const QString& value) {
    if (index < 0 || index >= listWidgets_.size()) {
        return;
    }
    listWidgets_[index]->setSelectedItem(value);
}

CycleListWidget* PickerPanel::column(int index) const {
    if (index < 0 || index >= listWidgets_.size()) {
        return nullptr;
    }
    return listWidgets_[index];
}

void PickerPanel::resizeEvent(QResizeEvent* e) {
    QWidget::resizeEvent(e);

    itemMaskWidget_->resize(view_->width() - 3, itemHeight_);
    // ItemMaskWidget is child of PickerPanel, position relative to PickerPanel
    QMargins m = hBoxLayout_->contentsMargins();
    itemMaskWidget_->move(m.left() + 2, m.top() + 148);
}

void PickerPanel::exec(const QPoint& pos, bool ani) {
    if (isVisible()) {
        return;
    }

    show();

    QPoint p = pos;
    const QRect rect = qfw::getCurrentScreenGeometry();
    const int w = width() + 5;
    const int h = height();

    p.setX(qMin(p.x() - layout()->contentsMargins().left(), rect.right() - w));
    p.setY(qMax(rect.top(), qMin(p.y() - 4, rect.bottom() - h + 5)));
    move(p);

    if (!ani) {
        return;
    }

    isExpanded_ = false;
    if (ani_) {
        ani_->stop();
        ani_->deleteLater();
    }
    ani_ = new QPropertyAnimation(view_, "windowOpacity", this);
    connect(ani_, &QPropertyAnimation::valueChanged, this, &PickerPanel::onAniValueChanged);
    ani_->setStartValue(0);
    ani_->setEndValue(1);
    ani_->setDuration(150);
    ani_->setEasingCurve(QEasingCurve::OutQuad);
    ani_->start();
}

void PickerPanel::onAniValueChanged(const QVariant& opacity) {
    const qreal op = opacity.toReal();
    const auto m = layout()->contentsMargins();
    const int w = view_->width() + m.left() + m.right() + 120;
    const int h = view_->height() + m.top() + m.bottom() + 12;

    if (!isExpanded_) {
        const int y = int(h / 2.0 * (1 - op));
        setMask(QRegion(0, y, w, h - y * 2));
    } else {
        const int y = int(h / 3.0 * (1 - op));
        setMask(QRegion(0, y, w, h - y * 2));
    }
}

void PickerPanel::fadeOut() {
    isExpanded_ = true;
    if (ani_) {
        ani_->stop();
        ani_->deleteLater();
    }
    ani_ = new QPropertyAnimation(this, "windowOpacity", this);
    connect(ani_, &QPropertyAnimation::valueChanged, this, &PickerPanel::onAniValueChanged);
    connect(ani_, &QPropertyAnimation::finished, this, &PickerPanel::deleteLater);
    ani_->setStartValue(1);
    ani_->setEndValue(0);
    ani_->setDuration(150);
    ani_->setEasingCurve(QEasingCurve::OutQuad);
    ani_->start();
}

}  // namespace qfw
