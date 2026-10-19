#pragma once
#include <utility>
#include "components/widgets/menu.h"
#include "components/widgets/acrylic_label.h"

namespace qfw {

class AcrylicMenuActionListWidget : public MenuActionListWidget {
    Q_OBJECT
public:
    explicit AcrylicMenuActionListWidget(QWidget* parent=nullptr);
    AcrylicBrush& acrylicBrush() { return brush_; }
    QPainterPath clipPath() const;
    void addItem(QListWidgetItem* item) override;
    void setItemHeight(int height) override;
    static QListWidgetItem* createPlaceholderItem(int height=6);
protected:
    void paintEvent(QPaintEvent* event) override;
private:
    AcrylicBrush brush_;
};

// The adapter retains each native Fluent menu's action and delegate behavior.
template<class Base> class AcrylicMenuAdapter : public Base {
    Q_DISABLE_COPY_MOVE(AcrylicMenuAdapter)
public:
    template<class... Args> explicit AcrylicMenuAdapter(Args&&... args)
        : Base(std::forward<Args>(args)...) {
        acrylicView_=new AcrylicMenuActionListWidget;
        this->replaceMenuView(acrylicView_);
    }
    AcrylicMenuActionListWidget* acrylicView() const { return acrylicView_; }
    void clear() override {
        Base::clear();
        while(acrylicView_->count()>2) delete acrylicView_->takeItem(1);
    }
protected:
    void prepareShowBackground(const QPoint& position) override {
        QWidget* viewport=acrylicView_->viewport();
        const QPoint offset=viewport->mapTo(this,QPoint());
        captureBackground(acrylicView_->acrylicBrush(),QRect(position+offset,viewport->size()));
    }
    virtual void captureBackground(AcrylicBrush& brush,const QRect& rect) { brush.grabImage(rect); }
private:
    AcrylicMenuActionListWidget* acrylicView_;
};

class AcrylicMenu : public AcrylicMenuAdapter<RoundMenu> {
    Q_OBJECT
public:
    explicit AcrylicMenu(const QString& title=QString(),QWidget* parent=nullptr);
};
class AcrylicCheckableMenu : public AcrylicMenuAdapter<CheckableMenu> {
    Q_OBJECT
public:
    explicit AcrylicCheckableMenu(const QString& title=QString(),QWidget* parent=nullptr,
                                 MenuIndicatorType type=MenuIndicatorType::Check);
};
class AcrylicLineEditMenu : public AcrylicMenuAdapter<LineEditMenu> {
    Q_OBJECT
public:
    explicit AcrylicLineEditMenu(QLineEdit* parent);
};
class AcrylicSystemTrayMenu : public AcrylicMenu {
    Q_OBJECT
public:
    using AcrylicMenu::AcrylicMenu;
protected:
    void showEvent(QShowEvent* event) override;
};
class AcrylicCheckableSystemTrayMenu : public AcrylicCheckableMenu {
    Q_OBJECT
public:
    using AcrylicCheckableMenu::AcrylicCheckableMenu;
protected:
    void showEvent(QShowEvent* event) override;
};
}
