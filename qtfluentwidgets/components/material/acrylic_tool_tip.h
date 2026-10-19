#pragma once

#include "acrylic_widget.h"
#include "components/widgets/tool_tip.h"

namespace qfw {

class AcrylicToolTipContainer : public AcrylicWidget {
    Q_OBJECT
public:
    explicit AcrylicToolTipContainer(QWidget* parent = nullptr);
    QPainterPath acrylicClipPath() const override;
};

class AcrylicToolTip : public ToolTip {
    Q_OBJECT
public:
    explicit AcrylicToolTip(const QString& text = QString(), QWidget* parent = nullptr);
    AcrylicToolTipContainer* acrylicContainer() const;

protected:
    void showEvent(QShowEvent* event) override;
    // The capture hook also supports clients with an application-owned backdrop.
    virtual void captureBackground(const QRect& globalRect);
};

class AcrylicToolTipFilter : public ToolTipFilter {
    Q_OBJECT
public:
    explicit AcrylicToolTipFilter(QWidget* parent, int showDelay = 300,
                                 ToolTipPosition position = ToolTipPosition::Top);
    ~AcrylicToolTipFilter() override;

protected:
    ToolTip* createToolTip() override;

private:
    QPointer<AcrylicToolTip> ownedToolTip_;
};

}  // namespace qfw
