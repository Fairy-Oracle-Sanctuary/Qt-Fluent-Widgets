#include "components/widgets/color_picker.h"

#include <QApplication>
#include <QFocusEvent>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QStyleOptionButton>
#include <algorithm>
#include <functional>
#include <stdexcept>

#include "common/font.h"
#include "components/dialog_box/color_dialog.h"

namespace qfw {
namespace {
QColor validColor(const QColor& color) {
    if (!color.isValid()) throw std::invalid_argument("invalid palette color");
    return QColor::fromRgba(color.rgba());
}
void checkColors(const QList<QColor>& colors) {
    if (colors.size() > 4096) throw std::invalid_argument("palette supports at most 4096 colors");
    for (const auto& color : colors) validColor(color);
}
void drawSwatch(QPainter& painter, const QRectF& rect, const QColor& color, bool circle) {
    QPainterPath path;
    if (circle)
        path.addEllipse(rect);
    else
        path.addRoundedRect(rect, 4, 4);
    if (color.alpha() < 255) {
        painter.save();
        painter.setClipPath(path);
        const QColor light(isDarkTheme() ? "#777777" : "#ffffff");
        const QColor dark(isDarkTheme() ? "#555555" : "#dddddd");
        for (int row = 0; row <= rect.height() / 4; ++row) {
            for (int column = 0; column <= rect.width() / 4; ++column) {
                painter.fillRect(QRectF(rect.x() + column * 4, rect.y() + row * 4, 4, 4),
                                 (row + column) % 2 ? dark : light);
            }
        }
        painter.restore();
    }
    painter.fillPath(path, color);
}
// Palette actions are not regular push buttons: only hover/press or keyboard
// focus adds a subtle fill, matching the Python palette's transparent rows.
class PaletteActionButton : public QAbstractButton {
public:
    PaletteActionButton(const QString& text, QWidget* parent) : QAbstractButton(parent) {
        setText(text);
        setAccessibleName(text);
        setFocusPolicy(Qt::StrongFocus);
        setCursor(Qt::PointingHandCursor);
        qfw::setFont(this);
    }
    void setColor(const QColor& color) {
        color_ = color;
        update();
    }

protected:
    bool event(QEvent* event) override {
        if (event->type() == QEvent::Enter) hovered_ = true;
        if (event->type() == QEvent::Leave) hovered_ = false;
        if (event->type() == QEvent::FocusIn) {
            const auto reason = static_cast<QFocusEvent*>(event)->reason();
            keyboardFocus_ = reason == Qt::TabFocusReason || reason == Qt::BacktabFocusReason ||
                             reason == Qt::OtherFocusReason;
        }
        const bool result = QAbstractButton::event(event);
        if (event->type() == QEvent::Enter || event->type() == QEvent::Leave ||
            event->type() == QEvent::FocusIn || event->type() == QEvent::FocusOut)
            update();
        return result;
    }
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
        if (!isEnabled()) painter.setOpacity(0.43);
        if (isEnabled() && (hovered_ || isDown() || (hasFocus() && keyboardFocus_))) {
            const int shade = isDarkTheme() ? 255 : 0;
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(shade, shade, shade, isDown() ? 22 : 12));
            QPainterPath row;
            row.setFillRule(Qt::WindingFill);
            row.addRoundedRect(QRectF(rect()), 8, 8);
            // Only the panel-facing corners are rounded; the separator edge is square.
            row.addRect(QRectF(0, color_.isValid() ? 8 : 0, width(), height() - 8));
            painter.drawPath(row);
        }
        const bool rtl = isRightToLeft();
        if (color_.isValid()) {
            drawSwatch(painter, QRectF(rtl ? width() - 39 : 11, 7, 28, 28), color_, false);
        } else {
            FluentIcon(FluentIconEnum::Palette)
                .render(&painter, QRect(rtl ? width() - 35 : 15, 11, 20, 20));
        }
        painter.setPen(isDarkTheme() ? Qt::white : Qt::black);
        painter.setFont(font());
        painter.drawText(QRect(rtl ? 12 : 51, 0, width() - 63, height()),
                         Qt::AlignVCenter | (rtl ? Qt::AlignRight : Qt::AlignLeft), text());
    }

private:
    QColor color_;
    bool hovered_ = false;
    bool keyboardFocus_ = false;
};
class SwatchButton : public QAbstractButton {
public:
    SwatchButton(const QColor& color, bool circle, QWidget* parent)
        : QAbstractButton(parent), color(color), circle(circle) {
        setFixedSize(circle ? 32 : 28, circle ? 32 : 28);
        setCheckable(circle);
        setCursor(Qt::PointingHandCursor);
        setFocusPolicy(Qt::StrongFocus);
        setAccessibleName(color.name(color.alpha() < 255 ? QColor::HexArgb : QColor::HexRgb));
        qfw::setFont(this);
    }
    QColor color;
    bool circle;
    bool selected = false;

protected:
    bool hitButton(const QPoint& point) const override {
        return circle ? (point.x() - 16) * (point.x() - 16) + (point.y() - 16) * (point.y() - 16) <=
                            256
                      : QAbstractButton::hitButton(point);
    }
    bool event(QEvent* event) override {
        if (event->type() == QEvent::Enter) hovered_ = true;
        if (event->type() == QEvent::Leave) hovered_ = false;
        if (event->type() == QEvent::FocusIn) {
            const auto reason = static_cast<QFocusEvent*>(event)->reason();
            keyboardFocus_ = reason == Qt::TabFocusReason || reason == Qt::BacktabFocusReason ||
                             reason == Qt::OtherFocusReason;
        }
        if (event->type() == QEvent::Enter || event->type() == QEvent::Leave ||
            event->type() == QEvent::FocusIn || event->type() == QEvent::FocusOut)
            update();
        return QAbstractButton::event(event);
    }
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        if (!isEnabled()) painter.setOpacity(0.43);
        const QRectF bounds = circle ? QRectF(5, 5, 22, 22) : QRectF(rect());
        const bool outlined =
            !circle && (selected || hovered_ || isDown() || (hasFocus() && keyboardFocus_));
        drawSwatch(painter, outlined ? bounds.adjusted(2, 2, -2, -2) : bounds, color, circle);
        painter.setBrush(Qt::NoBrush);
        if (circle) {
            if (isEnabled() && (underMouse() || isDown())) {
                const int shade = isDarkTheme() ? 255 : 0;
                painter.setPen(Qt::NoPen);
                painter.setBrush(QColor(shade, shade, shade, isDown() ? 30 : 15));
                painter.drawEllipse(bounds);
                painter.setBrush(Qt::NoBrush);
            }
            if (isChecked()) {
                QColor outline = color;
                outline.setAlpha(255);
                painter.setPen(QPen(outline, 1.5));
                painter.drawEllipse(QRectF(1, 1, 30, 30));
            } else if (hasFocus()) {
                painter.setPen(QPen(isDarkTheme() ? Qt::white : QColor("#606060"), 1, Qt::DotLine));
                painter.drawEllipse(QRectF(1, 1, 30, 30));
            }
        } else if (outlined) {
            const qreal inset = selected ? 0.75 : 0.5;
            painter.setPen(QPen(Qt::white, selected ? 1.5 : 1));
            painter.drawRoundedRect(bounds.adjusted(inset, inset, -inset, -inset), 4, 4);
        }
    }

