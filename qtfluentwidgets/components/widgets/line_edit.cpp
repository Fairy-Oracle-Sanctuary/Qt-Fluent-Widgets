#include "components/widgets/line_edit.h"

#include <QAction>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QStyle>
#include <QTimer>

#include "common/color.h"
#include "common/font.h"
#include "common/icon.h"
#include "common/style_sheet.h"
#include "components/widgets/menu.h"
#include "components/widgets/scroll_bar.h"
#include "components/widgets/tool_tip.h"

namespace qfw {

static void drawIconVariant(const QVariant& icon, QPainter* painter, const QRect& rect,
                            const QColor& fill = QColor()) {
    if (!painter) {
        return;
    }

    if (!icon.isValid()) {
        return;
    }

    if (icon.canConvert<QIcon>()) {
        const QIcon qicon = icon.value<QIcon>();
        if (!qicon.isNull()) {
            qicon.paint(painter, rect, Qt::AlignCenter, QIcon::Normal, QIcon::Off);
        }
        return;
    }

    if (icon.canConvert<const FluentIconBase*>()) {
        const auto* fluentIcon = icon.value<const FluentIconBase*>();
        if (!fluentIcon) {
            return;
        }

        if (fill.isValid()) {
            fluentIcon->render(painter, rect, Theme::Auto,
                               QVariantMap{{QStringLiteral("fill"), fill.name()},
                                           {QStringLiteral("stroke"), fill.name()}});
        } else {
            fluentIcon->render(painter, rect, Theme::Auto);
        }
        return;
    }
}

// ============================================================================
// LineEditButton
// ============================================================================

LineEditButton::LineEditButton(const QVariant& icon, QWidget* parent)
    : QToolButton(parent), icon_(icon) {
    isPressed_ = false;
    setFixedSize(31, 23);
    setIconSize(QSize(10, 10));
    setCursor(Qt::PointingHandCursor);
    setObjectName(QStringLiteral("lineEditButton"));

    qfw::setStyleSheet(this, qfw::FluentStyleSheet::LineEdit);
}

void LineEditButton::setAction(QAction* action) {
    action_ = action;
    onActionChanged();

    if (!action_) {
        return;
    }

    connect(this, &QToolButton::clicked, action_, &QAction::trigger);
    connect(action_, &QAction::toggled, this, &QToolButton::setChecked);
    connect(action_, &QAction::changed, this, &LineEditButton::onActionChanged);

    installEventFilter(new ToolTipFilter(this, 700));
}

QAction* LineEditButton::action() const { return action_; }

void LineEditButton::onActionChanged() {
    if (!action_) {
        return;
    }

    setIcon(QVariant(action_->icon()));
    setToolTip(action_->toolTip());
    setEnabled(action_->isEnabled());
    setCheckable(action_->isCheckable());
    setChecked(action_->isChecked());
}

void LineEditButton::setIcon(const QVariant& icon) {
    icon_ = icon;
    update();
}

QVariant LineEditButton::icon() const { return icon_; }

void LineEditButton::mousePressEvent(QMouseEvent* e) {
    isPressed_ = true;
    QToolButton::mousePressEvent(e);
}

void LineEditButton::mouseReleaseEvent(QMouseEvent* e) {
    isPressed_ = false;
    QToolButton::mouseReleaseEvent(e);
}

void LineEditButton::paintEvent(QPaintEvent* e) {
    QToolButton::paintEvent(e);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    const int iw = iconSize().width();
    const int ih = iconSize().height();
    const int w = width();
    const int h = height();

    QRect rect((w - iw) / 2, (h - ih) / 2, iw, ih);

    if (isPressed_) {
        painter.setOpacity(0.7);
    }

    if (qfw::isDarkTheme()) {
        drawIconVariant(icon_, &painter, rect);
    } else {
        drawIconVariant(icon_, &painter, rect, QColor(QStringLiteral("#656565")));
    }
}

// ============================================================================
// LineEdit
// ============================================================================

LineEdit::LineEdit(QWidget* parent) : QLineEdit(parent) {
    setProperty("transparent", true);
    setProperty("qssClass", "LineEdit");

    qfw::setStyleSheet(this, qfw::FluentStyleSheet::LineEdit);
    qfw::updateDynamicStyle(this);

    setFixedHeight(33);
    setAttribute(Qt::WA_MacShowFocusRect, false);
    qfw::setFont(this);

    hBoxLayout_ = new QHBoxLayout(this);

    static qfw::FluentIcon closeIcon(qfw::FluentIconEnum::Close);
    clearButton_ = new LineEditButton(QVariant(closeIcon.qicon()), this);

    clearButton_->setFixedSize(29, 25);
    clearButton_->hide();

    hBoxLayout_->setSpacing(3);
    hBoxLayout_->setContentsMargins(4, 4, 4, 4);
    hBoxLayout_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    hBoxLayout_->addWidget(clearButton_, 0, Qt::AlignRight);

    connect(clearButton_, &QToolButton::clicked, this, &QLineEdit::clear);
    connect(this, &QLineEdit::textChanged, this, &LineEdit::onTextChanged);
}

QHBoxLayout* LineEdit::buttonLayout() const { return hBoxLayout_; }

LineEditButton* LineEdit::clearButton() const { return clearButton_; }

bool LineEdit::isError() const { return isError_; }

void LineEdit::setError(bool isError) {
    if (isError == isError_) {
        return;
    }

    isError_ = isError;
    update();
}

void LineEdit::setCustomFocusedBorderColor(const QColor& light, const QColor& dark) {
    lightFocusedBorderColor_ = QColor(light);
    darkFocusedBorderColor_ = QColor(dark);
    update();
}

QColor LineEdit::focusedBorderColor() const {
    if (isError()) {
        return qfw::getFluentSystemColor(qfw::FluentSystemColor::CriticalForeground);
    }

    const bool dark = qfw::isDarkTheme();
    const QColor base = qfw::themeColor();

    if (dark) {
        // Dark mode: use ThemeColorLight1 as default
        const QColor darkLight1 = qfw::themedColor(base, true, QStringLiteral("ThemeColorLight1"));
        return qfw::validColor(darkFocusedBorderColor_, darkLight1);
    } else {
        // Light mode: use ThemeColorPrimary as default
        return qfw::validColor(lightFocusedBorderColor_, base);
    }
}

void LineEdit::setClearButtonEnabled(bool enable) {
    isClearButtonEnabled_ = enable;
    adjustTextMargins();
}

bool LineEdit::isClearButtonEnabled() const { return isClearButtonEnabled_; }

void LineEdit::addAction(QAction* action, QLineEdit::ActionPosition position) {
    if (!action) {
        return;
    }

    QWidget::addAction(action);

    auto* button = new LineEditButton(QVariant(action->icon()), this);
    button->setAction(action);
    button->setFixedWidth(29);

    if (position == QLineEdit::LeadingPosition) {
        hBoxLayout_->insertWidget(leftButtons_.size(), button, 0, Qt::AlignLeading);
        if (leftButtons_.isEmpty()) {
            hBoxLayout_->insertStretch(1, 1);
        }
        leftButtons_.append(button);
    } else {
        rightButtons_.append(button);
        hBoxLayout_->addWidget(button, 0, Qt::AlignRight);
    }

    adjustTextMargins();
}

void LineEdit::addActions(const QList<QAction*>& actions, QLineEdit::ActionPosition position) {
    for (auto* action : actions) {
        addAction(action, position);
    }
}

void LineEdit::adjustTextMargins() {
    const int left = leftButtons_.size() * 30;
    const int right = rightButtons_.size() * 30 + (isClearButtonEnabled() ? 28 : 0);

    const QMargins m = textMargins();
    setTextMargins(left, m.top(), right, m.bottom());
}

void LineEdit::focusOutEvent(QFocusEvent* e) {
    QLineEdit::focusOutEvent(e);
    if (clearButton_) {
        clearButton_->hide();
    }
}

void LineEdit::focusInEvent(QFocusEvent* e) {
    QLineEdit::focusInEvent(e);

    if (!clearButton_) {
        return;
    }

    if (isClearButtonEnabled()) {
        clearButton_->setVisible(!text().isEmpty());
    }
}

void LineEdit::onTextChanged(const QString& text) {
    if (!clearButton_) {
        return;
    }

    if (isClearButtonEnabled()) {
        clearButton_->setVisible(!text.isEmpty() && hasFocus());
    }
}

void LineEdit::paintEvent(QPaintEvent* e) {
    QLineEdit::paintEvent(e);

    if (!hasFocus()) {
        return;
    }

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    const QMargins m = contentsMargins();

    QPainterPath path;
    const int w = width() - m.left() - m.right();
    const int h = height();

    path.addRoundedRect(QRectF(m.left(), h - 10, w, 10), 5, 5);

    QPainterPath rectPath;
    rectPath.addRect(m.left(), h - 10, w, 8);

    path = path.subtracted(rectPath);

    painter.fillPath(path, focusedBorderColor());
}

// ============================================================================
// PlainTextEdit
// ============================================================================
PlainTextEdit::PlainTextEdit(QWidget* parent) : QPlainTextEdit(parent) {
    setProperty("qssClass", "PlainTextEdit");
    qfw::setStyleSheet(this, qfw::FluentStyleSheet::LineEdit);
    qfw::updateDynamicStyle(this);
    qfw::setFont(this);

    scrollDelegate_ = new SmoothScrollDelegate(this);
}

void PlainTextEdit::contextMenuEvent(QContextMenuEvent* e) {
    if (!e) {
        return;
    }

    auto* menu = new qfw::PlainTextEditMenu(this);
    qfw::setStyleSheet(menu, qfw::FluentStyleSheet::Menu);
    menu->execAt(e->globalPos(), true, qfw::MenuAnimationType::DropDown);
}

// ============================================================================
// TextBrowser
// ============================================================================
TextBrowser::TextBrowser(QWidget* parent) : QTextBrowser(parent) {
    setProperty("qssClass", "TextBrowser");
    qfw::setStyleSheet(this, qfw::FluentStyleSheet::LineEdit);
    qfw::updateDynamicStyle(this);
    qfw::setFont(this);

    scrollDelegate_ = new SmoothScrollDelegate(this);
}

void TextBrowser::contextMenuEvent(QContextMenuEvent* e) {
    if (!e) {
        return;
    }

    auto* menu = new qfw::TextEditMenu(this);
    qfw::setStyleSheet(menu, qfw::FluentStyleSheet::Menu);
    menu->execAt(e->globalPos(), true, qfw::MenuAnimationType::DropDown);
}

// ============================================================================
// PasswordLineEdit
// ============================================================================
PasswordLineEdit::PasswordLineEdit(QWidget* parent) : LineEdit(parent) {
    setEchoMode(QLineEdit::Password);
    setContextMenuPolicy(Qt::NoContextMenu);

    static qfw::FluentIcon viewIcon(qfw::FluentIconEnum::View);
    viewButton_ = new LineEditButton(QVariant(viewIcon.qicon()), this);
    viewButton_->setIconSize(QSize(13, 13));
    viewButton_->setFixedSize(29, 25);

    if (buttonLayout()) {
        buttonLayout()->addWidget(viewButton_, 0, Qt::AlignRight);
    }

    setClearButtonEnabled(false);
    setPasswordVisible(false);

    connect(viewButton_, &QToolButton::clicked, this,
            [this]() { setPasswordVisible(!isPasswordVisible()); });
}

void PasswordLineEdit::setPasswordVisible(bool visible) {
    if (passwordVisible_ == visible) {
        return;
    }

    passwordVisible_ = visible;
    setEchoMode(passwordVisible_ ? QLineEdit::Normal : QLineEdit::Password);
    updateViewButtonIcon();
}

bool PasswordLineEdit::isPasswordVisible() const { return passwordVisible_; }

void PasswordLineEdit::updateViewButtonIcon() {
    if (!viewButton_) {
        return;
    }

    if (passwordVisible_) {
        static qfw::FluentIcon hideIcon(qfw::FluentIconEnum::Hide);
        viewButton_->setIcon(QVariant(hideIcon.qicon()));
    } else {
        static qfw::FluentIcon viewIcon(qfw::FluentIconEnum::View);
        viewButton_->setIcon(QVariant(viewIcon.qicon()));
    }
}

// ============================================================================
// SearchLineEdit
// ============================================================================
SearchLineEdit::SearchLineEdit(QWidget* parent) : LineEdit(parent) {
    static qfw::FluentIcon searchIcon(qfw::FluentIconEnum::Search);
    searchButton_ = new LineEditButton(QVariant(searchIcon.qicon()), this);

    if (buttonLayout()) {
        buttonLayout()->addWidget(searchButton_, 0, Qt::AlignRight);
    }

    setClearButtonEnabled(true);
    setTextMargins(0, 0, 59, 0);

    connect(searchButton_, &QToolButton::clicked, this, &SearchLineEdit::search);
    connect(clearButton(), &QToolButton::clicked, this, &SearchLineEdit::clearSignal);
}

void SearchLineEdit::search() {
    const QString text = this->text().trimmed();
    if (!text.isEmpty()) {
        emit searchSignal(text);
    } else {
        emit clearSignal();
    }
}

}  // namespace qfw
