#include "view/menu_interface.h"

#include <QAction>
#include <QColor>
#include <QFileDialog>
#include <QLabel>
#include <QPoint>
#include <QStandardPaths>
#include <QVBoxLayout>

#include "common/config.h"
#include "common/font.h"
#include "common/icon.h"
#include "common/translator.h"
#include "components/widgets/command_bar.h"
#include "components/widgets/flyout.h"
#include "components/widgets/label.h"
#include "components/widgets/menu.h"

namespace qfw {

// ProfileCard implementation
ProfileCard::ProfileCard(const QString& avatarPath, const QString& name, const QString& email,
                         QWidget* parent)
    : QWidget(parent) {
    setFixedSize(307, 82);

    avatar_ = new AvatarWidget(avatarPath, this);
    avatar_->setRadius(24);
    avatar_->move(2, 6);

    nameLabel_ = new BodyLabel(name, this);
    nameLabel_->move(64, 13);

    emailLabel_ = new CaptionLabel(email, this);
    emailLabel_->move(64, 32);

    logoutButton_ = new HyperlinkButton(QStringLiteral("https://qfluentwidgets.com"), tr("Logout"),
                                        this, QVariant::fromValue(FluentIconEnum::Link));
    logoutButton_->move(52, 48);
    qfw::setFont(logoutButton_, 13);

    // Set colors based on theme
    QColor nameColor = isDarkTheme() ? QColor(255, 255, 255) : QColor(0, 0, 0);
    nameLabel_->setStyleSheet(QStringLiteral("QLabel{color: %1}").arg(nameColor.name()));

    QColor emailColor = isDarkTheme() ? QColor(206, 206, 206) : QColor(96, 96, 96);
    emailLabel_->setStyleSheet(QStringLiteral("QLabel{color: %1}").arg(emailColor.name()));
}

// MenuInterface implementation
MenuInterface::MenuInterface(QWidget* parent)
    : GalleryInterface(Translator().menus(), QStringLiteral("qfluentwidgets.components.widgets"),
                       parent) {
    setObjectName(QStringLiteral("menuInterface"));

    createActions();

    // Context menu button
    button1_ = new PushButton(tr("Show menu"), this);
    connect(button1_, &PushButton::clicked, this, &MenuInterface::onButton1Clicked);

    addExampleCard(tr("Rounded corners menu"), button1_,
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/"
                                  "examples/menu/menu/demo.py"));

    // Custom widget menu button
    button3_ = new PushButton(tr("Show menu"), this);
    connect(button3_, &PushButton::clicked, this, &MenuInterface::onButton3Clicked);

    addExampleCard(tr("Rounded corners menu with custom widget"), button3_,
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/"
                                  "examples/menu/widget_menu/demo.py"));

    // Checkable menu button
    button2_ = new PushButton(tr("Show menu"), this);
    connect(button2_, &PushButton::clicked, this, &MenuInterface::onButton2Clicked);

    addExampleCard(tr("Checkable menu"), button2_,
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/"
                                  "examples/menu/menu/demo.py"));

    // Command bar
    addExampleCard(tr("Command bar"), createCommandBar(),
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/"
                                  "examples/menu/menu/command_bar.py"),
                   1);

    // Command bar flyout
    auto* widget = new QWidget(this);
    auto* layout = new QVBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);

    auto* label = new QLabel(tr("Click the image to open a command bar flyout"), widget);
    imageLabel_ = new ImageLabel(QStringLiteral(":/gallery/images/chidanta5.jpg"), widget);
    imageLabel_->scaledToWidth(350);
    imageLabel_->setBorderRadius(8, 8, 8, 8);
    connect(imageLabel_, &ImageLabel::clicked, this, &MenuInterface::onImageLabelClicked);

    layout->addWidget(label);
    layout->addWidget(imageLabel_);

    addExampleCard(tr("Command bar flyout"), widget,
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/"
                                  "examples/menu/command_bar/demo.py"),
                   1);
}