private:
    bool hovered_ = false;
    bool keyboardFocus_ = false;
};
}  // namespace

CircleColorPicker::CircleColorPicker(QWidget* parent)
    : CircleColorPicker(defaultColors(), parent) {}
CircleColorPicker::CircleColorPicker(const QList<QColor>& colors, QWidget* parent)
    : QWidget(parent) {
    group_ = new QButtonGroup(this);
    layout_ = new QHBoxLayout(this);
    layout_->setContentsMargins(0, 0, 0, 0);
    layout_->setSpacing(10);
    layout_->addStretch();
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setAccessibleName(tr("Color palette"));
    setAccessibleDescription(tr("No color selected"));
    connect(&QConfig::instance(), &QConfig::themeChanged, this, [this] {
        for (auto* button : buttons_) button->update();
    });
    setColors(colors);
}
QList<QColor> CircleColorPicker::defaultColors() {
    return {QColor("#4BACF2"), QColor("#72D6F4"), QColor("#4BACF2"),
            QColor("#49A6EB"), QColor("#20396C"), QColor("#CBB0ED")};
}
QList<QColor> CircleColorPicker::colors() const { return colors_; }
int CircleColorPicker::count() const { return colors_.size(); }
QColor CircleColorPicker::color() const { return current_ >= 0 ? colors_[current_] : QColor(); }
int CircleColorPicker::currentIndex() const { return current_; }
void CircleColorPicker::setColors(const QList<QColor>& colors) {
    checkColors(colors);
    if (colors == colors_) return;
    const auto previous = color();
    const int oldIndex = current_;
    bool hadFocus = false;
    for (auto* button : buttons_) {
        hadFocus |= button->hasFocus();
        group_->removeButton(button);
        layout_->removeWidget(button);
        button->hide();
        delete button;
    }
    buttons_.clear();
    colors_ = colors;
    for (int i = 0; i < colors.size(); ++i) {
        auto* swatch = new SwatchButton(colors[i], true, this);
        swatch->installEventFilter(this);
        buttons_.append(swatch);
        group_->addButton(swatch, i);
        layout_->insertWidget(i, swatch);
        connect(swatch, &QAbstractButton::clicked, this, [this, i] { setCurrentIndex(i); });
    }
    int selected = oldIndex >= 0 && oldIndex < colors.size() && colors[oldIndex] == previous
                       ? oldIndex
                       : colors.indexOf(previous);
    if (selected < 0) selected = colors.isEmpty() ? -1 : 0;
    applySelection(selected);
    if (hadFocus && selected >= 0) buttons_[selected]->setFocus(Qt::OtherFocusReason);
    updateGeometry();
    emitChanges(oldIndex, previous);
}
void CircleColorPicker::addColors(const QList<QColor>& colors) { setColors(colors_ + colors); }
void CircleColorPicker::addColor(const QColor& color) { addColors({color}); }
void CircleColorPicker::setColor(const QColor& color) {
    validColor(color);
    if (this->color() == color) return;
    const int index = colors_.indexOf(color);
    if (index < 0) throw std::invalid_argument("selected color must be in the palette");
    setCurrentIndex(index);
}
void CircleColorPicker::setCurrentIndex(int index) {
    if (index < -1 || index >= count()) throw std::out_of_range("palette index out of range");
    if (index == current_) return;
    const auto previous = color();
    const int oldIndex = current_;
    applySelection(index);
    emitChanges(oldIndex, previous);
}
void CircleColorPicker::applySelection(int index) {
    current_ = index;
    group_->setExclusive(false);
    for (int i = 0; i < buttons_.size(); ++i) {
        const QSignalBlocker blocker(buttons_[i]);
        buttons_[i]->setChecked(i == index);
    }
    group_->setExclusive(true);
    setAccessibleDescription(index >= 0 ? color().name(QColor::HexArgb) : tr("No color selected"));
}
void CircleColorPicker::emitChanges(int oldIndex, const QColor& previous) {
    const int index = current_;
    const auto selected = color();
    QPointer<CircleColorPicker> guard(this);
    if (index != oldIndex) emit currentIndexChanged(index);
    if (!guard || current_ != index || color() != selected) return;
    if (selected != previous) emit colorChanged(selected);
}
QAbstractButton* CircleColorPicker::button(int index) const {
    return index >= 0 && index < count() ? buttons_[index] : nullptr;
}
QSize CircleColorPicker::sizeHint() const {
    return QSize(count() * 32 + std::max(0, count() - 1) * 10, 32);
}
QSize CircleColorPicker::minimumSizeHint() const { return sizeHint(); }
bool CircleColorPicker::eventFilter(QObject* watched, QEvent* event) {
    auto* swatch = qobject_cast<QAbstractButton*>(watched);
    const int old = buttons_.indexOf(swatch);
    if (old >= 0 && event->type() == QEvent::KeyPress && isEnabled()) {
        const auto key = static_cast<QKeyEvent*>(event)->key();
        if (key == Qt::Key_Return || key == Qt::Key_Enter) {
            swatch->click();
            return true;
        }
        int index = old;
        if (key == Qt::Key_Home)
            index = 0;
        else if (key == Qt::Key_End)
            index = count() - 1;
        else if (key == Qt::Key_Left || key == Qt::Key_Right || key == Qt::Key_Up ||
                 key == Qt::Key_Down) {
            int delta = key == Qt::Key_Left || key == Qt::Key_Up ? -1 : 1;
            if (isRightToLeft() && (key == Qt::Key_Left || key == Qt::Key_Right)) delta = -delta;
            index = std::clamp(old + delta, 0, count() - 1);
        } else
            return QWidget::eventFilter(watched, event);
        QPointer<CircleColorPicker> guard(this);
        QPointer<QAbstractButton> target(buttons_[index]);
        setCurrentIndex(index);
        if (guard && target) target->setFocus(Qt::TabFocusReason);
        return true;
    }
    return QWidget::eventFilter(watched, event);
}
void CircleColorPicker::changeEvent(QEvent* event) {
    QWidget::changeEvent(event);
    if (event->type() == QEvent::LanguageChange) {
        setAccessibleName(tr("Color palette"));
        setAccessibleDescription(current_ >= 0 ? color().name(QColor::HexArgb)
                                               : tr("No color selected"));
    }
}

