#pragma once

#include <QWidget>

#include "gallery_interface.h"

class QHBoxLayout;
class QLabel;

namespace qfw {

class StateToolTip;
class ProgressBar;
class ProgressRing;

class ProgressWidget : public QWidget {
    Q_OBJECT

public:
    explicit ProgressWidget(QWidget* progressWidget, QWidget* parent = nullptr);

private:
    void onValueChanged(int value);

private:
    QWidget* progressWidget_ = nullptr;
};

class StatusInfoInterface : public GalleryInterface {
    Q_OBJECT

public:
    explicit StatusInfoInterface(QWidget* parent = nullptr);

private slots:
    void onStateButtonClicked();
    void createTopRightInfoBar();
    void createTopInfoBar();
    void createTopLeftInfoBar();
    void createBottomRightInfoBar();
    void createBottomInfoBar();
    void createBottomLeftInfoBar();

private:
    StateToolTip* stateTooltip_ = nullptr;
};

}  // namespace qfw
