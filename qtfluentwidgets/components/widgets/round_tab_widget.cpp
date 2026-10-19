#include "round_tab_widget.h"

#include <QHash>
#include <QScrollBar>
#include <QShortcut>
#include <QTransform>
#include <stdexcept>

#include "common/config.h"

namespace qfw {
namespace {
class WidgetTabItem : public RoundTabItem {
public:
    WidgetTabItem(FluentIconEnum icon, const QString& text, QWidget* parent)
        : RoundTabItem(icon, text, parent) {}

protected:
    void drawRoundSelectedBackground(QPainter*) override {}
};
class WidgetTabBar : public RoundTabBar {
protected:
    TabItem* createTabItem(const QString& text, FluentIconEnum icon) override {
        return new WidgetTabItem(icon, text, view_);
    }
};
class RoundTabStack : public QStackedWidget {
protected:
    void paintEvent(QPaintEvent*) override {}
};
}  // namespace

class RoundTabWidget::PageChange {
public:
    explicit PageChange(RoundTabWidget* owner, bool notifyIndex = true)
        : owner_(owner),
          index_(owner->currentIndex()),
          page_(owner->currentWidget()),
          notify_(notifyIndex) {
        ++owner_->changeDepth_;
    }
    ~PageChange() { owner_->finishChange(index_, page_, notify_); }

private:
    RoundTabWidget* owner_;
    int index_;
    QWidget* page_;
    bool notify_;
};

RoundTabWidget::RoundTabWidget(QWidget* parent)
    : TabWidget(new WidgetTabBar, new RoundTabStack, parent) {
    vBoxLayout_->setSpacing(0);
    stackedWidget_->setAutoFillBackground(false);
    stackedWidget_->setFrameShape(QFrame::NoFrame);
    connect(stackedWidget_, &QStackedWidget::currentChanged, this, [this](int index) {
        if (changeDepth_) return;
        syncBar();
        stackedWidget_->update();
        update();
        emit currentChanged(index);
    });
    connect(&QConfig::instance(), &QConfig::themeChangedFinished, this, [this]() {
        stackedWidget_->update();
        update();
    });
    connect(tabBar_->horizontalScrollBar(), &QScrollBar::valueChanged, this, [this]() {
        stackedWidget_->update();
        update();
    });
    connect(tabBar_, &TabBar::tabSelectedBackgroundColorChanged, this, [this]() {
        stackedWidget_->update();
        update();
    });
    for (int step : {1, -1}) {
        auto* shortcut =
            new QShortcut(QKeySequence(step == 1 ? "Ctrl+Tab" : "Ctrl+Shift+Tab"), this);
        shortcut->setContext(Qt::WidgetWithChildrenShortcut);
        connect(shortcut, &QShortcut::activated, this, [this, step]() { cycleTab(step); });
    }
}
void RoundTabWidget::finishChange(int oldIndex, QWidget* oldPage, bool notifyIndex) {
    if (--changeDepth_) return;
    syncBar();
    updateAutoHide();
    stackedWidget_->update();
    update();
    if ((notifyIndex && oldIndex != currentIndex()) || oldPage != currentWidget())
        emit currentChanged(currentIndex());
}
void RoundTabWidget::syncBar() { tabBar_->setCurrentIndex(currentIndex()); }
int RoundTabWidget::indexOf(QWidget* page) const {
    return page ? stackedWidget_->indexOf(page) : -1;
}
int RoundTabWidget::addTab(QWidget* page, const QString& label, const QString& routeKey) {
    return insertTab(-1, page, label, routeKey);
}
int RoundTabWidget::addTab(QWidget* page, const QString& label, FluentIconEnum icon,
                           const QString& routeKey) {
    return insertTab(-1, page, label, icon, routeKey);
}
int RoundTabWidget::addTab(QWidget* page, FluentIconEnum icon, const QString& label) {
    return addTab(page, label, icon);
}
int RoundTabWidget::insertTab(int index, QWidget* page, const QString& label,
                              const QString& routeKey) {
    return insertPage(index, page, label, routeKey, nullptr);
}
int RoundTabWidget::insertTab(int index, QWidget* page, const QString& label, FluentIconEnum icon,
                              const QString& routeKey) {
    return insertPage(index, page, label, routeKey, &icon);
}
int RoundTabWidget::insertTab(int index, QWidget* page, FluentIconEnum icon, const QString& label) {
    return insertTab(index, page, label, icon);
}
int RoundTabWidget::insertPage(int index, QWidget* page, const QString& label,
                               const QString& routeKey, const FluentIconEnum* icon) {
    if (!page || page == this || page->isAncestorOf(this) || indexOf(page) >= 0) return -1;
    if (!routeKey.isEmpty() && tabBar_->tab(routeKey))
        throw std::invalid_argument("The tab route key is duplicated");
    auto* bar = qobject_cast<RoundTabBar*>(tabBar_);
    if (!bar) throw std::invalid_argument("RoundTabWidget requires a RoundTabBar");
    if (index < 0 || index > count()) index = count();
    const QString key =
        routeKey.isEmpty() ? QUuid::createUuid().toString(QUuid::WithoutBraces) : routeKey;
    PageChange change(this, false);
    auto* item =
        icon ? bar->insertTab(index, key, label, *icon) : bar->insertTab(index, key, label);
    if (!item) return -1;
    page->setProperty("routeKey", key);
    stackedWidget_->insertWidget(index, page);
    item->installEventFilter(this);
    return index;
}
void RoundTabWidget::setCurrentIndex(int index) {
    if (!isIndexValid(index, count())) return;
    PageChange change(this);
    TabWidget::setCurrentIndex(index);
}
void RoundTabWidget::onCurrentTabChanged(int index) {
    if (!changeDepth_) setCurrentIndex(index);
}
void RoundTabWidget::onTabMoved(int fromIndex, int toIndex) {
    if (!isIndexValid(fromIndex, count()) || !isIndexValid(toIndex, count())) return;
    PageChange change(this);
    QWidget* current = currentWidget();
    QHash<QString, QWidget*> pages;
    for (int i = 0; i < count(); ++i)
        pages.insert(widget(i)->property("routeKey").toString(), widget(i));
    QList<QWidget*> ordered;
    for (auto* item : tabBar_->items()) {
        auto* page = pages.value(item->routeKey(), nullptr);
        if (!page) return;
        ordered.append(page);
    }
    if (ordered.size() != count()) return;
    // Match actual bar routes, including swaps across hidden tabs; preserve current page identity.
    for (auto* page : ordered) stackedWidget_->removeWidget(page);
    for (auto* page : ordered) stackedWidget_->addWidget(page);
    if (current) stackedWidget_->setCurrentWidget(current);
}
void RoundTabWidget::removeTab(int index) {
    if (!isIndexValid(index, count())) return;
    PageChange change(this);
    QWidget* page = widget(index);
    QWidget* target = currentWidget();
    if (target == page) {
        QList<int> candidates;
        for (int i = index + 1; i < count(); ++i) candidates.append(i);
        for (int i = index - 1; i >= 0; --i) candidates.append(i);
        target = candidates.isEmpty() ? nullptr : widget(candidates.first());
        for (int i : candidates)
            if (isTabVisible(i) && isTabEnabled(i)) {
                target = widget(i);
                break;
            }
    }
    stackedWidget_->removeWidget(page);
    tabBar_->removeTab(index);
    page->hide();
    if (target) stackedWidget_->setCurrentWidget(target);
}
void RoundTabWidget::clear() {
    while (count()) removeTab(count() - 1);
}
void RoundTabWidget::selectAvailable(int index, bool allowDisabled) {
    QList<int> candidates;
    for (int i = index + 1; i < count(); ++i) candidates.append(i);
    for (int i = index - 1; i >= 0; --i) candidates.append(i);
    for (int i : candidates)
        if (isTabVisible(i) && isTabEnabled(i)) {
            setCurrentIndex(i);
            return;
        }
    if (allowDisabled)
        for (int i : candidates)
            if (isTabVisible(i)) {
                setCurrentIndex(i);
                return;
            }
}
void RoundTabWidget::setTabVisible(int index, bool visible) {
    auto* item = tabBar_->tabItem(index);
    if (!item || visible == !item->isHidden()) return;
    PageChange change(this);
    item->setVisible(visible);
    if (!visible && index == currentIndex())
        selectAvailable(index, true);
    else if (visible && !isTabVisible(currentIndex()))
        setCurrentIndex(index);
}
void RoundTabWidget::setTabEnabled(int index, bool enabled) {
    if (!isIndexValid(index, count())) return;
    PageChange change(this);
    TabWidget::setTabEnabled(index, enabled);
    widget(index)->setEnabled(enabled);
    if (!enabled && index == currentIndex()) selectAvailable(index);
}
void RoundTabWidget::cycleTab(int step) {
    for (int offset = 1; offset <= count(); ++offset) {
        const int index = (currentIndex() + offset * step % count() + count()) % count();
        if (isTabVisible(index) && isTabEnabled(index)) {
            setCurrentIndex(index);
            return;
        }
    }
}
void RoundTabWidget::setTabSelectedBackgroundColor(const QColor& light, const QColor& dark) {
    TabWidget::setTabSelectedBackgroundColor(light, dark);
    stackedWidget_->update();
    update();
}
void RoundTabWidget::setTabBarAutoHide(bool enabled) {
    autoHide_ = enabled;
    updateAutoHide();
}
void RoundTabWidget::updateAutoHide() { tabBar_->setVisible(!autoHide_ || count() >= 2); }
void RoundTabWidget::setTabWhatsThis(int index, const QString& text) {
    if (auto* item = tabBar_->tabItem(index)) item->setWhatsThis(text);
}
QString RoundTabWidget::tabWhatsThis(int index) const {
    auto* item = tabBar_->tabItem(index);
    return item ? item->whatsThis() : QString();
}
bool RoundTabWidget::eventFilter(QObject* watched, QEvent* event) {
    if (event->type() == QEvent::Move || event->type() == QEvent::Resize ||
        event->type() == QEvent::Show || event->type() == QEvent::Hide) {
        stackedWidget_->update();
        update();
    }
    return TabWidget::eventFilter(watched, event);
}
QPainterPath RoundTabWidget::framePath(bool outline) const {
    const qreal edge = outline ? .5 : 0;
    const QRectF content = QRectF(stackedWidget_->geometry()).adjusted(edge, edge, -edge, -edge);
    QPainterPath path;
    path.addRoundedRect(content, 8, 8);
    auto* item = qobject_cast<RoundTabItem*>(tabBar_->currentTab());
    if (!item || item->isHidden() || tabBar_->isHidden()) return path;
    const QPoint position = item->mapTo(this, QPoint());
    const qreal baseline = content.top() + .001;
    auto tab = item->selectedPath(outline, baseline - position.y());
    tab.closeSubpath();
    tab = QTransform::fromTranslate(position.x(), position.y()).map(tab);
    auto* viewport = tabBar_->viewport();
    QRectF viewportRect(viewport->rect());
    viewportRect.moveTopLeft(viewport->mapTo(this, QPoint()));
    viewportRect.setBottom(baseline);
    QPainterPath clip;
    clip.addRect(viewportRect);
    return path.united(tab.intersected(clip));
}
void RoundTabWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(isDarkTheme() ? tabBar_->darkSelectedBackgroundColor()
                                   : tabBar_->lightSelectedBackgroundColor());
    painter.drawPath(framePath());
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(isDarkTheme() ? QColor(255, 255, 255, 18) : QColor(0, 0, 0, 15), 1));
    painter.drawPath(framePath(true));
}
}  // namespace qfw
