#include "components/widgets/extended_table_view.h"

#include "common/style_sheet.h"

namespace qfw {
namespace {
void initializeTable(QTableView* view, bool line) {
    view->setProperty("isBorderVisible", false);
    view->verticalHeader()->hide();
    view->verticalHeader()->setDefaultSectionSize(48);
    auto* delegate = qobject_cast<TableItemDelegate*>(view->itemDelegate());
    delegate->setShowIndicator(false);
    delegate->setBorderRadius(line ? 0 : 8);
    if (line) {
        view->setProperty("isLineTable", true);
        view->horizontalHeader()->setFixedHeight(52);
        view->horizontalHeader()->setStretchLastSection(false);
        view->setAlternatingRowColors(false);
        delegate->setUseThemeColorSelection(true);
        delegate->setShowRowDivider(true);
        delegate->setRowMargin(0);
    } else {
        view->horizontalHeader()->hide();
        delegate->setUseStackedWidgetStyle(true);
    }
    updateDynamicStyle(view);
    auto repaint = [view] { view->viewport()->update(); };
    QObject::connect(&QConfig::instance(), &QConfig::themeChanged, view, repaint);
    QObject::connect(&QConfig::instance(), &QConfig::themeColorChanged, view, repaint);
}
}  // namespace

RoundTableWidget::RoundTableWidget(QWidget* parent) : TableWidget(parent) {
    initializeTable(this, false);
}
RoundTableView::RoundTableView(QWidget* parent) : TableView(parent) {
    initializeTable(this, false);
}
LineTableWidget::LineTableWidget(QWidget* parent) : TableWidget(parent) {
    initializeTable(this, true);
}
LineTableView::LineTableView(QWidget* parent) : TableView(parent) { initializeTable(this, true); }

}  // namespace qfw
