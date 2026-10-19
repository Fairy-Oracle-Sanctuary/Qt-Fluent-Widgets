#include "acrylic_line_edit.h"
namespace qfw {
AcrylicCompleterMenu::AcrylicCompleterMenu(LineEdit* edit) : AcrylicMenuAdapter(edit) {
    connect(view(),&QListWidget::itemClicked,this,[this](QListWidgetItem* item) { onItemClicked(item); });
    // Replacement keeps the completer's indicator delegate and its 33px rows.
    view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    view()->setItemHeight(33);
}
void AcrylicCompleterMenu::setItems(const QStringList& items) {
    view()->QListWidget::clear(); items_=items;
    view()->QListWidget::addItem(AcrylicMenuActionListWidget::createPlaceholderItem());
    view()->QListWidget::addItem(AcrylicMenuActionListWidget::createPlaceholderItem());
    for(const QString& text:items) {
        auto* item=new QListWidgetItem(text); item->setSizeHint(QSize(1,33)); view()->addItem(item);
    }
    view()->adjustSizeForMenu(); adjustSize();
}
}
