#pragma once

#include "window/fluent_window.h"
#include "components/navigation/navigation_interface.h"

namespace qfw {
class SearchLineEdit;
class ListWidget;
class BodyLabel;

// Expanded sidebar with accent-filled selection and non-activating page search.
class FilledFluentWindow : public FluentWindow {
    Q_OBJECT
public:
    explicit FilledFluentWindow(QWidget* parent = nullptr);
    SearchLineEdit* searchLineEdit() const { return searchLineEdit_; }
    NavigationInterface* navigationInterface() const { return navigationInterface_; }
    using FluentWindow::addSubInterface;
    NavigationWidget* addSubInterface(QWidget* page, const QVariant& icon,
        const QString& text, NavigationItemPosition position,
        QWidget* parent, bool isTransparent) override;
    void removeInterface(QWidget* page, bool isDelete = false) override;

signals:
    void pageSearchActivated(QWidget* page);

protected:
    bool eventFilter(QObject* object, QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    void updatePageSearch();
    void activateSearchResult();
    void repositionSearch();
    void refreshSearchTheme();
    void layoutTitleBar();
    SearchLineEdit* searchLineEdit_ = nullptr;
    QFrame* searchPopup_ = nullptr;
    ListWidget* searchResults_ = nullptr;
    BodyLabel* emptyLabel_ = nullptr;
    QString searchQuery_;
};
} // namespace qfw