// Paint the panel border inside its 340x432 bounds, like Python's FlyoutViewBase.
// A QSS border would reserve space in contentsRect() and widen the visible panel.
class PalettePanel : public QFrame {
public:
    using QFrame::QFrame;

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(isDarkTheme() ? QColor("#282828") : QColor(Qt::white));
        painter.setPen(isDarkTheme() ? QColor("#484848") : QColor("#dddddd"));
        painter.drawRoundedRect(QRectF(rect()).adjusted(1, 1, -1, -1), 8, 8);
    }
};
class ColorPalettePopup : public QFrame {
public:
    explicit ColorPalettePopup(DropDownColorPalette* picker)
        : QFrame(picker, Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint),
          picker_(picker) {
        // Only the rounded panel casts a shadow. The native popup shadow would
        // outline the rectangular transparent window along its right/bottom edges.
        setObjectName("colorPalettePopup");
        setAttribute(Qt::WA_TranslucentBackground);
        setAttribute(Qt::WA_DeleteOnClose);
        setLayoutDirection(picker->layoutDirection());
        auto* outer = new QHBoxLayout(this);
        outer->setContentsMargins(18, 18, 18, 18);
        frame_ = new PalettePanel(this);
        frame_->setObjectName("paletteFrame");
        auto* shadow = new QGraphicsDropShadowEffect(frame_);
        shadow->setBlurRadius(28);
        shadow->setOffset(0, 4);
        shadow->setColor(QColor(0, 0, 0, 80));
        frame_->setGraphicsEffect(shadow);
        outer->addWidget(frame_);
        auto* bodyLayout = new QHBoxLayout(frame_);
        bodyLayout->setContentsMargins(0, 0, 0, 0);
        auto* scroll = new QScrollArea(frame_);
        scroll->setFrameShape(QFrame::NoFrame);
        scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        scroll->setWidgetResizable(false);
        bodyLayout->addWidget(scroll);
        body_ = new QWidget;
        body_->setFixedSize(340, 432);
        scroll->setWidget(body_);
        body_->setAutoFillBackground(false);
        scroll->viewport()->setAutoFillBackground(false);
        automatic_ = new PaletteActionButton(DropDownColorPalette::tr("Automatic"), body_);
        automatic_->setObjectName("automaticColorButton");
        automatic_->setGeometry(1, 0, 338, 42);
        connect(automatic_, &QAbstractButton::clicked, picker,
                [picker] { picker->chooseColor(picker->automaticColor(), true); });
        auto* themeLabel = new QLabel(DropDownColorPalette::tr("Theme Colors"), body_);
        themeLabel->setGeometry(12, 43, 316, 40);
        auto* standardLabel = new QLabel(DropDownColorPalette::tr("Standard Colors"), body_);
        standardLabel->setGeometry(12, 301, 316, 40);
        qfw::setFont(themeLabel);
        qfw::setFont(standardLabel);
        auto rows = DropDownColorPalette::themeColors();
        rows.append(DropDownColorPalette::standardColors());
        for (int row = 0; row < rows.size(); ++row) {
            const int y = row == 0 ? 88 : row < 6 ? 128 + (row - 1) * 32 : 344;
            for (int column = 0; column < 10; ++column) {
                auto* button = new SwatchButton(rows[row][column], false, body_);
                button->setObjectName(QStringLiteral("paletteSwatch%1").arg(row * 10 + column));
                button->setGeometry(isRightToLeft() ? 300 - column * 32 : 12 + column * 32, y, 28,
                                    28);
                button->installEventFilter(this);
                swatches_.append(button);
                connect(button, &QAbstractButton::clicked, picker,
                        [picker, color = button->color] { picker->chooseColor(color, false); });
            }
        }
        more_ = new PaletteActionButton(DropDownColorPalette::tr("More Colors..."), body_);
        more_->setObjectName("moreColorsButton");
        more_->setGeometry(1, 390, 338, 42);
        connect(more_, &QAbstractButton::clicked, picker, &DropDownColorPalette::showColorDialog);
        automatic_->installEventFilter(this);
        more_->installEventFilter(this);
        for (int y : {42, 300, 389}) {
            auto* line = new QFrame(body_);
            line->setObjectName("paletteSeparator");
            line->setGeometry(1, y, 338, 1);
        }
        opacity_ = new QPropertyAnimation(this, "windowOpacity", this);
        opacity_->setDuration(160);
        connect(&QConfig::instance(), &QConfig::themeChanged, this, [this] { refresh(); });
        refresh();
    }
    std::function<void()> dismissed;
    void refresh() {
        const QString foreground = isDarkTheme() ? "#ffffff" : "#000000";
        frame_->setStyleSheet(
            QStringLiteral(
                "QFrame#paletteFrame {background:transparent; border:none;} "
                "QScrollArea, QScrollArea > QWidget > QWidget {background:transparent;} QLabel "
                "{color:%2;} QFrame#paletteSeparator {background:%1;}")
                .arg(isDarkTheme() ? "#484848" : "#dddddd", foreground));
        frame_->update();
        automatic_->setColor(picker_->automaticColor());
        more_->update();
        for (auto* swatch : swatches_) {
            swatch->selected = !picker_->isAutomatic() && swatch->color == picker_->color();
            swatch->update();
        }
    }
    void openAt(const QPoint& anchor, const QRect& screenBounds) {
        // The border is painted inside the panel, without extra layout padding.
        // The 340x432 content must fit without scroll bars on a normal-size monitor.
        resize(std::min(376, screenBounds.width()), std::min(468, screenBounds.height()));
        QPoint position(anchor.x() - width() / 2, anchor.y());
        if (position.y() + height() > screenBounds.bottom() + 1)
            position.setY(anchor.y() - picker_->height() - height());
        position.setX(
            std::clamp(position.x(), screenBounds.left(), screenBounds.right() + 1 - width()));
        position.setY(
            std::clamp(position.y(), screenBounds.top(), screenBounds.bottom() + 1 - height()));
        move(position);
        show();
        opacity_->setStartValue(0.0);
        opacity_->setEndValue(1.0);
        opacity_->start();
        QAbstractButton* focus = automatic_;
        for (auto* swatch : swatches_)
            if (swatch->selected) {
                focus = swatch;
                break;
            }
        focus->setFocus(Qt::PopupFocusReason);
    }

protected:
    void hideEvent(QHideEvent* event) override {
        opacity_->stop();
        QFrame::hideEvent(event);
        if (!dismissed_) {
            dismissed_ = true;
            if (dismissed) dismissed();
            deleteLater();
        }
    }
    bool eventFilter(QObject* watched, QEvent* event) override {
        if (event->type() != QEvent::KeyPress) return QFrame::eventFilter(watched, event);
        const int key = static_cast<QKeyEvent*>(event)->key();
        if (key == Qt::Key_Escape) {
            close();
            return true;
        }
        auto* button = qobject_cast<QAbstractButton*>(watched);
        if (key == Qt::Key_Return || key == Qt::Key_Enter) {
            button->click();
            return true;
        }
        if (key != Qt::Key_Left && key != Qt::Key_Right && key != Qt::Key_Up &&
            key != Qt::Key_Down && key != Qt::Key_Home && key != Qt::Key_End)
            return false;
        const int index = swatches_.indexOf(static_cast<SwatchButton*>(
            watched == automatic_ || watched == more_ ? nullptr : button));
        QAbstractButton* target = nullptr;
        if (index < 0)
            target = swatches_[watched == automatic_ ? 0 : 60];
        else {
            const int column = index % 10;
            if (key == Qt::Key_Left || key == Qt::Key_Right) {
                int delta = key == Qt::Key_Left ? -1 : 1;
                if (isRightToLeft()) delta = -delta;
                target = swatches_[index - column + (column + delta + 10) % 10];
            } else if (key == Qt::Key_Home)
                target = swatches_[index - column];
            else if (key == Qt::Key_End)
                target = swatches_[index - column + 9];
            else if (key == Qt::Key_Up)
                target =
                    index >= 10 ? static_cast<QAbstractButton*>(swatches_[index - 10]) : automatic_;
            else
                target = index < 60 ? static_cast<QAbstractButton*>(swatches_[index + 10]) : more_;
        }
        target->setFocus(Qt::OtherFocusReason);
        return true;
    }
    void keyPressEvent(QKeyEvent* event) override {
        if (event->key() == Qt::Key_Escape)
            close();
        else
            QFrame::keyPressEvent(event);
    }

private:
    QPointer<DropDownColorPalette> picker_;
    QFrame* frame_;
    QWidget* body_;
    PaletteActionButton* automatic_;
    PaletteActionButton* more_;
    QList<SwatchButton*> swatches_;
    QPropertyAnimation* opacity_;
    bool dismissed_ = false;
};