void MenuInterface::createActions() {
    // Create sort actions
    createTimeAction_ = new Action(FluentIcon(FluentIconEnum::Calendar), tr("Create Date"), this);
    shootTimeAction_ = new Action(FluentIcon(FluentIconEnum::Camera), tr("Shooting Date"), this);
    modifiedTimeAction_ = new Action(FluentIcon(FluentIconEnum::Edit), tr("Modified time"), this);
    nameAction_ = new Action(FluentIcon(FluentIconEnum::Font), tr("Name"), this);

    createTimeAction_->setCheckable(true);
    shootTimeAction_->setCheckable(true);
    modifiedTimeAction_->setCheckable(true);
    nameAction_->setCheckable(true);

    actionGroup1_ = new QActionGroup(this);
    actionGroup1_->addAction(createTimeAction_);
    actionGroup1_->addAction(shootTimeAction_);
    actionGroup1_->addAction(modifiedTimeAction_);
    actionGroup1_->addAction(nameAction_);

    // Create order actions
    ascendAction_ = new Action(FluentIcon(FluentIconEnum::Up), tr("Ascending"), this);
    descendAction_ = new Action(FluentIcon(FluentIconEnum::Down), tr("Descending"), this);

    ascendAction_->setCheckable(true);
    descendAction_->setCheckable(true);

    actionGroup2_ = new QActionGroup(this);
    actionGroup2_->addAction(ascendAction_);
    actionGroup2_->addAction(descendAction_);

    // Set defaults
    shootTimeAction_->setChecked(true);
    ascendAction_->setChecked(true);
}

void MenuInterface::onButton1Clicked() {
    QPoint pos = button1_->mapToGlobal(QPoint(button1_->width() + 5, -100));
    createMenu(pos);
}

void MenuInterface::onButton2Clicked() {
    QPoint pos = button2_->mapToGlobal(QPoint(button2_->width() + 5, -100));
    createCheckableMenu(pos);
}

void MenuInterface::onButton3Clicked() {
    QPoint pos = button3_->mapToGlobal(QPoint(button3_->width() + 5, -100));
    createCustomWidgetMenu(pos);
}

void MenuInterface::onImageLabelClicked() { createCommandBarFlyout(); }

void MenuInterface::onSaveImage() {
    QString path = QFileDialog::getSaveFileName(
        this, tr("Save image"), QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
        QStringLiteral("PNG (*.png)"));
    if (path.isEmpty()) {
        return;
    }
    imageLabel_->pixmap().save(path);
}

RoundMenu* MenuInterface::createMenu(const QPoint& pos) {
    auto* menu = new RoundMenu(QString(), this);

    // Add actions
    menu->addAction(new Action(FluentIcon(FluentIconEnum::Copy), tr("Copy"), menu));
    menu->addAction(new Action(FluentIcon(FluentIconEnum::Cut), tr("Cut"), menu));

    // Add sub menu
    auto* submenu = new RoundMenu(tr("Add to"), this);
    submenu->setIcon(QIcon(FluentIcon(FluentIconEnum::Add).icon()));
    submenu->addActions({new Action(FluentIcon(FluentIconEnum::Video), tr("Video"), submenu),
                         new Action(FluentIcon(FluentIconEnum::Music), tr("Music"), submenu)});
    menu->addMenu(submenu);

    // Add more actions
    menu->addActions({new Action(FluentIcon(FluentIconEnum::Paste), tr("Paste"), menu),
                      new Action(FluentIcon(FluentIconEnum::Cancel), tr("Undo"), menu)});

    // Add separator
    menu->addSeparator();
    auto* selectAllAct = new QAction(tr("Select all"), menu);
    menu->addAction(selectAllAct);

    // Insert actions
    auto* settingsAct = new Action(FluentIcon(FluentIconEnum::Setting), tr("Settings"), menu);
    menu->insertAction(selectAllAct, settingsAct);

    auto* helpAct = new Action(FluentIcon(FluentIconEnum::Help), tr("Help"), menu);
    auto* feedbackAct = new Action(FluentIcon(FluentIconEnum::Feedback), tr("Feedback"), menu);
    menu->insertActions(selectAllAct, {helpAct, feedbackAct});

    menu->execAt(pos, true);
    return menu;
}

