#include "components/widgets/progress_info_bar.h"

#include <QCloseEvent>
#include <QHideEvent>
#include <QShowEvent>

#include "common/config.h"

namespace qfw {
namespace {
int textWidth(QLabel* label) {
    // Measure using the actual QLabel font (including Windows fallback fonts).
    QLabel probe(label->parentWidget());
    probe.setTextFormat(Qt::PlainText);
    label->ensurePolished();
    probe.setFont(label->font());
    probe.setText(label->text());
    probe.ensurePolished();
    return probe.sizeHint().width();
}
}  // namespace

ProgressInfoBar::ProgressInfoBar(const QString& title, const QString& content,
                                 Qt::Orientation orient, bool isClosable, int duration,
                                 InfoBarPosition position, QWidget* parent)
    : InfoBar(InfoBarIconEnum::Information, title, content, orient, isClosable, duration, position,
              parent),
      description_(content) {
    auto* icon = new QWidget(this);
    icon->setFixedSize(36, 36);
    progressRing = new IndeterminateProgressRing(icon, false);
    determinateRing = new ProgressRing(icon, false);
    for (QWidget* ring :
         {static_cast<QWidget*>(progressRing), static_cast<QWidget*>(determinateRing)}) {
        ring->setFixedSize(20, 20);
        ring->move(8, 8);
        ring->setAttribute(Qt::WA_TransparentForMouseEvents);
    }
    progressRing->setStrokeWidth(3);
    determinateRing->setStrokeWidth(3);
    determinateRing->hide();
    replaceIconWidget(icon);
    textLayout_->setContentsMargins(1, 8, 0, 8);
    titleLabel_->setTextFormat(Qt::PlainText);
    contentLabel_->setTextFormat(Qt::PlainText);
    connect(determinateRing, &QProgressBar::valueChanged, this, [this](int value) {
        if (!settingValue_) emit valueChanged(value);
    });
    connect(&QConfig::instance(), &QConfig::themeColorChanged, this, [this] {
        progressRing->update();
        determinateRing->update();
    });
    connect(&QConfig::instance(), &QConfig::themeChangedFinished, this, [this] {
        progressRing->update();
        determinateRing->update();
        refreshText();
    });
    progressRing->start();
    refreshText();
}

void ProgressInfoBar::setTitle(const QString& title) { setText(title, content_); }
void ProgressInfoBar::setContent(const QString& content) {
    description_ = content;
    refreshText();
}
QString ProgressInfoBar::remainingTime() const { return remainingTime_; }
void ProgressInfoBar::setRemainingTime(const QString& text) {
    remainingTime_ = text;
    refreshText();
}
void ProgressInfoBar::refreshText() {
    QStringList parts;
    if (!description_.isEmpty()) parts.append(description_);
    if (!remainingTime_.isEmpty()) parts.append(remainingTime_);
    setText(title_, parts.join(QStringLiteral(" · ")));
}

void ProgressInfoBar::adjustText() {
    titleLabel_->setText(title_);
    contentLabel_->setText(content_);
    titleLabel_->setVisible(!title_.isEmpty());
    contentLabel_->setVisible(!content_.isEmpty());
    const int available = qMax(1, parentWidget() ? parentWidget()->width() - 50 : 900);
    widgetLayout_->activate();
    // 2px frame, 12px outer margins, 36px icon, 1px text inset, 12px close gap,
    // optional 36px close button; keep the inherited accessory layout.
    const int overhead = 63 + (isClosable_ ? 36 : 0) +
                         (orient_ == Qt::Horizontal ? widgetLayout_->sizeHint().width() : 0);
    const int budget = qMax(1, available - overhead);
    int titleWidth = textWidth(titleLabel_);
    int contentWidth = textWidth(contentLabel_);
    if (orient_ == Qt::Horizontal) {
        const int gap = title_.isEmpty() || content_.isEmpty() ? 0 : 17;
        if (titleWidth + contentWidth + gap > budget) {
            titleWidth = qMin(titleWidth, qMax(1, (budget - gap) * 2 / 5));
            contentWidth = qMin(contentWidth, qMax(1, budget - gap - titleWidth));
        }
    } else {
        const int width = qMin(qMax(titleWidth, contentWidth), budget);
        titleWidth = contentWidth = width;
    }
    titleLabel_->setWordWrap(true);
    contentLabel_->setWordWrap(true);
    titleLabel_->setFixedWidth(qMax(1, titleWidth));
    contentLabel_->setFixedWidth(qMax(1, contentWidth));
    textLayout_->invalidate();
    hBoxLayout_->invalidate();
    hBoxLayout_->activate();
    adjustSize();
}

int ProgressInfoBar::value() const { return determinateRing->value(); }
void ProgressInfoBar::setValue(int value) {
    // QProgressBar continues working after its signal. Relay our signal only
    // after it returns so a valueChanged callback may delete this task bar.
    setIndeterminate(false);
    const int previous = determinateRing->value();
    settingValue_ = true;
    determinateRing->setValue(qBound(0, value, 100));
    settingValue_ = false;
    const int current = determinateRing->value();
    if (current != previous) emit valueChanged(current);
}
bool ProgressInfoBar::isIndeterminate() const { return indeterminate_; }
void ProgressInfoBar::setIndeterminate(bool enabled) {
    if (indeterminate_ == enabled) return;
    indeterminate_ = enabled;
    progressRing->setVisible(enabled);
    determinateRing->setVisible(!enabled);
    if (enabled && isEnabled())
        progressRing->start();
    else
        progressRing->stop();
}
void ProgressInfoBar::setCustomBarColor(const QColor& light, const QColor& dark) {
    progressRing->setCustomBarColor(light, dark);
    determinateRing->setCustomBarColor(light, dark);
}
void ProgressInfoBar::addWidget(QWidget* widget, int stretch) {
    if (!widget || widget == this || widget == iconWidget_ || widget == titleLabel_ ||
        widget == contentLabel_ || widget == closeButton_ || widget == progressRing ||
        widget == determinateRing)
        return;
    InfoBar::addWidget(widget, stretch);
    refreshText();
}
void ProgressInfoBar::closeEvent(QCloseEvent* event) {
    if (closing_) {
        event->ignore();
        return;
    }
    closing_ = true;
    progressRing->stop();
    hide();
    InfoBar::closeEvent(event);
}
void ProgressInfoBar::hideEvent(QHideEvent* event) {
    if (position_ != InfoBarPosition::None) InfoBarManager::make(position_)->remove(this);
    InfoBar::hideEvent(event);
}
void ProgressInfoBar::showEvent(QShowEvent* event) { InfoBar::showEvent(event); }
void ProgressInfoBar::changeEvent(QEvent* event) {
    InfoBar::changeEvent(event);
    if (event->type() == QEvent::EnabledChange) {
        if (isEnabled() && indeterminate_)
            progressRing->start();
        else
            progressRing->stop();
    } else if (event->type() == QEvent::FontChange ||
               event->type() == QEvent::LayoutDirectionChange) {
        refreshText();
    }
}
bool ProgressInfoBar::eventFilter(QObject* obj, QEvent* event) {
    const bool result = InfoBar::eventFilter(obj, event);
    if (obj == parent() &&
        (event->type() == QEvent::Resize || event->type() == QEvent::WindowStateChange) &&
        isVisible() && position_ != InfoBarPosition::None)
        InfoBarManager::make(position_)->refresh(this);
    return result;
}
ProgressInfoBar* ProgressInfoBar::create(const QString& title, const QString& content,
                                         Qt::Orientation orient, bool isClosable, int duration,
                                         InfoBarPosition position, QWidget* parent) {
    auto* bar = new ProgressInfoBar(title, content, orient, isClosable, duration, position, parent);
    bar->show();
    return bar;
}
ProgressInfoBar* ProgressInfoBar::info(const QString& title, const QString& content,
                                       Qt::Orientation orient, bool isClosable, int duration,
                                       InfoBarPosition position, QWidget* parent) {
    return create(title, content, orient, isClosable, duration, position, parent);
}

}  // namespace qfw