QList<QList<QColor>> DropDownColorPalette::themeColors() {
    const QList<QStringList> rows = {{"#FFFFFF", "#000000", "#E7E6E6", "#44546A", "#4472C4",
                                      "#4472C4", "#A5A5A5", "#FFC000", "#5B9BD5", "#70AD47"},
                                     {"#F2F2F2", "#808080", "#D0CECE", "#D6DCE5", "#DAE3F3",
                                      "#FBE5D6", "#EDEDED", "#FFF2CC", "#DEEBF7", "#E2F0D9"},
                                     {"#D9D9D9", "#595959", "#AFABAB", "#ADB9CA", "#B4C7E7",
                                      "#F8CBAD", "#DBDBDB", "#FFE699", "#BDD7EE", "#C5E0B4"},
                                     {"#BFBFBF", "#404040", "#767171", "#8497B0", "#8FAADC",
                                      "#F4B183", "#C9C9C9", "#FFD966", "#9DC3E6", "#A9D18E"},
                                     {"#A6A6A6", "#262626", "#3B3838", "#333F50", "#2F5597",
                                      "#C55A11", "#7C7C7C", "#BF9000", "#2E75B6", "#548235"},
                                     {"#808080", "#0D0D0D", "#181717", "#222A35", "#203864",
                                      "#843C0B", "#525252", "#806000", "#1F4E79", "#385624"}};
    QList<QList<QColor>> result;
    for (const auto& row : rows) {
        QList<QColor> colors;
        for (const auto& value : row) colors.append(QColor(value));
        result.append(colors);
    }
    return result;
}
QList<QColor> DropDownColorPalette::standardColors() {
    return {QColor("#C00000"), QColor("#FF0000"), QColor("#FFC000"), QColor("#FFFF00"),
            QColor("#92D050"), QColor("#00B050"), QColor("#00B0F0"), QColor("#0070C0"),
            QColor("#002060"), QColor("#7030A0")};
}
DropDownColorPalette::DropDownColorPalette(QWidget* parent)
    : DropDownColorPalette(QColor("#0078D4"), parent) {}