RoundMenu* MenuInterface::createCustomWidgetMenu(const QPoint& pos) {
    auto* menu = new RoundMenu(QString(), this);

    // Add custom widget
    auto* card = new ProfileCard(QStringLiteral(":/gallery/images/baby2016.jpg"), tr("baby2016"),
                                 QStringLiteral("2185823427@qq.com"), menu);
    menu->addWidget(card, false);

    menu->addSeparator();
    menu->addActions(
        {new Action(FluentIcon(FluentIconEnum::People), tr("Manage account profile"), menu),
         new Action(FluentIcon(FluentIconEnum::ShoppingCart), tr("Payment method"), menu),
         new Action(FluentIcon(FluentIconEnum::Code), tr("Redemption code and gift card"), menu)});
    menu->addSeparator();
    menu->addAction(new Action(FluentIcon(FluentIconEnum::Setting), tr("Settings"), menu));

    menu->execAt(pos);
    return menu;
}

CheckableMenu* MenuInterface::createCheckableMenu(const QPoint& pos) {
    auto* menu = new CheckableMenu(QString(), this, MenuIndicatorType::Radio);

    menu->addActions({createTimeAction_, shootTimeAction_, modifiedTimeAction_, nameAction_});
    menu->addSeparator();
    menu->addActions({ascendAction_, descendAction_});

    if (!pos.isNull()) {
        menu->execAt(pos, true);
    }

    return menu;
}

CommandBar* MenuInterface::createCommandBar() {
    auto* bar = new CommandBar(this);
    bar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    bar->addActions({new Action(FluentIcon(FluentIconEnum::Add), tr("Add"), bar),
                     new Action(FluentIcon(FluentIconEnum::Rotate), tr("Rotate"), bar),
                     new Action(FluentIcon(FluentIconEnum::ZoomIn), tr("Zoom in"), bar),
                     new Action(FluentIcon(FluentIconEnum::ZoomOut), tr("Zoom out"), bar)});

    bar->addSeparator();
    auto* editAct = new Action(FluentIcon(FluentIconEnum::Edit), tr("Edit"), bar);
    editAct->setCheckable(true);
    bar->addActions({editAct, new Action(FluentIcon(FluentIconEnum::Info), tr("Info"), bar),
                     new Action(FluentIcon(FluentIconEnum::Delete), tr("Delete"), bar),
                     new Action(FluentIcon(FluentIconEnum::Share), tr("Share"), bar)});

    // Add custom widget
    auto* button =
        new TransparentDropDownPushButton(FluentIcon(FluentIconEnum::Scroll), tr("Sort"), this);
    button->setMenu(createCheckableMenu());
    button->setFixedHeight(34);
    qfw::setFont(button, 12);
    bar->addWidget(button);

    bar->addHiddenActions({new Action(FluentIcon(FluentIconEnum::Setting), tr("Settings"), bar)});

    return bar;
}

void MenuInterface::createCommandBarFlyout() {
    auto* view = new CommandBarView(this);

    auto* saveAct = new Action(FluentIcon(FluentIconEnum::Save), tr("Save"), view);
    connect(saveAct, &QAction::triggered, this, &MenuInterface::onSaveImage);

    view->addActions({new Action(FluentIcon(FluentIconEnum::Share), tr("Share"), view), saveAct,
                      new Action(FluentIcon(FluentIconEnum::Heart), tr("Add to favorite"), view),
                      new Action(FluentIcon(FluentIconEnum::Delete), tr("Delete"), view)});

    view->addHiddenAction(new Action(FluentIcon(FluentIconEnum::Print), tr("Print"), view));
    view->addHiddenAction(new Action(FluentIcon(FluentIconEnum::Setting), tr("Settings"), view));
    view->resizeToSuitableWidth();

    int x = imageLabel_->width();
    QPoint pos = imageLabel_->mapToGlobal(QPoint(x, 0));
    Flyout::make(view, pos, this, FlyoutAnimationType::FadeIn);
}

}  // namespace qfw
