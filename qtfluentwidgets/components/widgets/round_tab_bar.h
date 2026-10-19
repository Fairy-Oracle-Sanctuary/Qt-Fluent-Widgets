#pragma once

#include "tab_view.h"

namespace qfw {

class RoundTabItem : public TabItem {
    Q_OBJECT
public:
    explicit RoundTabItem(QWidget* parent = nullptr);
    explicit RoundTabItem(const QString& text, QWidget* parent = nullptr);
    RoundTabItem(FluentIconEnum icon, const QString& text, QWidget* parent = nullptr);
    QSize sizeHint() const override;
    void setShadowEnabled(bool enabled) override;
    void clearIcon();
    QPainterPath selectedPath(bool outline = false, qreal bottom = -1) const;

protected:
    void paintEvent(QPaintEvent* event) override;
    virtual void drawRoundSelectedBackground(QPainter* painter);

private:
    void initRound();
};

// Browser-style tabs. Add and close buttons emit requests; pages belong to the application.
class RoundTabBar : public TabBar {
    Q_OBJECT
public:
    explicit RoundTabBar(QWidget* parent = nullptr);
    TabItem* addTab(const QString& routeKey, const QString& text);
    TabItem* addTab(const QString& routeKey, const QString& text, FluentIconEnum icon);
    TabItem* insertTab(int index, const QString& routeKey, const QString& text);
    TabItem* insertTab(int index, const QString& routeKey, const QString& text,
                       FluentIconEnum icon);
    QRect tabRect(int index) const override;
    bool isTabVisible(int index) const override;
    void setTabVisible(int index, bool visible) override;
    void setTabMaximumWidth(int width) override;

protected:
    TabItem* createTabItem(const QString& text, FluentIconEnum icon) override;
    void paintEvent(QPaintEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;
};
}  // namespace qfw
