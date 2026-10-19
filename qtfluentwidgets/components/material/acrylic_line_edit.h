#pragma once
#include "acrylic_menu.h"
#include "components/widgets/line_edit.h"
#include <QContextMenuEvent>

namespace qfw {
class AcrylicCompleterMenu : public AcrylicMenuAdapter<CompleterMenu> {
    Q_OBJECT
public:
    explicit AcrylicCompleterMenu(LineEdit* edit);
    void setItems(const QStringList& items) override;
protected:
    int completionRowOffset() const override { return 1; }
};

template<class Base> class AcrylicLineEditBase : public Base {
    Q_DISABLE_COPY_MOVE(AcrylicLineEditBase)
public:
    explicit AcrylicLineEditBase(QWidget* parent=nullptr) : Base(parent) {}
    void setCompleter(QCompleter* completer) override {
        Base::setCompleter(completer);
        this->setCompleterMenu(completer?new AcrylicCompleterMenu(this):nullptr);
    }
protected:
    void contextMenuEvent(QContextMenuEvent* event) override {
        auto* menu=new AcrylicLineEditMenu(this);
        QObject::connect(menu,&RoundMenu::closedSignal,menu,&QObject::deleteLater);
        menu->execAt(event->globalPos());
    }
};
class AcrylicLineEdit : public AcrylicLineEditBase<LineEdit> {
    Q_OBJECT
public:
    using AcrylicLineEditBase::AcrylicLineEditBase;
};
class AcrylicSearchLineEdit : public AcrylicLineEditBase<SearchLineEdit> {
    Q_OBJECT
public:
    using AcrylicLineEditBase::AcrylicLineEditBase;
};
}
