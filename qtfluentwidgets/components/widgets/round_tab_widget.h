#pragma once

#include "round_tab_bar.h"

namespace qfw {
// Rounded browser tabs and an application-owned page stack. removeTab/clear do not delete pages.
class RoundTabWidget : public TabWidget {
    Q_OBJECT
    Q_PROPERTY(bool tabBarAutoHide READ tabBarAutoHide WRITE setTabBarAutoHide)
public:
    explicit RoundTabWidget(QWidget* parent = nullptr);
    int addPage(QWidget* page, const QString& label, const QString& routeKey = QString()) {
        return addTab(page, label, routeKey);
    }
    int addPage(QWidget* page, const QString& label, FluentIconEnum icon,
                const QString& routeKey = QString()) {
        return addTab(page, label, icon, routeKey);
    }
    int addTab(QWidget* page, const QString& label, const QString& routeKey = QString());
    int addTab(QWidget* page, const QString& label, FluentIconEnum icon,
               const QString& routeKey = QString()) override;
    int addTab(QWidget* page, FluentIconEnum icon, const QString& label);
    int insertTab(int index, QWidget* page, const QString& label,
                  const QString& routeKey = QString());
    int insertTab(int index, QWidget* page, const QString& label, FluentIconEnum icon,
                  const QString& routeKey = QString()) override;
    int insertTab(int index, QWidget* page, FluentIconEnum icon, const QString& label);
    void removeTab(int index) override;
    void clear() override;
    int indexOf(QWidget* page) const;
    void setCurrentIndex(int index) override;
    void setTabVisible(int index, bool visible) override;
    void setTabEnabled(int index, bool enabled) override;
    void setTabSelectedBackgroundColor(const QColor& light, const QColor& dark) override;
    void setTabBarAutoHide(bool enabled);
    bool tabBarAutoHide() const { return autoHide_; }
    void setTabWhatsThis(int index, const QString& text);
    QString tabWhatsThis(int index) const;
    QStackedWidget* stackedWidget() const { return stackedWidget_; }

protected:
    void paintEvent(QPaintEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;
    void onCurrentTabChanged(int index) override;
    void onTabMoved(int fromIndex, int toIndex) override;

private:
    class PageChange;
    int insertPage(int index, QWidget* page, const QString& label, const QString& routeKey,
                   const FluentIconEnum* icon);
    void finishChange(int oldIndex, QWidget* oldPage, bool notifyIndex);
    void updateAutoHide();
    void syncBar();
    void selectAvailable(int index, bool allowDisabled = false);
    void cycleTab(int step);
    QPainterPath framePath(bool outline = false) const;
    int changeDepth_ = 0;
    bool autoHide_ = false;
};
}  // namespace qfw
