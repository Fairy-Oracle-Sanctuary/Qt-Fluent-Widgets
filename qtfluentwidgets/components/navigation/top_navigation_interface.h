#pragma once

#include <QHBoxLayout>
#include <QWidget>

#include "components/navigation/top_navigation_panel.h"

namespace qfw {

class TopNavigationInterface : public QWidget {
    Q_OBJECT

public:
    explicit TopNavigationInterface(QWidget* parent = nullptr, bool showReturnButton = false);

    TopNavigationPanel* panel() const { return panel_; }

    TopNavigationPushButton* addItem(
        const QString& routeKey, const QVariant& icon, const QString& text,
        std::function<void()> onClick = nullptr, bool selectable = true,
        TopNavigationItemPosition position = TopNavigationItemPosition::Left,
        const QString& tooltip = QString(), bool expanded = false);

    TopNavigationPushButton* insertItem(
        int index, const QString& routeKey, const QVariant& icon, const QString& text,
        std::function<void()> onClick = nullptr, bool selectable = true,
        TopNavigationItemPosition position = TopNavigationItemPosition::Left,
        const QString& tooltip = QString(), bool expanded = false);

    void addWidget(const QString& routeKey, NavigationWidget* widget,
                   std::function<void()> onClick = nullptr,
                   TopNavigationItemPosition position = TopNavigationItemPosition::Left,
                   const QString& tooltip = QString());

    void insertWidget(int index, const QString& routeKey, NavigationWidget* widget,
                      std::function<void()> onClick = nullptr,
                      TopNavigationItemPosition position = TopNavigationItemPosition::Left,
                      const QString& tooltip = QString());

    void removeWidget(const QString& routeKey);

    void setCurrentItem(const QString& name);

    void expand() { panel_->expand(); }
    void collapse() { panel_->collapse(); }
    void toggle() { panel_->toggle(); }

    void setReturnButtonVisible(bool visible) { panel_->setReturnButtonVisible(visible); }

    Router* history() const { return panel_->history(); }

    bool isIndicatorAnimationEnabled() const { return panel_->isIndicatorAnimationEnabled(); }
    void setIndicatorAnimationEnabled(bool enabled) {
        panel_->setIndicatorAnimationEnabled(enabled);
    }

    void setItemExpanded(const QString& routeKey, bool expanded) {
        panel_->setItemExpanded(routeKey, expanded);
    }

    NavigationWidget* widget(const QString& routeKey) const { return panel_->widget(routeKey); }

signals:
    void displayModeChanged(TopNavigationDisplayMode mode);

protected:
    bool eventFilter(QObject* obj, QEvent* e) override;
    void resizeEvent(QResizeEvent* e) override;

private:
    TopNavigationPanel* panel_ = nullptr;
    QHBoxLayout* hBoxLayout_ = nullptr;
};

}  // namespace qfw
