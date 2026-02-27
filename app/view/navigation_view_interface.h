#pragma once

#include <QWidget>

#include "gallery_interface.h"

class QFrame;
class QHBoxLayout;
class QLabel;
class QStackedWidget;
class QVBoxLayout;

namespace qfw {

class BodyLabel;
class BreadcrumbBar;
class CheckBox;
class ComboBox;
class Pivot;
class SegmentedWidget;
class SegmentedToggleToolWidget;
class SpinBox;
class TabBar;

class PivotInterface : public QWidget {
    Q_OBJECT

public:
    explicit PivotInterface(QWidget* parent = nullptr);
    virtual ~PivotInterface() = default;

protected:
    void addSubInterface(QLabel* widget, const QString& objectName, const QString& text);
    void onCurrentIndexChanged(int index);

protected:
    Pivot* pivot_ = nullptr;
    QStackedWidget* stackedWidget_ = nullptr;
    QVBoxLayout* vBoxLayout_ = nullptr;
};

class SegmentedInterface : public PivotInterface {
    Q_OBJECT

public:
    explicit SegmentedInterface(QWidget* parent = nullptr);
};

class TabInterface : public QWidget {
    Q_OBJECT

public:
    explicit TabInterface(QWidget* parent = nullptr);

private:
    void initWidget();
    void initLayout();
    void connectSignalToSlot();
    void addSubInterface(QLabel* widget, const QString& objectName, const QString& text,
                         const QString& icon);
    void onCurrentIndexChanged(int index);
    void onDisplayModeChanged(int index);
    void addTab();
    void removeTab(int index);

private:
    int tabCount_ = 1;

    TabBar* tabBar_ = nullptr;
    QStackedWidget* stackedWidget_ = nullptr;
    QWidget* tabView_ = nullptr;
    QFrame* controlPanel_ = nullptr;

    CheckBox* movableCheckBox_ = nullptr;
    CheckBox* scrollableCheckBox_ = nullptr;
    CheckBox* shadowEnabledCheckBox_ = nullptr;
    BodyLabel* tabMaxWidthLabel_ = nullptr;
    SpinBox* tabMaxWidthSpinBox_ = nullptr;
    BodyLabel* closeDisplayModeLabel_ = nullptr;
    ComboBox* closeDisplayModeComboBox_ = nullptr;

    QHBoxLayout* hBoxLayout_ = nullptr;
    QVBoxLayout* vBoxLayout_ = nullptr;
    QVBoxLayout* panelLayout_ = nullptr;
};

class NavigationViewInterface : public GalleryInterface {
    Q_OBJECT

public:
    explicit NavigationViewInterface(QWidget* parent = nullptr);

private:
    QWidget* createToggleToolWidget();
};

}  // namespace qfw
