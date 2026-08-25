#pragma once

#include <QFrame>
#include <QShowEvent>
#include <QTimer>

#include "components/window/frameless_window.h"

namespace qfw {

class StandardTitleBar;

class FluentMainWindow : public FramelessMainWindow {
    Q_OBJECT

public:
    explicit FluentMainWindow(QWidget* parent = nullptr);

    void setContentWidget(QWidget* widget);

protected:
    void showEvent(QShowEvent* e) override;
    void paintEvent(QPaintEvent* e) override;

    virtual void applyMica();

    bool micaApplied_ = false;
    QTimer* micaRefreshTimer_ = nullptr;

    StandardTitleBar* titleBar_ = nullptr;
    QFrame* contentFrame_ = nullptr;
};

}  // namespace qfw