DropDownColorPalette::DropDownColorPalette(const QColor& color, QWidget* parent)
    : DropDownPushButton(parent), color_(validColor(color)) {
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setAccessibleName(tr("Color palette"));
    connect(this, &QAbstractButton::clicked, this, &DropDownColorPalette::showPalette);
    connect(&QConfig::instance(), &QConfig::themeChanged, this, [this] { update(); });
}
DropDownColorPalette::~DropDownColorPalette() {
    if (popup_) {
        popup_->dismissed = {};
        delete popup_.data();
    }
    if (dialog_) {
        disconnect(dialog_, nullptr, this, nullptr);
        delete dialog_.data();
    }
}
QColor DropDownColorPalette::color() const { return color_; }
void DropDownColorPalette::setColor(const QColor& color) { setState(validColor(color), false); }
QColor DropDownColorPalette::automaticColor() const { return automaticColor_; }
void DropDownColorPalette::setAutomaticColor(const QColor& color) {
    automaticColor_ = validColor(color);
    if (popup_) popup_->refresh();
    if (automatic_) setState(automaticColor_, true);
}
bool DropDownColorPalette::isAutomatic() const { return automatic_; }
void DropDownColorPalette::setAutomatic() { setState(automaticColor_, true); }
void DropDownColorPalette::setAlphaEnabled(bool enabled) { alphaEnabled_ = enabled; }
bool DropDownColorPalette::isAlphaEnabled() const { return alphaEnabled_; }
void DropDownColorPalette::setState(const QColor& color, bool automatic) {
    const bool changed = color_ != color;
    const bool modeChanged = automatic_ != automatic;
    color_ = color;
    automatic_ = automatic;
    update();
    if (popup_) popup_->refresh();
    QPointer<DropDownColorPalette> guard(this);
    if (modeChanged) emit automaticChanged(automatic);
    if (!guard || color_ != color || automatic_ != automatic) return;
    if (changed) emit colorChanged(color);
}
void DropDownColorPalette::chooseColor(const QColor& color, bool automatic) {
    QPointer<DropDownColorPalette> guard(this);
    closePalette();
    if (!guard) return;
    setState(color, automatic);
    if (guard) emit colorSelected(color);
}
void DropDownColorPalette::showPalette() {
    if (!isEnabled() || !isVisible() || dialog_) return;
    if (popup_) {
        closePalette();
        return;
    }
    auto* popup = new ColorPalettePopup(this);
    popup_ = popup;
    popup->dismissed = [this] {
        popup_.clear();
        setDown(false);
        if (isEnabled() && isVisible()) setFocus(Qt::PopupFocusReason);
    };
    const QPoint anchor = mapToGlobal(QPoint(width() / 2, height()));
    auto* screen = QGuiApplication::screenAt(anchor);
    if (!screen) screen = QGuiApplication::primaryScreen();
    popup->openAt(anchor, screen->availableGeometry());
}
void DropDownColorPalette::closePalette() {
    if (popup_) popup_->close();
}
bool DropDownColorPalette::isPaletteVisible() const { return popup_ && popup_->isVisible(); }
bool DropDownColorPalette::isColorDialogVisible() const { return dialog_ && dialog_->isVisible(); }
void DropDownColorPalette::showColorDialog() {
    if (!isEnabled() || !isVisible() || dialog_) return;
    closePalette();
    auto* dialog = new ColorDialog(color_, tr("Choose Color"), window(), alphaEnabled_);
    dialog_ = dialog;
    dialog->setObjectName("paletteColorDialog");
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(dialog, &QDialog::finished, this, [this, dialog](int result) {
        dialog_.clear();
        const QColor selected = QColor::fromRgba(dialog->color().rgba());
        if (result == QDialog::Accepted) chooseColor(selected, false);
    });
    dialog->open();
}
QSize DropDownColorPalette::sizeHint() const { return QSize(54, 32); }
QSize DropDownColorPalette::minimumSizeHint() const { return sizeHint(); }
void DropDownColorPalette::paintEvent(QPaintEvent*) {
    QStyleOptionButton option;
    initStyleOption(&option);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    style()->drawControl(QStyle::CE_PushButtonBevel, &option, &painter, this);
    if (!isEnabled())
        painter.setOpacity(0.3628);
    else if (isDown())
        painter.setOpacity(0.786);
    drawSwatch(painter, QRectF(isRightToLeft() ? width() - 26 : 4, (height() - 22) / 2.0, 22, 22),
               color_, false);
    FluentIcon(FluentIconEnum::ArrowDown)
        .render(&painter, QRect(isRightToLeft() ? 7 : width() - 17, height() / 2 - 5, 10, 10));
}
void DropDownColorPalette::mouseReleaseEvent(QMouseEvent* event) {
    FluentPushButton::mouseReleaseEvent(event);
}
void DropDownColorPalette::keyPressEvent(QKeyEvent* event) {
    if (isEnabled() && (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter ||
                        event->key() == Qt::Key_Down)) {
        if (!event->isAutoRepeat()) showPalette();
        event->accept();
    } else
        DropDownPushButton::keyPressEvent(event);
}
void DropDownColorPalette::hideEvent(QHideEvent* event) {
    closePalette();
    if (dialog_) dialog_->reject();
    DropDownPushButton::hideEvent(event);
}
void DropDownColorPalette::changeEvent(QEvent* event) {
    if (event->type() == QEvent::EnabledChange && !isEnabled()) {
        closePalette();
        if (dialog_) dialog_->reject();
    } else if (event->type() == QEvent::LanguageChange) {
        closePalette();
        setAccessibleName(tr("Color palette"));
    }
    DropDownPushButton::changeEvent(event);
}
}  // namespace qfw
