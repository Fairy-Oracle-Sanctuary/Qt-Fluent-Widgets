#include "components/widgets/pager.h"

#include <QHBoxLayout>
#include <QIntValidator>
#include <QPainter>
#include <algorithm>
#include <limits>
#include <stdexcept>

#include "common/config.h"
#include "common/font.h"
#include "common/style_sheet.h"
#include "components/widgets/button.h"
#include "components/widgets/label.h"
#include "components/widgets/line_edit.h"

namespace qfw {
namespace {
void validatePages(int pages) {
    if (pages < 0) {
        throw std::invalid_argument("Page count must be nonnegative");
    }
}
void validateVisible(int count) {
    if (count < 1 || count > 25) {
        throw std::invalid_argument("Visible page count must be between 1 and 25");
    }
}
}  // namespace
PageButton::PageButton(int page, bool selected, QWidget* parent)
    : QAbstractButton(parent), page_(page) {
    setFixedSize(35, 35);
    setFocusPolicy(Qt::StrongFocus);
    setCheckable(true);
    setFont(getFont());
    setPage(page);
    setSelected(selected);
    connect(this, &QAbstractButton::clicked, this, [this] { emit clicked(page_); });
    connect(&QConfig::instance(), &QConfig::themeChanged, this, [this] { update(); });
    connect(&QConfig::instance(), &QConfig::themeColorChanged, this, [this] { update(); });
}
int PageButton::page() const { return page_; }
void PageButton::setPage(int page) {
    if (page < 1) {
        throw std::invalid_argument("Page button number must be positive");
    }
    page_ = page;
    setText(QString::number(page));
    setFixedSize(qMax(35, fontMetrics().horizontalAdvance(text()) + 16), 35);
    setAccessibleName(tr("Page %1").arg(page));
}
void PageButton::setSelected(bool selected) { setChecked(selected); }
bool PageButton::isSelected() const { return isChecked(); }
QSize PageButton::sizeHint() const {
    return QSize(qMax(35, fontMetrics().horizontalAdvance(text()) + 16), 35);
}
// Page selection belongs to the Pager; re-clicking must not unselect the page.
void PageButton::nextCheckState() {}
void PageButton::enterEvent(enterEvent_QEnterEvent* event) {
    hovered_ = true;
    update();
    QAbstractButton::enterEvent(event);
}
void PageButton::leaveEvent(QEvent* event) {
    hovered_ = false;
    update();
    QAbstractButton::leaveEvent(event);
}
void PageButton::changeEvent(QEvent* event) {
    if (event->type() == QEvent::FontChange || event->type() == QEvent::LanguageChange) {
        setPage(page_);
    }
    QAbstractButton::changeEvent(event);
}
void PageButton::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    if (!isEnabled()) {
        painter.setOpacity(0.4);
    }
    const bool dark = isDarkTheme();
    const QRectF bounds = QRectF(rect()).adjusted(1, 1, -1, -1);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::NoBrush);
    if (isSelected()) {
        painter.setBrush(themeColor());
    } else if (hovered_ || isDown()) {
        const int color = dark ? 255 : 0;
        painter.setPen(QColor(color, color, color, 64));
        painter.setBrush(dark ? QColor(46, 46, 46) : QColor(224, 224, 224));
    }
    painter.drawRoundedRect(bounds, 6, 6);
    const QColor foreground =
        isSelected() ? QColor(dark ? Qt::black : Qt::white) : QColor(dark ? Qt::white : Qt::black);
    painter.setPen(foreground);
    painter.setFont(font());
    painter.drawText(bounds, Qt::AlignCenter, text());
    if (hasFocus() && testAttribute(Qt::WA_KeyboardFocusChange)) {
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(foreground, 1, Qt::DashLine));
        painter.drawRoundedRect(bounds.adjusted(3, 3, -3, -3), 4, 4);
    }
}

