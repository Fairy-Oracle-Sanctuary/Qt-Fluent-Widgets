#include "components/widgets/round_list_view.h"

#include <QApplication>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QStyle>
#include <QSvgRenderer>
#include <memory>

#include "common/color.h"
#include "common/style_sheet.h"

namespace qfw {
namespace {
class RoundListIconEngine final : public QIconEngine {
public:
    explicit RoundListIconEngine(const FluentIconBase& icon) : icon_(icon.clone()) {}
    QIconEngine* clone() const override { return new RoundListIconEngine(*icon_); }
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QString iconName() override { return QStringLiteral("qfw.round-list.fluent"); }
#else
    QString iconName() const override { return QStringLiteral("qfw.round-list.fluent"); }
#endif
    void paint(QPainter* painter, const QRect& rect, QIcon::Mode mode, QIcon::State) override {
        painter->save();
        if (mode == QIcon::Disabled) painter->setOpacity(painter->opacity() * .43);
        QVariantMap attributes;
        if (mode == QIcon::Selected) attributes.insert("fill", themeColor().name());
        if (dynamic_cast<const FluentIcon*>(icon_.get())) {
            // Render vectors at the paint device's actual scale. The shared icon
            // pixmap cache can reuse a 1x image for an offscreen 2x paint device.
            QSvgRenderer renderer(writeSvg(icon_->path(), {}, attributes).toUtf8());
            renderer.render(painter, QRectF(rect));
        } else {
            icon_->render(painter, rect, Theme::Auto, attributes);
        }
        painter->restore();
    }
    QPixmap pixmap(const QSize& size, QIcon::Mode mode, QIcon::State state) override {
        QPixmap image(size);
        image.fill(Qt::transparent);
        QPainter painter(&image);
        paint(&painter, QRect(QPoint(), size), mode, state);
        return image;
    }

private:
    std::unique_ptr<FluentIconBase> icon_;
};

void initializeRoundList(QListView* view) {
    view->setProperty("roundList", true);
    view->setSpacing(1);
    view->setIconSize(QSize(16, 16));
    // Geometry and text/check-indicator layout remain Qt-native. The delegate paints
    // the card itself so stylesheet selection cannot cover the theme-colored border.
    const QString qss = QStringLiteral(
        "QListView[roundList=\"true\"]::item { background: transparent; border: none;"
        " padding-left: 16px; padding-right: 11px; height: 42px; }"
        "QListView[roundList=\"true\"]::item:selected,"
        "QListView[roundList=\"true\"]::item:hover { background: transparent; border: none; }");
    setCustomStyleSheet(view, qss, qss);
    auto repaint = [view] { view->viewport()->update(); };
    QObject::connect(&QConfig::instance(), &QConfig::themeChanged, view, repaint);
    QObject::connect(&QConfig::instance(), &QConfig::themeColorChanged, view, repaint);
}
}  // namespace

RoundListItemDelegate::RoundListItemDelegate(QListView* parent) : ListItemDelegate(parent) {}
bool RoundListItemDelegate::transparentSelection() const { return transparentSelection_; }
void RoundListItemDelegate::setTransparentSelection(bool transparent) {
    if (transparentSelection_ == transparent) return;
    transparentSelection_ = transparent;
    refresh();
}
bool RoundListItemDelegate::keepNormalBackground() const { return keepNormalBackground_; }
void RoundListItemDelegate::setKeepNormalBackground(bool keep) {
    if (keepNormalBackground_ == keep) return;
    keepNormalBackground_ = keep;
    refresh();
}
void RoundListItemDelegate::refresh() {
    if (auto* view = qobject_cast<QListView*>(parent())) {
        view->doItemsLayout();
        view->viewport()->update();
    }
}
QIcon RoundListItemDelegate::themedIcon(const FluentIconBase& icon) {
    return QIcon(new RoundListIconEngine(icon));
}
void RoundListItemDelegate::initStyleOption(QStyleOptionViewItem* option,
                                            const QModelIndex& index) const {
    TableItemDelegate::initStyleOption(option, index);
    option->fontMetrics = QFontMetrics(option->font);
}
QSize RoundListItemDelegate::sizeHint(const QStyleOptionViewItem& option,
                                      const QModelIndex& index) const {
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    const auto explicitSize = index.data(Qt::SizeHintRole);
    if (explicitSize.isValid()) return explicitSize.toSize();
    auto size = QStyledItemDelegate::sizeHint(opt, index);
    // Measured in the Python reference, including its 4px delegate margin.
    size.setHeight(qMax(transparentSelection_ ? 46 : 48, opt.fontMetrics.height() + 12));
    return size;
}
void RoundListItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                                  const QModelIndex& index) const {
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    const bool selected = option.state.testFlag(QStyle::State_Selected);
    const bool hover = option.state.testFlag(QStyle::State_MouseOver);
    const bool dark = isDarkTheme();
    const int c = dark ? 255 : 0;
    painter->save();
    painter->setClipRect(option.rect);
    painter->setRenderHint(QPainter::Antialiasing);
    const QRectF card = QRectF(option.rect).adjusted(.5, .5, -.5, -.5);
    painter->setPen(Qt::NoPen);
    // Python first draws its delegate background, then the stylesheet card.
    if (!(transparentSelection_ && selected && !hover)) {
        const int alpha = selected && !keepNormalBackground_ ? 17 : (hover ? 12 : 0);
        painter->setBrush(opt.backgroundBrush.style() != Qt::NoBrush
                              ? opt.backgroundBrush
                              : QBrush(QColor(c, c, c, alpha)));
        painter->drawRoundedRect(card.adjusted(.5, .5, -.5, -.5), 5, 5);
    }
    // In Python's stylesheet :selected follows :hover and takes precedence.
    const int fillAlpha = selected                ? (transparentSelection_   ? 0
                                                     : keepNormalBackground_ ? (dark ? 25 : 3)
                                                                             : (dark ? 21 : 5))
                          : hover                 ? (dark ? 21 : 3)
                          : transparentSelection_ ? 0
                                                  : (dark ? 25 : 3);
    QColor border(
        c, c, c,
        transparentSelection_ && !hover ? 0 : (hover ? (dark ? 20 : 31) : (dark ? 14 : 20)));
    if (selected) border = autoFallbackThemeColor(lightCheckedColor_, darkCheckedColor_);
    painter->setPen(QPen(border, 1));
    painter->setBrush(QColor(c, c, c, fillAlpha));
    painter->drawRoundedRect(card, 5, 5);
    painter->restore();

    // Use the actual view selection state, not cached row numbers: sorting,
    // filtering, model replacement and external selection all stay correct.
    opt.backgroundBrush = Qt::NoBrush;
    opt.state &= ~QStyle::State_HasFocus;
    const QWidget* widget = option.widget;
    auto* style = widget ? widget->style() : QApplication::style();
    const auto icon = opt.icon;
    auto iconRect = style->subElementRect(QStyle::SE_ItemViewItemDecoration, &opt, widget);
    if (!icon.isNull() && !opt.features.testFlag(QStyleOptionViewItem::HasCheckIndicator)) {
        const bool rtl = opt.direction == Qt::RightToLeft;
        // Python reserves 30px for the decoration and draws a 16px icon 16px
        // from the leading edge. Qt's default spacing is noticeably tighter.
        iconRect = QRect(rtl ? option.rect.right() - 31 : option.rect.left() + 16,
                         option.rect.top() + (option.rect.height() - 16) / 2, 16, 16);
        opt.rect.adjust(rtl ? 0 : 10, 0, rtl ? -10 : 0, 0);
    }
    // Retain HasDecoration and its size so Qt reserves the text/check-box space,
    // but paint the icon ourselves. Qt otherwise recolors ordinary selected QIcons
    // and may rasterize FluentIcons at the wrong scale.
    opt.icon = QIcon();
    style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);
    if (!icon.isNull()) {
        const auto mode = !opt.state.testFlag(QStyle::State_Enabled) ? QIcon::Disabled
                          : selected && icon.name() == QStringLiteral("qfw.round-list.fluent")
                              ? QIcon::Selected
                              : QIcon::Normal;
        icon.paint(painter, iconRect, Qt::AlignCenter, mode);
    }
}

