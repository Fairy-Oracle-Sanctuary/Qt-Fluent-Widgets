#pragma once

#include <QFrame>
#include <QShowEvent>

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
    bool event(QEvent* e) override;

private:
    void applyMica();

    bool micaApplied_ = false;

    StandardTitleBar* titleBar_ = nullptr;
    QFrame* contentFrame_ = nullptr;
};

}  // namespace qfw
