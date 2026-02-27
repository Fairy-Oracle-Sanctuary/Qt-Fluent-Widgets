#include "view/home_interface.h"

#include <QBrush>
#include <QLabel>
#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath>
#include <QVBoxLayout>

#include "common/app_config.h"
#include "common/config.h"
#include "common/gallery_style_sheet.h"
#include "components/link_card.h"
#include "components/sample_card.h"

namespace qfw {

// ==========================================================================
// BannerWidget
// ==========================================================================
BannerWidget::BannerWidget(QWidget* parent) : QWidget(parent) {
    setFixedHeight(336);
    setProperty("qssClass", "BannerWidget");
    setAttribute(Qt::WA_StyledBackground);

    vBoxLayout_ = new QVBoxLayout(this);
    galleryLabel_ = new QLabel(tr("Fluent Gallery"), this);
    banner_ = QPixmap(QStringLiteral(":/gallery/images/header1.png"));
    linkCardView_ = new LinkCardView(this);

    galleryLabel_->setObjectName(QStringLiteral("galleryLabel"));

    vBoxLayout_->setSpacing(0);
    vBoxLayout_->setContentsMargins(0, 20, 0, 0);
    vBoxLayout_->addWidget(galleryLabel_);
    vBoxLayout_->addWidget(linkCardView_, 1, Qt::AlignBottom);
    vBoxLayout_->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    linkCardView_->addCard(QStringLiteral(":/gallery/images/logo.png"), tr("Getting started"),
                           tr("An overview of app development options and samples."),
                           QString::fromUtf8(AppConfig::HELP_URL));

    linkCardView_->addCard(
        FluentIconEnum::Github, tr("GitHub repo"),
        tr("The latest fluent design controls and styles for your applications."),
        QString::fromUtf8(AppConfig::REPO_URL));

    linkCardView_->addCard(FluentIconEnum::Code, tr("Code samples"),
                           tr("Find samples that demonstrate specific tasks, features and APIs."),
                           QString::fromUtf8(AppConfig::EXAMPLE_URL));

    linkCardView_->addCard(FluentIconEnum::Feedback, tr("Send feedback"),
                           tr("Help us improve Qt-Fluent-Widgets by providing feedback."),
                           QString::fromUtf8(AppConfig::FEEDBACK_URL));
}

void BannerWidget::paintEvent(QPaintEvent* e) {
    QWidget::paintEvent(e);

    QPainter painter(this);
    painter.setRenderHints(QPainter::SmoothPixmapTransform | QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    QPainterPath path;
    path.setFillRule(Qt::WindingFill);

    const int w = width();
    const int h = height();

    path.addRoundedRect(QRectF(0, 0, w, h), 10, 10);
    path.addRect(QRectF(0, h - 50, 50, 50));
    path.addRect(QRectF(w - 50, 0, 50, 50));
    path.addRect(QRectF(w - 50, h - 50, 50, 50));
    path = path.simplified();

    QLinearGradient gradient(0, 0, 0, h);

    if (!isDarkTheme()) {
        gradient.setColorAt(0, QColor(207, 216, 228, 255));
        gradient.setColorAt(1, QColor(207, 216, 228, 0));
    } else {
        gradient.setColorAt(0, QColor(0, 0, 0, 255));
        gradient.setColorAt(1, QColor(0, 0, 0, 0));
    }

    painter.fillPath(path, QBrush(gradient));

    const QPixmap pixmap = banner_.scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    painter.fillPath(path, QBrush(pixmap));
}

// ==========================================================================
// HomeInterface
// ==========================================================================
HomeInterface::HomeInterface(QWidget* parent) : ScrollArea(parent) {
    banner_ = new BannerWidget(this);
    view_ = new QWidget(this);
    vBoxLayout_ = new QVBoxLayout(view_);

    initWidget();
    loadSamples();
}

void HomeInterface::initWidget() {
    view_->setObjectName(QStringLiteral("view"));
    setObjectName(QStringLiteral("homeInterface"));
    setProperty("qssClass", QStringLiteral("HomeInterface"));

    applyGalleryStyleSheet(this, GalleryStyleSheet::HomeInterface);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setWidget(view_);
    setWidgetResizable(true);

    vBoxLayout_->setContentsMargins(0, 0, 0, 36);
    vBoxLayout_->setSpacing(40);
    vBoxLayout_->addWidget(banner_);

    vBoxLayout_->setAlignment(Qt::AlignTop);
}

void HomeInterface::loadSamples() {
    // basic input samples
    auto* basicInputView = new SampleCardView(tr("Basic input samples"), view_);
    basicInputView->addSampleCard(
        QStringLiteral(":/gallery/images/controls/Button.png"), QStringLiteral("Button"),
        tr("A control that responds to user input and emit clicked signal."),
        QStringLiteral("basicInputInterface"), 0);
    basicInputView->addSampleCard(
        QStringLiteral(":/gallery/images/controls/Checkbox.png"), QStringLiteral("CheckBox"),
        tr("A control that a user can select or clear."), QStringLiteral("basicInputInterface"), 8);
    basicInputView->addSampleCard(QStringLiteral(":/gallery/images/controls/ComboBox.png"),
                                  QStringLiteral("ComboBox"),
                                  tr("A drop-down list of items a user can select from."),
                                  QStringLiteral("basicInputInterface"), 10);
    basicInputView->addSampleCard(QStringLiteral(":/gallery/images/controls/DropDownButton.png"),
                                  QStringLiteral("DropDownButton"),
                                  tr("A button that displays a flyout of choices when clicked."),
                                  QStringLiteral("basicInputInterface"), 12);
    basicInputView->addSampleCard(QStringLiteral(":/gallery/images/controls/HyperlinkButton.png"),
                                  QStringLiteral("HyperlinkButton"),
                                  tr("A button that appears as hyperlink text, and can navigate to "
                                     "a URI or handle a Click event."),
                                  QStringLiteral("basicInputInterface"), 18);
    basicInputView->addSampleCard(
        QStringLiteral(":/gallery/images/controls/RadioButton.png"), QStringLiteral("RadioButton"),
        tr("A control that allows a user to select a single option from a group of options."),
        QStringLiteral("basicInputInterface"), 19);
    basicInputView->addSampleCard(QStringLiteral(":/gallery/images/controls/Slider.png"),
                                  QStringLiteral("Slider"),
                                  tr("A control that lets the user select from a range of values "
                                     "by moving a Thumb control along a track."),
                                  QStringLiteral("basicInputInterface"), 20);
    basicInputView->addSampleCard(
        QStringLiteral(":/gallery/images/controls/SplitButton.png"), QStringLiteral("SplitButton"),
        tr("A two-part button that displays a flyout when its secondary part is clicked."),
        QStringLiteral("basicInputInterface"), 21);
    basicInputView->addSampleCard(QStringLiteral(":/gallery/images/controls/ToggleSwitch.png"),
                                  QStringLiteral("SwitchButton"),
                                  tr("A switch that can be toggled between 2 states."),
                                  QStringLiteral("basicInputInterface"), 25);
    basicInputView->addSampleCard(
        QStringLiteral(":/gallery/images/controls/ToggleButton.png"),
        QStringLiteral("ToggleButton"),
        tr("A button that can be switched between two states like a CheckBox."),
        QStringLiteral("basicInputInterface"), 26);
    vBoxLayout_->addWidget(basicInputView);

    // date time samples
    auto* dateTimeView = new SampleCardView(tr("Date & time samples"), view_);
    dateTimeView->addSampleCard(
        QStringLiteral(":/gallery/images/controls/CalendarDatePicker.png"),
        QStringLiteral("CalendarPicker"),
        tr("A control that lets a user pick a date value using a calendar."),
        QStringLiteral("dateTimeInterface"), 0);
    dateTimeView->addSampleCard(QStringLiteral(":/gallery/images/controls/DatePicker.png"),
                                QStringLiteral("DatePicker"),
                                tr("A control that lets a user pick a date value."),
                                QStringLiteral("dateTimeInterface"), 2);
    dateTimeView->addSampleCard(QStringLiteral(":/gallery/images/controls/TimePicker.png"),
                                QStringLiteral("TimePicker"),
                                tr("A configurable control that lets a user pick a time value."),
                                QStringLiteral("dateTimeInterface"), 4);
    vBoxLayout_->addWidget(dateTimeView);

    // dialog samples
    auto* dialogView = new SampleCardView(tr("Dialog samples"), view_);
    dialogView->addSampleCard(QStringLiteral(":/gallery/images/controls/Flyout.png"),
                              QStringLiteral("Dialog"), tr("A frameless message dialog."),
                              QStringLiteral("dialogInterface"), 0);
    dialogView->addSampleCard(QStringLiteral(":/gallery/images/controls/ContentDialog.png"),
                              QStringLiteral("MessageBox"), tr("A message dialog with mask."),
                              QStringLiteral("dialogInterface"), 1);
    dialogView->addSampleCard(
        QStringLiteral(":/gallery/images/controls/ColorPicker.png"), QStringLiteral("ColorDialog"),
        tr("A dialog that allows user to select color."), QStringLiteral("dialogInterface"), 2);
    dialogView->addSampleCard(QStringLiteral(":/gallery/images/controls/Flyout.png"),
                              QStringLiteral("Flyout"),
                              tr("Shows contextual information and enables user interaction."),
                              QStringLiteral("dialogInterface"), 3);
    dialogView->addSampleCard(
        QStringLiteral(":/gallery/images/controls/TeachingTip.png"), QStringLiteral("TeachingTip"),
        tr("A content-rich flyout for guiding users and enabling teaching moments."),
        QStringLiteral("dialogInterface"), 5);
    vBoxLayout_->addWidget(dialogView);

    // layout samples
    auto* layoutView = new SampleCardView(tr("Layout samples"), view_);
    layoutView->addSampleCard(QStringLiteral(":/gallery/images/controls/Grid.png"),
                              QStringLiteral("FlowLayout"),
                              tr("A layout arranges components in a left-to-right flow, wrapping "
                                 "to the next row when the current row is full."),
                              QStringLiteral("layoutInterface"), 0);
    vBoxLayout_->addWidget(layoutView);

    // material samples
    auto* materialView = new SampleCardView(tr("Material samples"), view_);
    materialView->addSampleCard(QStringLiteral(":/gallery/images/controls/Acrylic.png"),
                                QStringLiteral("AcrylicLabel"),
                                tr("A translucent material recommended for panel background."),
                                QStringLiteral("materialInterface"), 0);
    vBoxLayout_->addWidget(materialView);

    // menu samples
    auto* menuView = new SampleCardView(tr("Menu & toolbars samples"), view_);
    menuView->addSampleCard(QStringLiteral(":/gallery/images/controls/MenuFlyout.png"),
                            QStringLiteral("RoundMenu"),
                            tr("Shows a contextual list of simple commands or options."),
                            QStringLiteral("menuInterface"), 0);
    menuView->addSampleCard(QStringLiteral(":/gallery/images/controls/CommandBar.png"),
                            QStringLiteral("CommandBar"),
                            tr("Shows a contextual list of simple commands or options."),
                            QStringLiteral("menuInterface"), 2);
    menuView->addSampleCard(
        QStringLiteral(":/gallery/images/controls/CommandBarFlyout.png"),
        QStringLiteral("CommandBarFlyout"),
        tr("A mini-toolbar displaying proactive commands, and an optional menu of commands."),
        QStringLiteral("menuInterface"), 3);
    vBoxLayout_->addWidget(menuView);

    // navigation
    auto* navigationView = new SampleCardView(tr("Navigation"), view_);
    navigationView->addSampleCard(
        QStringLiteral(":/gallery/images/controls/BreadcrumbBar.png"),
        QStringLiteral("BreadcrumbBar"),
        tr("Shows the trail of navigation taken to the current location."),
        QStringLiteral("navigationViewInterface"), 0);
    navigationView->addSampleCard(
        QStringLiteral(":/gallery/images/controls/Pivot.png"), QStringLiteral("Pivot"),
        tr("Presents information from different sources in a tabbed view."),
        QStringLiteral("navigationViewInterface"), 1);
    navigationView->addSampleCard(
        QStringLiteral(":/gallery/images/controls/TabView.png"), QStringLiteral("TabView"),
        tr("Presents information from different sources in a tabbed view."),
        QStringLiteral("navigationViewInterface"), 3);
    vBoxLayout_->addWidget(navigationView);

    // scrolling
    auto* scrollView = new SampleCardView(tr("Scrolling samples"), view_);
    scrollView->addSampleCard(
        QStringLiteral(":/gallery/images/controls/ScrollViewer.png"), QStringLiteral("ScrollArea"),
        tr("A container control that lets the user pan and zoom its content smoothly."),
        QStringLiteral("scrollInterface"), 0);
    scrollView->addSampleCard(
        QStringLiteral(":/gallery/images/controls/PipsPager.png"), QStringLiteral("PipsPager"),
        tr("A control to let the user navigate through a paginated collection when the page "
           "numbers do not need to be visually known."),
        QStringLiteral("scrollInterface"), 3);
    vBoxLayout_->addWidget(scrollView);

    // status & info
    auto* statusInfoView = new SampleCardView(tr("Status & info samples"), view_);
    statusInfoView->addSampleCard(QStringLiteral(":/gallery/images/controls/ProgressRing.png"),
                                  QStringLiteral("StateToolTip"),
                                  tr("Shows the apps progress on a task, or that the app is "
                                     "performing ongoing work that does block user interaction."),
                                  QStringLiteral("statusInfoInterface"), 0);
    statusInfoView->addSampleCard(QStringLiteral(":/gallery/images/controls/ToolTip.png"),
                                  QStringLiteral("ToolTip"),
                                  tr("Displays information for an element in a pop-up window."),
                                  QStringLiteral("statusInfoInterface"), 1);
    statusInfoView->addSampleCard(
        QStringLiteral(":/gallery/images/controls/InfoBadge.png"), QStringLiteral("InfoBadge"),
        tr("An non-intrusive Ul to display notifications or bring focus to an area."),
        QStringLiteral("statusInfoInterface"), 3);
    statusInfoView->addSampleCard(
        QStringLiteral(":/gallery/images/controls/InfoBar.png"), QStringLiteral("InfoBar"),
        tr("An inline message to display app-wide status change information."),
        QStringLiteral("statusInfoInterface"), 4);
    statusInfoView->addSampleCard(
        QStringLiteral(":/gallery/images/controls/ProgressBar.png"), QStringLiteral("ProgressBar"),
        tr("Shows the apps progress on a task, or that the app is performing ongoing work that "
           "doesn't block user interaction."),
        QStringLiteral("statusInfoInterface"), 8);
    statusInfoView->addSampleCard(
        QStringLiteral(":/gallery/images/controls/ProgressRing.png"),
        QStringLiteral("ProgressRing"),
        tr("Shows the apps progress on a task, or that the app is performing ongoing work that "
           "doesn't block user interaction."),
        QStringLiteral("statusInfoInterface"), 10);
    vBoxLayout_->addWidget(statusInfoView);

    // text
    auto* textView = new SampleCardView(tr("Text samples"), view_);
    textView->addSampleCard(QStringLiteral(":/gallery/images/controls/TextBox.png"),
                            QStringLiteral("LineEdit"), tr("A single-line plain text field."),
                            QStringLiteral("textInterface"), 0);
    textView->addSampleCard(QStringLiteral(":/gallery/images/controls/PasswordBox.png"),
                            QStringLiteral("PasswordLineEdit"),
                            tr("A control for entering passwords."),
                            QStringLiteral("textInterface"), 2);
    textView->addSampleCard(
        QStringLiteral(":/gallery/images/controls/NumberBox.png"), QStringLiteral("SpinBox"),
        tr("A text control used for numeric input and evaluation of algebraic equations."),
        QStringLiteral("textInterface"), 3);
    textView->addSampleCard(QStringLiteral(":/gallery/images/controls/RichEditBox.png"),
                            QStringLiteral("TextEdit"),
                            tr("A rich text editing control that supports formatted text, "
                               "hyperlinks, and other rich content."),
                            QStringLiteral("textInterface"), 8);
    vBoxLayout_->addWidget(textView);

    // view
    auto* viewSamples = new SampleCardView(tr("View samples"), view_);
    viewSamples->addSampleCard(
        QStringLiteral(":/gallery/images/controls/ListView.png"), QStringLiteral("ListView"),
        tr("A control that presents a collection of items in a vertical list."),
        QStringLiteral("viewInterface"), 0);
    viewSamples->addSampleCard(QStringLiteral(":/gallery/images/controls/DataGrid.png"),
                               QStringLiteral("TableView"),
                               tr("The DataGrid control provides a flexible way to display a "
                                  "collection of data in rows and columns."),
                               QStringLiteral("viewInterface"), 1);
    viewSamples->addSampleCard(QStringLiteral(":/gallery/images/controls/TreeView.png"),
                               QStringLiteral("TreeView"),
                               tr("The TreeView control is a hierarchical list pattern with "
                                  "expanding and collapsing nodes that contain nested items."),
                               QStringLiteral("viewInterface"), 2);
    viewSamples->addSampleCard(
        QStringLiteral(":/gallery/images/controls/FlipView.png"), QStringLiteral("FlipView"),
        tr("Presents a collection of items that the user can flip through,one item at a time."),
        QStringLiteral("viewInterface"), 4);
    vBoxLayout_->addWidget(viewSamples);
}

}  // namespace qfw
