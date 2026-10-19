#include "window/filled_fluent_window.h"

#include <QApplication>
#include <QGraphicsDropShadowEffect>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QScreen>
#include <QRegularExpression>
#include <algorithm>
#include "common/config.h"
#include "common/font.h"
#include "common/icon.h"
#include "common/style_sheet.h"
#include "components/navigation/navigation_interface.h"
#include "components/widgets/label.h"
#include "components/widgets/line_edit.h"
#include "components/widgets/list_view.h"
#include "window/stacked_widget.h"

namespace qfw {
namespace {
class FilledTitleBar : public FluentTitleBar {
public:
    explicit FilledTitleBar(QWidget* parent) : FluentTitleBar(parent) {
        hBoxLayout()->setContentsMargins(24, 0, 0, 0);
        hBoxLayout()->setSpacing(0);
        // FluentTitleBar's legacy spacer is absent in the Python filled title bar.
        delete hBoxLayout()->takeAt(1);
    }
};
class PageSearchDelegate : public ListItemDelegate {
public:
    explicit PageSearchDelegate(QListView* view) : ListItemDelegate(view) {}
protected:
    void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override {
        ListItemDelegate::initStyleOption(option, index);
        const QIcon original = option->icon;
        for (auto state : {QIcon::Off, QIcon::On})
            option->icon.addPixmap(original.pixmap(option->decorationSize, QIcon::Normal, state),
                                   QIcon::Selected, state);
    }
    void drawIndicator(QPainter* painter, const QStyleOptionViewItem& option,
                       const QModelIndex&) const override {
        painter->setPen(Qt::NoPen);
        painter->setBrush(themeColor());
        const int h = qMin(16, option.rect.height());
        painter->drawRoundedRect(QRectF(option.rect.x() + 1,
            option.rect.center().y() - h / 2.0, 3, h), 1.5, 1.5);
    }
};
class PageSearchList : public ListWidget {
public:
    explicit PageSearchList(QWidget* parent) : ListWidget(parent) {
        auto* previous = delegate_;
        delegate_ = new PageSearchDelegate(this);
        QListWidget::setItemDelegate(delegate_);
        previous->deleteLater();
    }
};
}

FilledFluentWindow::FilledFluentWindow(QWidget* parent) : FluentWindow(parent) {
    auto* bar = new FilledTitleBar(this);
    setTitleBar(bar);
    hBoxLayout_->setContentsMargins(0, bar->height(), 0, 0);
    widgetLayout_->setContentsMargins(0, 0, 0, 0);
    auto* panel = navigationInterface_->panel();
    panel->setFilledSelectionEnabled(true);
    navigationInterface_->setMenuButtonVisible(false);
    navigationInterface_->setReturnButtonVisible(false);
    navigationInterface_->setCollapsible(false);
    searchLineEdit_ = new SearchLineEdit(panel);
    searchLineEdit_->setPlaceholderText(tr("Search"));
    searchLineEdit_->setFixedHeight(32);
    panel->topLayout()->insertWidget(0, searchLineEdit_);
    panel->topLayout()->insertSpacing(1, 8);
    connect(navigationInterface_, &NavigationInterface::displayModeChanged, this,
        [this](NavigationDisplayMode mode) {
            searchLineEdit_->setVisible(mode == NavigationDisplayMode::Expand ||
                                       mode == NavigationDisplayMode::Menu);
        });
    searchPopup_ = new QFrame(this, Qt::Tool | Qt::FramelessWindowHint |
                                     Qt::WindowDoesNotAcceptFocus);
    searchPopup_->setObjectName("pageSearchPopup");
    searchPopup_->setAttribute(Qt::WA_TranslucentBackground);
    searchPopup_->setAttribute(Qt::WA_ShowWithoutActivating);
    searchPopup_->setFocusPolicy(Qt::NoFocus);
    auto* card = new QFrame(searchPopup_);
    card->setObjectName("pageSearchCard");
    auto* shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(18);
    shadow->setOffset(0, 3);
    shadow->setColor(QColor(0, 0, 0, 45));
    card->setGraphicsEffect(shadow);
    searchResults_ = new PageSearchList(card);
    searchResults_->setObjectName("pageSearchResults");
    searchResults_->setFocusPolicy(Qt::NoFocus);
    searchResults_->setIconSize(QSize(16, 16));
    searchResults_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    searchResults_->setSpacing(2);
    qfw::setFont(searchResults_, 14);
    emptyLabel_ = new BodyLabel(tr("No matching pages"), card);
    emptyLabel_->setObjectName("pageSearchEmptyLabel");
    emptyLabel_->setAlignment(Qt::AlignCenter);
    emptyLabel_->setFixedHeight(44);
    auto* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(8, 8, 8, 8);
    cardLayout->addWidget(searchResults_);
    cardLayout->addWidget(emptyLabel_);
    auto* outerLayout = new QVBoxLayout(searchPopup_);
    outerLayout->setContentsMargins(10, 8, 10, 14);
    outerLayout->addWidget(card);
    searchLineEdit_->installEventFilter(this);
    qApp->installEventFilter(this);
    connect(searchLineEdit_, &QLineEdit::textChanged, this, &FilledFluentWindow::updatePageSearch);
    connect(searchLineEdit_, &SearchLineEdit::searchSignal, this, [this]() {
        searchLineEdit_->setFocus(); updatePageSearch();
        if (!searchLineEdit_->text().trimmed().isEmpty()) searchPopup_->show();
    });
    connect(searchLineEdit_, &SearchLineEdit::clearSignal, searchPopup_, &QWidget::hide);
    connect(searchResults_, &QListWidget::itemClicked, this, [this]() { activateSearchResult(); });
    connect(&QConfig::instance(), &QConfig::themeChangedFinished, this, [this]() {
        refreshSearchTheme();
        if (searchPopup_->isVisible()) updatePageSearch();
    });
    connect(&QConfig::instance(), &QConfig::themeChanged, panel, [panel]() { panel->update(); });
    connect(&QConfig::instance(), &QConfig::themeColorChanged, panel, [panel]() { panel->update(); });
    refreshSearchTheme();
    layoutTitleBar();
}

void FilledFluentWindow::refreshSearchTheme() {
    searchPopup_->setStyleSheet(QStringLiteral(
        "#pageSearchPopup{background:transparent;border:none;}"
        "#pageSearchCard{background:%1;border:1px solid %2;border-radius:8px;}"
        "#pageSearchEmptyLabel{color:%3;background:transparent;}")
        .arg(isDarkTheme() ? "rgb(40,42,50)" : "rgb(249,249,249)",
             isDarkTheme() ? "rgba(255,255,255,0.08)" : "rgba(0,0,0,0.12)",
             isDarkTheme() ? "rgb(180,180,180)" : "rgb(96,96,96)"));
}

void FilledFluentWindow::updatePageSearch() {
    const QString query = searchLineEdit_->text().trimmed();
    if (query.isEmpty()) { searchQuery_.clear(); searchPopup_->hide(); return; }
    const auto tokens = query.toCaseFolded().split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    struct Result { int score; QString key, title; QVariant icon; };
    QList<Result> results;
    const QString oldRoute = query == searchQuery_ && searchResults_->currentItem()
        ? searchResults_->currentItem()->data(Qt::UserRole).toString() : QString();
    for (int i = 0; i < stackedWidget_->count(); ++i) {
        const QString key = stackedWidget_->widget(i)->objectName();
        auto* item = qobject_cast<NavigationTreeWidget*>(navigationInterface_->widget(key));
        if (!item || !item->isSelectable() || !item->isEnabled()) continue;
        const QString folded = item->text().toCaseFolded();
        if (!std::all_of(tokens.cbegin(), tokens.cend(), [&folded](const QString& token) {
            return folded.contains(token); })) continue;
        const QString joined = tokens.join(' ');
        results.append({folded == joined ? 0 : folded.startsWith(joined) ? 1 : 2,
                        key, item->text(), item->iconVariant()});
    }
    std::stable_sort(results.begin(), results.end(), [](const Result& a, const Result& b) {
        return a.score < b.score;
    });
    searchResults_->clear();
    int selected = 0;
    for (const auto& result : results) {
        QIcon icon = result.icon.value<QIcon>();
        if (result.icon.canConvert<const FluentIconBase*>()) {
            if (auto* fluent = result.icon.value<const FluentIconBase*>()) icon = fluent->qicon();
        } else if (result.icon.type() == QVariant::String) icon = QIcon(result.icon.toString());
        auto* row = new QListWidgetItem(icon, result.title);
        row->setData(Qt::UserRole, result.key);
        row->setToolTip(result.title);
        row->setSizeHint(QSize(1, 36));
        searchResults_->addItem(row);
        if (result.key == oldRoute) selected = searchResults_->count() - 1;
    }
    searchResults_->setVisible(!results.isEmpty());
    emptyLabel_->setVisible(results.isEmpty());
    if (!results.isEmpty()) searchResults_->setCurrentRow(selected, QItemSelectionModel::ClearAndSelect);
    searchQuery_ = query;
    repositionSearch();
    if (searchLineEdit_->hasFocus() || searchPopup_->isVisible()) searchPopup_->show();
}

void FilledFluentWindow::activateSearchResult() {
    auto* result = searchResults_->currentItem();
    if (!result) return;
    const QString key = result->data(Qt::UserRole).toString();
    auto* item = navigationInterface_->widget(key);
    if (!item || !item->isEnabled()) { updatePageSearch(); return; }
    for (int i = 0; i < stackedWidget_->count(); ++i) {
        auto* page = stackedWidget_->widget(i);
        if (page->objectName() != key) continue;
        searchPopup_->hide();
        switchTo(page);
        page->setFocus(Qt::OtherFocusReason);
        emit pageSearchActivated(page);
        return;
    }
    updatePageSearch();
}

void FilledFluentWindow::repositionSearch() {
    const QRect screenRect = searchLineEdit_->screen()->availableGeometry();
    const int w = qMin(searchLineEdit_->width() + 20, screenRect.width() - 16);
    searchResults_->ensurePolished();
    const int rowHeight = searchResults_->count() ? searchResults_->sizeHintForRow(0) : 44;
    int h = qMin(8, qMax(1, searchResults_->count())) * (rowHeight + 4) + 40;
    const QPoint below = searchLineEdit_->mapToGlobal(QPoint(-10, searchLineEdit_->height() - 3));
    const QPoint above = searchLineEdit_->mapToGlobal(QPoint(-10, 0));
    const int available = qMin(screenRect.bottom() - 8, frameGeometry().bottom() - 8) - below.y();
    int y = below.y();
    if (available < qMin(h, 100) && above.y() - screenRect.top() > available) {
        h = qMin(h, above.y() - screenRect.top() - 8); y = above.y() - h;
    } else h = qMin(h, qMax(60, available));
    const int x = qMax(screenRect.left() + 8, qMin(below.x(), screenRect.right() - w - 8));
    searchPopup_->setFixedSize(qMax(120, w), qMax(60, h));
    searchPopup_->move(x, y);
}

bool FilledFluentWindow::eventFilter(QObject* object, QEvent* event) {
    if (searchPopup_ && searchPopup_->isVisible()) {
        if (event->type() == QEvent::MouseButtonPress) {
            auto* mouse = static_cast<QMouseEvent*>(event);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            const QPoint pos = mouse->globalPosition().toPoint();
#else
            const QPoint pos = mouse->globalPos();
#endif
            if (!searchPopup_->frameGeometry().contains(pos) &&
                !QRect(searchLineEdit_->mapToGlobal(QPoint()), searchLineEdit_->size()).contains(pos))
                searchPopup_->hide();
        } else if (object == this) {
            if (event->type() == QEvent::Move || event->type() == QEvent::Resize) repositionSearch();
            else if (event->type() == QEvent::Hide || event->type() == QEvent::WindowDeactivate)
                searchPopup_->hide();
        }
    }
    if (object == searchLineEdit_) {
        if (event->type() == QEvent::FocusIn && !searchLineEdit_->text().trimmed().isEmpty())
            updatePageSearch();
        else if (event->type() == QEvent::KeyPress) {
            const int key = static_cast<QKeyEvent*>(event)->key();
            if ((key == Qt::Key_Up || key == Qt::Key_Down || key == Qt::Key_Return || key == Qt::Key_Enter)
                && !searchLineEdit_->text().trimmed().isEmpty()) {
                if (!searchPopup_->isVisible()) {
                    searchLineEdit_->setFocus();
                    updatePageSearch();
                    searchPopup_->show();
                }
                if (key == Qt::Key_Return || key == Qt::Key_Enter) activateSearchResult();
                else if (searchResults_->count()) {
                    const int row = qBound(0, searchResults_->currentRow() + (key == Qt::Key_Down ? 1 : -1),
                                           searchResults_->count() - 1);
                    searchResults_->setCurrentRow(row, QItemSelectionModel::ClearAndSelect);
                    searchResults_->scrollToItem(searchResults_->currentItem());
                }
                return true;
            }
            if (key == Qt::Key_Escape && searchPopup_->isVisible()) { searchPopup_->hide(); return true; }
            if (key == Qt::Key_Tab || key == Qt::Key_Backtab) searchPopup_->hide();
        }
    }
    return FluentWindow::eventFilter(object, event);
}

NavigationWidget* FilledFluentWindow::addSubInterface(QWidget* page, const QVariant& icon,
    const QString& text, NavigationItemPosition position, QWidget* parent, bool isTransparent) {
    auto* item = FluentWindow::addSubInterface(page, icon, text, position, parent, isTransparent);
    if (searchPopup_ && searchPopup_->isVisible()) updatePageSearch();
    return item;
}
void FilledFluentWindow::removeInterface(QWidget* page, bool isDelete) {
    FluentWindow::removeInterface(page, isDelete);
    if (searchPopup_->isVisible()) updatePageSearch();
}
void FilledFluentWindow::layoutTitleBar() {
    titleBar()->move(0, 0);
    titleBar()->resize(width(), titleBar()->height());
    titleBar()->raise();
}
void FilledFluentWindow::resizeEvent(QResizeEvent* event) {
    FluentWindow::resizeEvent(event); layoutTitleBar();
}
void FilledFluentWindow::showEvent(QShowEvent* event) {
    FluentWindow::showEvent(event); layoutTitleBar();
}
} // namespace qfw
