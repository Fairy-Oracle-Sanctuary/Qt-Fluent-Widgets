#include "view/pro_navigation_examples.h"

#include "qtfluentwidgets.h"
#include "view/gallery_interface.h"

namespace qfw {
namespace {
QString sourcePath() {
    return QStringLiteral(
        "https://github.com/Fairy-Oracle-Sanctuary/Qt-Fluent-Widgets/blob/main/app/view/"
        "pro_navigation_examples.cpp");
}
QWidget* column(int maximumWidth) {
    auto* host = new QWidget;
    host->setMaximumWidth(maximumWidth);
    auto* layout = new QVBoxLayout(host);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);
    return host;
}
}  // namespace
void ProNavigationExamples::addNavigation(GalleryInterface* gallery) {
    auto* filledButton = new PushButton(tr("Open FilledFluentWindow"));
    connect(filledButton, &QAbstractButton::clicked, gallery, [gallery]() {
        auto* window = new FilledFluentWindow(gallery->window());
        window->setAttribute(Qt::WA_DeleteOnClose);
        window->setWindowTitle(tr("Filled navigation and page search"));
        window->resize(900, 640);
        const QStringList titles = {tr("Home"), tr("Documents"), tr("Settings")};
        const QList<FluentIconEnum> icons = {FluentIconEnum::Home, FluentIconEnum::Document,
                                            FluentIconEnum::Setting};
        for (int i = 0; i < titles.size(); ++i) {
            auto* page = new QWidget;
            page->setObjectName(QStringLiteral("filledPage%1").arg(i));
            auto* layout = new QVBoxLayout(page);
            auto* label = new TitleLabel(titles[i], page);
            layout->addWidget(label);
            layout->addWidget(new BodyLabel(tr("Type a page name in the sidebar; use Up/Down and Enter to navigate."), page));
            layout->addStretch();
            window->addSubInterface(page, icons[i], titles[i]);
        }
        window->show();
        window->setMicaEffectEnabled(true);
    });
    gallery->addExampleCard(tr("Filled navigation and page search"), filledButton, sourcePath());
    auto* menuHost = column(760);
    auto* menuLayout = static_cast<QVBoxLayout*>(menuHost->layout());
    auto* menuBar = new MenuBar(menuHost);
    auto* menuStatus = new BodyLabel(tr("Choose a menu command"), menuHost);
    auto* fileMenu = menuBar->addMenu(tr("&File"));
    auto* openAction = new QAction(tr("Open"), fileMenu);
    openAction->setShortcut(QKeySequence("Ctrl+O"));
    fileMenu->addAction(openAction);
    auto* saveAction = new QAction(tr("Save"), fileMenu);
    saveAction->setShortcut(QKeySequence("Ctrl+S"));
    fileMenu->addAction(saveAction);
    fileMenu->addSeparator();
    auto* recentMenu = new RoundMenu(tr("Recent documents"), fileMenu);
    recentMenu->addAction(new QAction(tr("Document %1").arg(1), recentMenu));
    recentMenu->addAction(new QAction(tr("Document %1").arg(2), recentMenu));
    fileMenu->addMenu(recentMenu);
    auto* viewMenu = new CheckableMenu(tr("&View"), menuBar);
    menuBar->addMenu(viewMenu);
    auto* previewAction = new QAction(tr("Preview"), viewMenu);
    previewAction->setCheckable(true);
    previewAction->setChecked(true);
    viewMenu->addAction(previewAction);
    auto* unavailable = new QAction(tr("Unavailable command"), viewMenu);
    unavailable->setEnabled(false);
    viewMenu->addAction(unavailable);
    auto* helpAction = new QAction(tr("&Help"), menuBar);
    menuBar->addAction(helpAction);
    connect(menuBar, &MenuBar::triggered, menuStatus, [menuStatus](QAction* action) {
        menuStatus->setText(tr("Command: %1").arg(QString(action->text()).remove('&')));
    });
    auto* menuHint = new BodyLabel(tr("Press F10 to focus the menu bar. Use arrow keys, Enter and "
                                     "Esc to navigate; hover another heading to switch menus."), menuHost);
    menuHint->setWordWrap(true);
    menuLayout->addWidget(menuBar);
    menuLayout->addWidget(menuStatus);
    menuLayout->addWidget(menuHint);
    gallery->addExampleCard(tr("Menu bar"), menuHost, sourcePath(), 1);

    auto* pagesHost = column(760);
    auto* pagesLayout = static_cast<QVBoxLayout*>(pagesHost->layout());
    auto* pages = new RoundTabWidget(pagesHost);
    pages->setFixedSize(640, 240);
    pages->setMovable(true);
    pages->setScrollable(true);
    pages->setTabMaximumWidth(160);
    pages->setCloseButtonDisplayMode(TabCloseButtonDisplayMode::OnHover);
    auto* pageStatus = new BodyLabel(pagesHost);
    const auto refreshPages = [pages, pageStatus]() {
        pageStatus->setText(pages->count()
                                ? tr("Current tab: %1").arg(pages->tabText(pages->currentIndex()))
                                : tr("No pages"));
    };
    const auto addPage = [pages]() {
        const int number = pages->property("nextPageNumber").toInt() + 1;
        pages->setProperty("nextPageNumber", number);
        auto* page = new QWidget;
        auto* layout = new QVBoxLayout(page);
        layout->setContentsMargins(20, 20, 20, 20);
        layout->setSpacing(12);
        auto* title = new BodyLabel(
            tr("Page %1 keeps its input while you switch or reorder tabs.").arg(number), page);
        title->setWordWrap(true);
        auto* input = new LineEdit(page);
        input->setPlaceholderText(tr("Type on this page"));
        input->setText(tr("Document %1").arg(number));
        layout->addWidget(title);
        layout->addWidget(input);
        layout->addStretch();
        const int index =
            pages->addTab(page, tr("Document %1").arg(number), FluentIconEnum::Document);
        pages->setCurrentIndex(index);
    };
    connect(pages, &TabWidget::currentChanged, pageStatus, refreshPages);
    connect(pages, &TabWidget::tabAddRequested, pagesHost, addPage);
    connect(pages, &TabWidget::tabCloseRequested, pagesHost, [pages](int index) {
        auto* page = pages->widget(index);
        pages->removeTab(index);
        if (page) page->deleteLater();  // Demo owns these pages; the library only detaches them.
    });
    for (int i = 0; i < 4; ++i) addPage();
    pages->setCurrentIndex(0);
    refreshPages();
    auto* pageControls = new QWidget(pagesHost);
    auto* pageControlLayout = new QHBoxLayout(pageControls);
    pageControlLayout->setContentsMargins(0, 0, 0, 0);
    auto* addPageButton = new PushButton(tr("Add page"), pageControls);
    auto* autoHide =
        new CheckBox(tr("Hide the tab bar when fewer than two pages remain"), pageControls);
    connect(addPageButton, &QAbstractButton::clicked, pagesHost, addPage);
    connect(autoHide, &QCheckBox::toggled, pages, &RoundTabWidget::setTabBarAutoHide);
    pageControlLayout->addWidget(addPageButton);
    pageControlLayout->addWidget(autoHide);
    pageControlLayout->addStretch();
    auto* pageHint =
        new BodyLabel(tr("Use Ctrl+Tab or Ctrl+Shift+Tab to switch pages. Closing a tab requests "
                         "removal; page data is preserved until the application deletes it."),
                      pagesHost);
    pageHint->setWordWrap(true);
    pagesLayout->addWidget(pages);
    pagesLayout->addWidget(pageStatus);
    pagesLayout->addWidget(pageControls);
    pagesLayout->addWidget(pageHint);
    gallery->addExampleCard(tr("Rounded tab pages"), pagesHost, sourcePath(), 1);

    auto* tabHost = column(760);
    auto* tabLayout = static_cast<QVBoxLayout*>(tabHost->layout());
    auto* tabs = new RoundTabBar(tabHost);
    tabs->setFixedWidth(640);
    tabs->setMovable(true);
    tabs->setScrollable(true);
    tabs->setTabMaximumWidth(160);
    tabs->setCloseButtonDisplayMode(TabCloseButtonDisplayMode::OnHover);
    auto* tabStatus = new BodyLabel(tabHost);
    const auto refreshTabs = [tabs, tabStatus]() {
        tabStatus->setText(tr("Current tab: %1").arg(tabs->tabText(tabs->currentIndex())));
    };
    int* nextTab = new int(0);
    connect(tabHost, &QObject::destroyed, [nextTab]() { delete nextTab; });
    const auto addTab = [tabs, nextTab, refreshTabs]() {
        const int number = ++*nextTab;
        tabs->addTab(QStringLiteral("roundTab%1").arg(number), tr("Document %1").arg(number),
                     FluentIconEnum::Document);
        tabs->setCurrentIndex(tabs->count() - 1);
        refreshTabs();
    };
    for (int i = 0; i < 5; ++i) addTab();
    tabs->setCurrentIndex(0);
    refreshTabs();
    connect(tabs, &TabBar::currentChanged, tabStatus, refreshTabs);
    connect(tabs, &TabBar::tabAddRequested, tabHost, addTab);
    connect(tabs, &TabBar::tabCloseRequested, tabHost, [tabs, refreshTabs](int index) {
        tabs->removeTab(index);
        refreshTabs();
    });
    auto* tabHint = new BodyLabel(
        tr("Drag tabs to reorder, scroll to see more, and use + to add a tab."), tabHost);
    tabHint->setWordWrap(true);
    auto* scrollTabs = new CheckBox(tr("Scrollable tabs"), tabHost);
    scrollTabs->setChecked(true);
    connect(scrollTabs, &QCheckBox::toggled, tabs, &TabBar::setScrollable);
    tabLayout->addWidget(tabs);
    tabLayout->addWidget(tabStatus);
    tabLayout->addWidget(scrollTabs);
    tabLayout->addWidget(tabHint);
    gallery->addExampleCard(tr("Rounded tab bar"), tabHost, sourcePath(), 1);

    const QStringList options = {tr("All"),      tr("Active"),    tr("Completed"), tr("Pending"),
                                 tr("Archived"), tr("Favorites"), tr("Shared"),    tr("Recent")};
    const QList<QPair<QString, ExclusiveLiteFilterBase*>> exclusive = {
        {tr("Exclusive pill filter"), new ExclusiveLiteFilter},
        {tr("Exclusive outlined filter"), new OutlinedExclusiveLiteFilter}};
    for (const auto& example : exclusive) {
        auto* host = column(500);
        auto* layout = static_cast<QVBoxLayout*>(host->layout());
        auto* filter = example.second;
        filter->setFixedWidth(440);
        filter->addItems(options);
        auto* status = new BodyLabel(tr("Selected: %1").arg(filter->currentItem()), host);
        connect(filter, &ExclusiveLiteFilterBase::currentTextChanged, status,
                [status](const QString& selected) {
                    status->setText(tr("Selected: %1").arg(selected));
                });
        auto* hint = new BodyLabel(tr("Scroll horizontally to see more options."), host);
        hint->setWordWrap(true);
        layout->addWidget(filter, 0, Qt::AlignLeft);
        layout->addWidget(status);
        layout->addWidget(hint);
        gallery->addExampleCard(example.first, host, sourcePath(), 1);
    }
    const QList<QPair<QString, MultiSelectionLiteFilterBase*>> multiple = {
        {tr("Multi-selection pill filter"), new MultiSelectionLiteFilter},
        {tr("Multi-selection outlined filter"), new OutlinedMultiSelectionLiteFilter}};
    for (const auto& example : multiple) {
        auto* host = column(500);
        auto* layout = static_cast<QVBoxLayout*>(host->layout());
        auto* filter = example.second;
        filter->setFixedWidth(440);
        filter->addItems(options);
        filter->setCurrentItems({options[1], options[2]});
        auto* status = new BodyLabel(
            tr("Selected: %1").arg(filter->currentItems().join(QStringLiteral(", "))), host);
        status->setWordWrap(true);
        auto* clear = new PushButton(tr("Clear selection"), host);
        connect(clear, &QAbstractButton::clicked, filter,
                &MultiSelectionLiteFilterBase::clearSelection);
        connect(
            filter, &MultiSelectionLiteFilterBase::currentItemsChanged, status,
            [status](const QStringList& selected) {
                status->setText(selected.isEmpty()
                                    ? tr("No options selected")
                                    : tr("Selected: %1").arg(selected.join(QStringLiteral(", "))));
            });
        layout->addWidget(filter, 0, Qt::AlignLeft);
        layout->addWidget(status);
        layout->addWidget(clear, 0, Qt::AlignLeft);
        gallery->addExampleCard(example.first, host, sourcePath(), 1);
    }
    auto* pagerHost = column(800);
    auto* pagerLayout = static_cast<QVBoxLayout*>(pagerHost->layout());
    auto* pager = new Pager(20, 5, pagerHost);
    pager->setCurrentPage(7);
    auto* status = new BodyLabel(tr("Current page: %1").arg(pager->currentPage()), pagerHost);
    auto* controls = new QWidget(pagerHost);
    auto* row = new QHBoxLayout(controls);
    row->setContentsMargins(0, 0, 0, 0);
    auto* count = new SpinBox(controls);
    count->setRange(0, 200);
    count->setValue(pager->pages());
    count->setFixedWidth(120);
    auto* visible = new SpinBox(controls);
    visible->setRange(1, 7);
    visible->setValue(pager->maxVisible());
    visible->setFixedWidth(120);
    row->addWidget(new BodyLabel(tr("Page count"), controls));
    row->addWidget(count);
    row->addWidget(new BodyLabel(tr("Visible pages"), controls));
    row->addWidget(visible);
    row->addStretch(1);
    connect(count, qOverload<int>(&QSpinBox::valueChanged), pager, &Pager::setPages);
    connect(visible, qOverload<int>(&QSpinBox::valueChanged), pager, &Pager::setMaxVisible);
    connect(pager, &Pager::currentPageChanged, status,
            [status](int page) { status->setText(tr("Current page: %1").arg(page)); });
    pagerLayout->addWidget(pager);
    pagerLayout->addWidget(status);
    pagerLayout->addWidget(controls);
    gallery->addExampleCard(tr("Numbered pager with a page jump input"), pagerHost, sourcePath(),
                            1);
}
}  // namespace qfw