RoundListWidget::RoundListWidget(QWidget* parent) : ListWidget(parent) {
    auto* old = delegate_;
    delegate_ = new RoundListItemDelegate(this);
    setItemDelegate(delegate_);
    delete old;
    initializeRoundList(this);
}
RoundListItemDelegate* RoundListWidget::roundDelegate() const {
    return qobject_cast<RoundListItemDelegate*>(itemDelegate());
}
void RoundListWidget::addItem(const QString& text, const QIcon& icon) {
    QListWidget::addItem(new QListWidgetItem(icon, text));
}
void RoundListWidget::setCurrentRow(int row) { QListWidget::setCurrentRow(row); }
void RoundListWidget::setCurrentRow(int row, QItemSelectionModel::SelectionFlags command) {
    QListWidget::setCurrentRow(row, command);
}
void RoundListWidget::setCurrentItem(QListWidgetItem* item) { QListWidget::setCurrentItem(item); }
void RoundListWidget::setCurrentItem(QListWidgetItem* item,
                                     QItemSelectionModel::SelectionFlags command) {
    QListWidget::setCurrentItem(item, command);
}
void RoundListWidget::addItem(const QString& text, const FluentIconBase& icon) {
    addItem(text, RoundListItemDelegate::themedIcon(icon));
}
void RoundListWidget::addItem(const QString& text, FluentIconEnum icon) {
    addItem(text, FluentIcon(icon));
}
void RoundListWidget::setCheckedColor(const QColor& light, const QColor& dark) {
    ListWidget::setCheckedColor(light, dark);
    viewport()->update();
}
void RoundListWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::RightButton && !isSelectRightClickedRow()) {
        event->ignore();
        return;
    }
    ListWidget::mousePressEvent(event);
}
void RoundListWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::RightButton && !isSelectRightClickedRow()) {
        event->ignore();
        return;
    }
    ListWidget::mouseReleaseEvent(event);
}
RoundListView::RoundListView(QWidget* parent) : ListView(parent) {
    auto* old = delegate_;
    delegate_ = new RoundListItemDelegate(this);
    setItemDelegate(delegate_);
    delete old;
    initializeRoundList(this);
}
RoundListItemDelegate* RoundListView::roundDelegate() const {
    return qobject_cast<RoundListItemDelegate*>(itemDelegate());
}
void RoundListView::setCheckedColor(const QColor& light, const QColor& dark) {
    ListView::setCheckedColor(light, dark);
    viewport()->update();
}
void RoundListView::clearSelection() { QListView::clearSelection(); }
void RoundListView::setCurrentIndex(const QModelIndex& index) { QListView::setCurrentIndex(index); }
void RoundListView::keyPressEvent(QKeyEvent* event) { QListView::keyPressEvent(event); }
void RoundListView::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::RightButton && !isSelectRightClickedRow()) {
        event->ignore();
        return;
    }
    QListView::mousePressEvent(event);
}
void RoundListView::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::RightButton && !isSelectRightClickedRow()) {
        event->ignore();
        return;
    }
    QListView::mouseReleaseEvent(event);
}
TransparentRoundListWidget::TransparentRoundListWidget(QWidget* parent) : RoundListWidget(parent) {
    roundDelegate()->setTransparentSelection(true);
}
TransparentRoundListView::TransparentRoundListView(QWidget* parent) : RoundListView(parent) {
    roundDelegate()->setTransparentSelection(true);
}
CategoryCardListWidget::CategoryCardListWidget(QWidget* parent) : RoundListWidget(parent) {
    roundDelegate()->setKeepNormalBackground(true);
}
CategoryCardListView::CategoryCardListView(QWidget* parent) : RoundListView(parent) {
    roundDelegate()->setKeepNormalBackground(true);
}

}  // namespace qfw