Pager::Pager(QWidget* parent) : Pager(1, 5, parent) {}
Pager::Pager(int pages, int visible, QWidget* parent)
    : QWidget(parent), pages_(pages), currentPage_(pages ? 1 : 0), maxVisible_(visible) {
    validatePages(pages);
    validateVisible(visible);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    layout_ = new QHBoxLayout(this);
    layout_->setContentsMargins(0, 0, 0, 0);
    layout_->setSpacing(4);
    layout_->setAlignment(Qt::AlignVCenter);
    topLabel = new BodyLabel(QStringLiteral("..."), this);
    bottomLabel = new BodyLabel(QStringLiteral("..."), this);
    for (auto* label : {topLabel, bottomLabel}) {
        label->setFixedWidth(20);
        label->setAlignment(Qt::AlignCenter);
    }
    topButton = new TransparentToolButton(this);
    bottomButton = new TransparentToolButton(this);
    previousButton = new TransparentToolButton(this);
    nextButton = new TransparentToolButton(this);
    for (auto* button : {topButton, bottomButton, previousButton, nextButton}) {
        button->setFixedSize(35, 35);
        button->setIconSize(QSize(16, 16));
    }
    jumpLabel = new BodyLabel(this);
    countLabel = new BodyLabel(this);
    jumpEdit = new LineEdit(this);
    jumpEdit->setFixedWidth(64);
    jumpEdit->setAlignment(Qt::AlignCenter);
    validator_ = new QIntValidator(1, qMax(1, pages_), jumpEdit);
    jumpEdit->setValidator(validator_);
    updateLabels();
    updateDirection();
    connect(topButton, &QAbstractButton::clicked, this, [this] { setCurrentPage(1); });
    connect(bottomButton, &QAbstractButton::clicked, this, [this] { setCurrentPage(pages_); });
    connect(previousButton, &QAbstractButton::clicked, this, [this] {
        if (currentPage_ > 1) {
            setCurrentPage(currentPage_ - 1);
        }
    });
    connect(nextButton, &QAbstractButton::clicked, this, [this] {
        if (currentPage_ < pages_) {
            setCurrentPage(currentPage_ + 1);
        }
    });
    connect(jumpEdit, &QLineEdit::returnPressed, this, &Pager::jumpToPage);
    rebuild();
}
int Pager::pages() const { return pages_; }
int Pager::currentPage() const { return currentPage_; }
int Pager::maxVisible() const { return maxVisible_; }
void Pager::setPages(int pages) {
    validatePages(pages);
    if (pages_ == pages) {
        return;
    }
    const int previous = currentPage_;
    pages_ = pages;
    currentPage_ = pages ? qBound(1, currentPage_, pages) : 0;
    validator_->setRange(1, qMax(1, pages_));
    updateLabels();
    rebuild();
    if (previous != currentPage_) {
        emit currentPageChanged(currentPage_);
    }
}
void Pager::setCurrentPage(int page) {
    if (page < 1 || page > pages_ || currentPage_ == page) {
        return;
    }
    currentPage_ = page;
    rebuild();
    emit currentPageChanged(page);
}
void Pager::setMaxVisible(int number) {
    validateVisible(number);
    if (maxVisible_ == number) {
        return;
    }
    maxVisible_ = number;
    rebuild();
}
QList<int> Pager::visiblePages() const { return visiblePages_; }
PageButton* Pager::pageButton(int page) const {
    for (auto* button : buttons_) {
        if (!button->isHidden() && button->page() == page) {
            return button;
        }
    }
    return nullptr;
}
void Pager::jumpToPage() {
    bool valid;
    const int page = jumpEdit->text().toInt(&valid);
    if (valid && page >= 1 && page <= pages_) {
        setCurrentPage(page);
    }
    jumpEdit->setText(pages_ ? QString::number(currentPage_) : QString());
}
void Pager::updateLabels() {
    jumpLabel->setText(tr("Page"));
    countLabel->setText(tr("Total %1").arg(pages_));
    const QStringList tips = {tr("Jump to first page"), tr("Jump to last page"),
                              tr("Previous page"), tr("Next page")};
    const QList<TransparentToolButton*> controls = {topButton, bottomButton, previousButton,
                                                    nextButton};
    for (int i = 0; i < controls.size(); ++i) {
        controls[i]->setToolTip(tips[i]);
        controls[i]->setAccessibleName(tips[i]);
    }
    jumpEdit->setAccessibleName(tr("Jump to page"));
}
void Pager::updateDirection() {
    const bool rtl = isRightToLeft();
    topButton->setIcon(FluentIcon(rtl ? FluentIconEnum::SkipForward : FluentIconEnum::SkipBack));
    bottomButton->setIcon(FluentIcon(rtl ? FluentIconEnum::SkipBack : FluentIconEnum::SkipForward));
    previousButton->setIcon(
        FluentIcon(rtl ? FluentIconEnum::CareRightSolid : FluentIconEnum::CareLeftSolid));
    nextButton->setIcon(
        FluentIcon(rtl ? FluentIconEnum::CareLeftSolid : FluentIconEnum::CareRightSolid));
}
void Pager::rebuild() {
    // Reuse page buttons; changing page must neither leak widgets nor steal keyboard focus.
    const bool pageHadFocus = std::any_of(buttons_.cbegin(), buttons_.cend(),
                                          [](PageButton* button) { return button->hasFocus(); });
    while (auto* item = layout_->takeAt(0)) {
        if (item->widget()) {
            item->widget()->hide();
        }
        delete item;
    }
    visiblePages_.clear();
    int start = qMax(1, currentPage_ - maxVisible_ / 2);
    int end = int(qMin(qint64(pages_), qint64(start) + maxVisible_ - 1));
    if (end - start + 1 < maxVisible_) {
        start = qMax(1, end - maxVisible_ + 1);
    }
    if (start > 1) {
        visiblePages_.append(1);
    }
    for (qint64 page = start; page <= end; ++page) {
        visiblePages_.append(int(page));
    }
    if (end < pages_) {
        visiblePages_.append(pages_);
    }
    while (buttons_.size() < visiblePages_.size()) {
        auto* button = new PageButton(1, false, this);
        connect(button, &PageButton::clicked, this, &Pager::setCurrentPage);
        buttons_.append(button);
    }
    auto add = [this](QWidget* widget) {
        layout_->addWidget(widget, 0, Qt::AlignVCenter);
        widget->show();
    };
    add(topButton);
    add(previousButton);
    for (int i = 0; i < visiblePages_.size(); ++i) {
        if (i && visiblePages_[i] - visiblePages_[i - 1] > 1) {
            add(i == 1 ? topLabel : bottomLabel);
        }
        auto* button = buttons_[i];
        button->setPage(visiblePages_[i]);
        button->setSelected(visiblePages_[i] == currentPage_);
        add(button);
    }
    add(nextButton);
    add(bottomButton);
    topButton->setEnabled(currentPage_ > 1);
    previousButton->setEnabled(currentPage_ > 1);
    nextButton->setEnabled(currentPage_ < pages_);
    bottomButton->setEnabled(currentPage_ < pages_);
    layout_->addStretch(1);
    add(jumpLabel);
    add(jumpEdit);
    add(countLabel);
    jumpEdit->setEnabled(pages_ > 0);
    jumpEdit->setText(pages_ ? QString::number(currentPage_) : QString());
    if (pageHadFocus) {
        if (auto* selected = pageButton(currentPage_)) {
            selected->setFocus(Qt::OtherFocusReason);
        }
    }
    updateGeometry();
}
void Pager::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LayoutDirectionChange) {
        updateDirection();
    }
    if (event->type() == QEvent::LanguageChange) {
        updateLabels();
    }
    QWidget::changeEvent(event);
}
}  // namespace qfw
