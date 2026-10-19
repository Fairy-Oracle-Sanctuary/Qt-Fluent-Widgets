#include "view/material_interface.h"

#include <QColor>
#include <QSize>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "common/app_config.h"
#include "common/translator.h"
#include "components/widgets/acrylic_label.h"
#include "components/material/acrylic_widget.h"
#include "components/material/acrylic_tool_tip.h"
#include "components/material/acrylic_flyout.h"
#include "components/material/acrylic_menu.h"
#include "components/material/acrylic_line_edit.h"
#include "components/material/acrylic_combo_box.h"
#include "components/widgets/line_edit.h"
#include <QContextMenuEvent>
#include "components/widgets/button.h"
#include "components/widgets/label.h"

namespace qfw {

class AcrylicMenuEditPreview : public LineEdit {
public:
    using LineEdit::LineEdit;
protected:
    void contextMenuEvent(QContextMenuEvent* event) override {
        auto* menu=new AcrylicLineEditMenu(this);
        connect(menu,&RoundMenu::closedSignal,menu,&QObject::deleteLater);
        menu->execAt(event->globalPos());
    }
};

class CustomAcrylicFlyoutView : public AcrylicFlyoutViewBase {
public:
    CustomAcrylicFlyoutView(const QString& text) {
        auto* column = new QVBoxLayout(this);
        column->setContentsMargins(20,16,20,16);
        column->addWidget(new BodyLabel(text,this));
    }
    void addWidget(QWidget* widget,int stretch=0,Qt::Alignment alignment=Qt::AlignLeft) override {
        static_cast<QVBoxLayout*>(layout())->addWidget(widget,stretch,alignment);
    }
};

MaterialInterface::MaterialInterface(QWidget* parent)
    : GalleryInterface(Translator().material(), QStringLiteral("qtfluentwidgets.components.widgets"),
                       parent) {
    setObjectName(QStringLiteral("materialInterface"));
    auto* comboHost=new QWidget;
    auto* comboRow=new QHBoxLayout(comboHost); comboRow->setContentsMargins(0,0,0,0);
    auto* combo=new AcrylicComboBox(comboHost);
    auto* editable=new AcrylicEditableComboBox(comboHost);
    const QStringList choices={tr("Acrylic"),tr("Animation"),tr("Application"),tr("Disabled option")};
    combo->addItems(choices); editable->addItems(choices);
    combo->setItemEnabled(3,false); editable->setItemEnabled(3,false);
    combo->setMinimumWidth(180); editable->setMinimumWidth(220); editable->setClearButtonEnabled(true);
    auto* completion=new QCompleter(choices.mid(0,3),editable); completion->setCaseSensitivity(Qt::CaseInsensitive);
    editable->setCompleter(completion);
    comboRow->addWidget(combo); comboRow->addWidget(editable); comboRow->addStretch();
    addExampleCard(tr("Acrylic combo boxes"),comboHost,QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets"),1);
    auto* selection=new OptionsConfigItem("MaterialPreview","Selection",0,
        std::make_shared<OptionsValidator>(QList<QVariant>{0,1,2})); selection->setParent(this);
    auto* comboCard=new AcrylicComboBoxSettingCard(selection,FluentIconEnum::Palette,tr("Acrylic option"),
        tr("Choose an option from the acrylic menu."),choices.mid(0,3));
    comboCard->setMinimumWidth(520);
    addExampleCard(tr("Acrylic combo box setting card"),comboCard,QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets"),1);

    auto* menuHost=new QWidget;
    auto* menuRow=new QHBoxLayout(menuHost); menuRow->setContentsMargins(0,0,0,0);
    for(int kind=0;kind<4;++kind) {
        const QString names[]={tr("Acrylic menu"),tr("Acrylic checkable menu"),tr("Acrylic tray menu"),tr("Acrylic checkable tray menu")};
        auto* button=new PushButton(names[kind],menuHost); menuRow->addWidget(button);
        connect(button,&QAbstractButton::clicked,this,[this,button,kind] {
            RoundMenu* menu=nullptr;
            if(kind==0) menu=new AcrylicMenu(QString(),window());
            else if(kind==1) menu=new AcrylicCheckableMenu(QString(),window());
            else if(kind==2) menu=new AcrylicSystemTrayMenu(QString(),window());
            else menu=new AcrylicCheckableSystemTrayMenu(QString(),window());
            auto* first=new QAction(toQIcon(FluentIcon(FluentIconEnum::Add)),tr("Add item"),menu);
            first->setShortcut(QKeySequence("Ctrl+N"));
            if(auto* check=qobject_cast<AcrylicCheckableMenu*>(menu)) {
                first->setCheckable(true); first->setChecked(true); check->addAction(first);
            } else menu->addAction(first);
            menu->addSeparator();
            auto* disabled=new QAction(tr("Disabled action"),menu); disabled->setEnabled(false); menu->addAction(disabled);
            if(kind==0) { auto* child=new AcrylicMenu(tr("More actions"),menu); child->addAction(new QAction(tr("Child action"),child)); menu->addMenu(child); }
            connect(menu,&RoundMenu::closedSignal,menu,&QObject::deleteLater);
            menu->execAt(button->mapToGlobal(QPoint(0,button->height())));
        });
    }
    addExampleCard(tr("Acrylic menus"),menuHost,QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets"),1);
    auto* menuEdit=new AcrylicMenuEditPreview;
    menuEdit->setText(tr("Select this text and right-click to open the acrylic edit menu."));
    menuEdit->setMinimumWidth(400);
    addExampleCard(tr("Acrylic line edit menu"),menuEdit,QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets"),1);
    auto* editHost=new QWidget;
    auto* editRow=new QHBoxLayout(editHost); editRow->setContentsMargins(0,0,0,0);
    auto* acrylicEdit=new AcrylicLineEdit(editHost);
    auto* acrylicSearch=new AcrylicSearchLineEdit(editHost);
    acrylicEdit->setPlaceholderText(tr("Type A to show acrylic completions"));
    acrylicSearch->setPlaceholderText(tr("Search with acrylic completions"));
    acrylicEdit->setClearButtonEnabled(true);
    const QStringList words={QStringLiteral("Acrylic"),QStringLiteral("Animation"),QStringLiteral("Application"),QStringLiteral("Azure")};
    for(LineEdit* edit:{static_cast<LineEdit*>(acrylicEdit),static_cast<LineEdit*>(acrylicSearch)}) {
        auto* completer=new QCompleter(words,edit); completer->setCaseSensitivity(Qt::CaseInsensitive);
        edit->setCompleter(completer); editRow->addWidget(edit);
    }
    addExampleCard(tr("Acrylic line edit and search"),editHost,QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets"),1);

    auto* flyoutHost = new QWidget;
    auto* flyoutLayout = new QHBoxLayout(flyoutHost);
    flyoutLayout->setContentsMargins(0,0,0,0);
    auto* flyoutButton = new PushButton(tr("Show acrylic flyout"),flyoutHost);
    auto* customButton = new PushButton(tr("Show custom acrylic flyout"),flyoutHost);
    flyoutLayout->addWidget(flyoutButton); flyoutLayout->addWidget(customButton); flyoutLayout->addStretch();
    connect(flyoutButton,&QAbstractButton::clicked,this,[this,flyoutButton] {
        AcrylicFlyout::create(tr("Acrylic flyout"),tr("The blurred background is captured before the flyout opens."),
                             toQIcon(FluentIcon(FluentIconEnum::Info)),{},true,
                             QVariant::fromValue<QWidget*>(flyoutButton),window());
    });
    connect(customButton,&QAbstractButton::clicked,this,[this,customButton] {
        auto* view=new CustomAcrylicFlyoutView(tr("Custom content uses the same acrylic surface."));
        auto* closeButton=new PushButton(tr("Close"),view); view->addWidget(closeButton);
        auto* flyout=AcrylicFlyout::make(view,QVariant::fromValue<QWidget*>(customButton),window());
        connect(closeButton,&QAbstractButton::clicked,flyout,&QWidget::close);
    });
    addExampleCard(tr("Acrylic flyout and custom view"),flyoutHost,
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets"),1);

    auto* material = new AcrylicWidget;
    material->setMinimumSize(320, 120);
    material->acrylicBrush().setBlurPicSize(QSize(640, 240));
    material->acrylicBrush().setImage(QPixmap(QStringLiteral(":/gallery/images/chidanta.jpg")));
    auto* materialLayout = new QVBoxLayout(material);
    materialLayout->setContentsMargins(24, 20, 24, 20);
    materialLayout->addWidget(new BodyLabel(tr("Acrylic surface"), material));
    materialLayout->addWidget(new BodyLabel(tr("Blurred background, theme tint and noise texture."), material));
    addExampleCard(tr("Acrylic widget"), material,
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets"), 1);

    auto* tooltipHost = new QWidget;
    auto* tooltipLayout = new QHBoxLayout(tooltipHost);
    tooltipLayout->setContentsMargins(0, 0, 0, 0);
    auto* tooltipButton = new PushButton(tr("Hover for acrylic tooltip"), tooltipHost);
    tooltipButton->setToolTip(tr("Acrylic tooltip follows the background and current theme."));
    tooltipButton->setToolTipDuration(2000);
    tooltipButton->installEventFilter(new AcrylicToolTipFilter(tooltipButton, 300, ToolTipPosition::Top));
    tooltipLayout->addWidget(tooltipButton);
    tooltipLayout->addStretch();
    addExampleCard(tr("Acrylic tooltip"), tooltipHost,
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets"), 1);

    label_ = new AcrylicLabel(appConfig().getBlurRadius(), QColor(105, 114, 168, 102),
                              QColor(255, 255, 255, 0), QSize(), this);
    label_->setImage(QStringLiteral(":/gallery/images/chidanta.jpg"));
    label_->setMaximumSize(787, 579);
    label_->setMinimumSize(197, 145);

    connect(&appConfig(), &AppConfig::blurRadiusChanged, this,
            &MaterialInterface::onBlurRadiusChanged);

    addExampleCard(tr("Acrylic label"), label_,
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/"
                                  "examples/material/acrylic_label/demo.py"),
                   1);
}

void MaterialInterface::onBlurRadiusChanged(int radius) { label_->setBlurRadius(radius); }

}  // namespace qfw
