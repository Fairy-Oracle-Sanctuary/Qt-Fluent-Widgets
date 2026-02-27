#pragma once

#include <QFrame>
#include <QShowEvent>

#include "components/window/windows_frameless_window.h"

namespace qfw {

class StandardTitleBar;

class FluentMainWindow : public WindowsFramelessMainWindow {
    Q_OBJECT

public:
    explicit FluentMainWindow(QWidget* parent = nullptr);

    void setContentWidget(QWidget* widget);

protected:
    void showEvent(QShowEvent* e) override;

private:
    void applyMica();

    bool micaApplied_ = false;

    StandardTitleBar* titleBar_ = nullptr;
    QFrame* contentFrame_ = nullptr;
};

}  // namespace qfw
