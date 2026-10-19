#include "view/pro_container_examples.h"

#include <QPainter>
#include <QPointer>
#include <QSignalBlocker>
#include <QStandardItemModel>
#include <QTimer>
#include <cmath>
#include <memory>

#include "qtfluentwidgets.h"
#include "view/gallery_interface.h"

namespace qfw {
namespace {
class AcrylicSamplingPreview : public QWidget {
public:
    AcrylicSamplingPreview() : brush(this,18,QColor(242,242,242,80),QColor(255,255,255,10),0.03) {
        setFixedSize(480,160);
        QPixmap source(960,320); source.fill(Qt::white);
        QPainter painter(&source);
        for (int x=0;x<960;x+=40)
            painter.fillRect(x,0,20,320,QColor::fromHsv(x*359/960,180,230));
        painter.end(); brush.setImage(source);
    }
    AcrylicBrush brush;
protected:
    void paintEvent(QPaintEvent*) override { QPainter painter(this); brush.paint(&painter); }
};
QString sourcePath() {
    return QStringLiteral(
        "https://github.com/Fairy-Oracle-Sanctuary/Qt-Fluent-Widgets/blob/main/app/view/"
        "pro_container_examples.cpp");
}
QIcon emojiIcon(const QString& text) {
    QPixmap pixmap(64, 64);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    QFont font(QStringLiteral("Segoe UI Emoji"));
    font.setPixelSize(48);
    painter.setFont(font);
    painter.drawText(pixmap.rect(), Qt::AlignCenter, text);
    painter.end();
    return QIcon(pixmap);
}
}  // namespace
void ProContainerExamples::addStatus(GalleryInterface* gallery) {
    auto* managerHost=new QWidget;
    auto* managerLayout=new QVBoxLayout(managerHost);
    managerLayout->setContentsMargins(0,0,0,0); managerLayout->setSpacing(12);
    auto* spacing=new Slider(Qt::Horizontal,managerHost);
    spacing->setRange(16,40); spacing->setValue(24); spacing->setMaximumWidth(300);
    auto* notify=new PushButton(tr("Show managed Toast"),managerHost);
    auto* desktop=new PushButton(tr("Show desktop InfoBar"),managerHost);
    connect(spacing,&QSlider::valueChanged,managerHost,[](int margin) { ToastManager::get(ToastPosition::TopRight)->setMargin(margin); });
    connect(notify,&QPushButton::clicked,gallery,[gallery] { Toast::info(tr("Managed notification"),tr("Adjust the margin slider to realign the stack."),3000,true,ToastPosition::TopRight,Qt::Vertical,gallery->window()); });
    connect(desktop,&QPushButton::clicked,gallery,[] { InfoBar::info(tr("Desktop notification"),tr("This notification uses the transparent desktop container."),Qt::Horizontal,true,3000,InfoBarPosition::TopRight,InfoBar::desktopView()); });
    managerLayout->addWidget(new BodyLabel(tr("Toast margin"),managerHost));
    managerLayout->addWidget(spacing); managerLayout->addWidget(notify,0,Qt::AlignLeft); managerLayout->addWidget(desktop,0,Qt::AlignLeft);
    gallery->addExampleCard(tr("Public notification manager and desktop notifications"),managerHost,sourcePath(),1);
    auto* notifications = new QWidget;
    auto* notificationColumn = new QVBoxLayout(notifications);
    notificationColumn->setContentsMargins(0, 0, 0, 0);
    notificationColumn->setSpacing(12);
    auto* position = new ComboBox(notifications);
    const QStringList positions = {tr("Top"),       tr("Bottom"),      tr("Top left"),
                                   tr("Top right"), tr("Bottom left"), tr("Bottom right")};
    position->addItems(positions);
    position->setCurrentIndex(3);
    position->setFixedWidth(200);
    auto* buttons = new QWidget(notifications);
    auto* flow = new FlowLayout(buttons);
    flow->setContentsMargins(0, 0, 0, 0);
    flow->setHorizontalSpacing(10);
    flow->setVerticalSpacing(10);
    const QStringList kinds = {tr("Information"), tr("Success"), tr("Warning"), tr("Error")};
    for (int kind = 0; kind < kinds.size(); ++kind) {
        auto* button = new PushButton(kinds[kind], buttons);
        flow->addWidget(button);
        connect(button, &QAbstractButton::clicked, gallery, [gallery, position, kind] {
            const auto anchor = static_cast<ToastPosition>(position->currentIndex());
            auto* parent = gallery->window();
            const auto title = tr("Notification");
            const auto content = tr("Click several times to see stacked notifications.");
            if (kind == 0)
                Toast::info(title, content, 3500, true, anchor, Qt::Vertical, parent);
            else if (kind == 1)
                Toast::success(title, content, 3500, true, anchor, Qt::Vertical, parent);
            else if (kind == 2)
                Toast::warning(title, content, 3500, true, anchor, Qt::Vertical, parent);
            else
                Toast::error(title, content, -1, true, anchor, Qt::Vertical, parent);
        });
    }
    auto* notificationHint =
        new BodyLabel(tr("Information follows the theme color. Errors stay open until dismissed."),
                      notifications);
    notificationHint->setWordWrap(true);
    notificationColumn->addWidget(position, 0, Qt::AlignLeft);
    notificationColumn->addWidget(buttons);
    notificationColumn->addWidget(notificationHint);
    notifications->setMaximumWidth(640);
    gallery->addExampleCard(tr("Toast notifications"), notifications, sourcePath(), 1);

    auto progressToast = std::make_shared<QPointer<ProgressToast>>();
    auto loadingToast = std::make_shared<QPointer<RoundProgressToast>>();
    auto* taskHost = new QWidget;
    taskHost->setMaximumWidth(640);
    auto* taskColumn = new QVBoxLayout(taskHost);
    taskColumn->setContentsMargins(0, 0, 0, 0);
    taskColumn->setSpacing(12);
    auto* status = new ComboBox(taskHost);
    status->addItems(kinds);
    status->setCurrentIndex(0);
    status->setFixedWidth(200);
    auto* taskProgress = new Slider(Qt::Horizontal, taskHost);
    taskProgress->setRange(0, 100);
    taskProgress->setValue(35);
    auto* progressText = new BodyLabel(tr("Progress: %1%").arg(35), taskHost);
    auto* taskButtons = new QWidget(taskHost);
    auto* taskRow = new FlowLayout(taskButtons);
    taskRow->setContentsMargins(0, 0, 0, 0);
    taskRow->setHorizontalSpacing(10);
    taskRow->setVerticalSpacing(10);
    auto* showTask = new PushButton(tr("Show progress notification"), taskButtons);
    auto* closeTask = new PushButton(tr("Close notification"), taskButtons);
    taskRow->addWidget(showTask);
    taskRow->addWidget(closeTask);
    connect(showTask, &QAbstractButton::clicked, gallery,
            [gallery, position, status, taskProgress, progressToast] {
                const auto icon = static_cast<InfoBarIconEnum>(status->currentIndex());
                const auto anchor = static_cast<ToastPosition>(position->currentIndex());
                if (progressToast->isNull()) {
                    *progressToast = ProgressToast::newToast(tr("Preparing the release package"),
                                                             icon, taskProgress->value(), -1, true,
                                                             anchor, gallery->window());
                    connect(*progressToast, &Toast::closed, gallery,
                            [progressToast] { *progressToast = nullptr; });
                } else {
                    (*progressToast)->setIcon(icon);
                    (*progressToast)->setValue(taskProgress->value());
                    (*progressToast)->setPosition(anchor);
                }
            });
    connect(taskProgress, &QSlider::valueChanged, progressText,
            [progressText, progressToast](int value) {
                progressText->setText(tr("Progress: %1%").arg(value));
                if (*progressToast) (*progressToast)->setValue(value);
            });
    connect(status, &ComboBox::currentIndexChanged, gallery, [progressToast](int index) {
        if (*progressToast) (*progressToast)->setIcon(static_cast<InfoBarIconEnum>(index));
    });
    connect(closeTask, &QAbstractButton::clicked, gallery, [progressToast] {
        if (*progressToast) (*progressToast)->close();
    });
    auto* taskHint = new BodyLabel(tr("Progress and status are independent. Reaching 100% does not "
                                      "close the notification or change its status."),
                                   taskHost);
    taskHint->setWordWrap(true);
    taskColumn->addWidget(status, 0, Qt::AlignLeft);
    taskColumn->addWidget(taskProgress);
    taskColumn->addWidget(progressText);
    taskColumn->addWidget(taskButtons);
    taskColumn->addWidget(taskHint);
    gallery->addExampleCard(tr("Compact progress notification"), taskHost, sourcePath(), 1);

    auto* loadingHost = new QWidget;
    loadingHost->setMaximumWidth(640);
    auto* loadingColumn = new QVBoxLayout(loadingHost);
    loadingColumn->setContentsMargins(0, 0, 0, 0);
    loadingColumn->setSpacing(12);
    auto* loadingButtons = new QWidget(loadingHost);
    auto* loadingRow = new FlowLayout(loadingButtons);
    loadingRow->setContentsMargins(0, 0, 0, 0);
    loadingRow->setHorizontalSpacing(10);
    loadingRow->setVerticalSpacing(10);
    auto* showLoading = new PushButton(tr("Show loading notification"), loadingButtons);
    auto* closeLoading = new PushButton(tr("Finish loading"), loadingButtons);
    loadingRow->addWidget(showLoading);
    loadingRow->addWidget(closeLoading);
    connect(showLoading, &QAbstractButton::clicked, gallery, [gallery, position, loadingToast] {
        const auto anchor = static_cast<ToastPosition>(position->currentIndex());
        if (loadingToast->isNull()) {
            *loadingToast = RoundProgressToast::newToast(QString(), -1, anchor, gallery->window());
            connect(*loadingToast, &Toast::closed, gallery,
                    [loadingToast] { *loadingToast = nullptr; });
        } else
            (*loadingToast)->setPosition(anchor);
    });
    connect(closeLoading, &QAbstractButton::clicked, gallery, [loadingToast] {
        if (*loadingToast) (*loadingToast)->close();
    });
    auto* loadingHint =
        new BodyLabel(tr("The capsule has no close button. Finish loading to dismiss it; closing a "
                         "notification does not cancel the task."),
                      loadingHost);
    loadingHint->setWordWrap(true);
    loadingColumn->addWidget(loadingButtons);
    loadingColumn->addWidget(loadingHint);
    gallery->addExampleCard(tr("Capsule loading notification"), loadingHost, sourcePath(), 1);

    auto infoTask = std::make_shared<QPointer<ProgressInfoBar>>();
    auto* infoHost = new QWidget;
    infoHost->setMaximumWidth(640);
    auto* infoColumn = new QVBoxLayout(infoHost);
    infoColumn->setContentsMargins(0, 0, 0, 0);
    infoColumn->setSpacing(12);
    auto* indefinite = new CheckBox(tr("Indeterminate progress"), infoHost);
    indefinite->setChecked(true);
    auto* percentage = new Slider(Qt::Horizontal, infoHost);
    percentage->setRange(0, 100);
    percentage->setValue(35);
    percentage->setEnabled(false);
    auto* percentageText = new BodyLabel(tr("Progress: %1%").arg(35), infoHost);
    auto* eta = new LineEdit(infoHost);
    eta->setPlaceholderText(tr("Optional remaining time text"));
    eta->setText(tr("Remaining: 10 s"));
    auto* infoButtons = new QWidget(infoHost);
    auto* infoRow = new FlowLayout(infoButtons);
    infoRow->setContentsMargins(0, 0, 0, 0);
    infoRow->setHorizontalSpacing(10);
    infoRow->setVerticalSpacing(10);
    auto* showInfo = new PushButton(tr("Show task InfoBar"), infoButtons);
    auto* closeInfo = new PushButton(tr("Close notification"), infoButtons);
    infoRow->addWidget(showInfo);
    infoRow->addWidget(closeInfo);
    connect(showInfo, &QAbstractButton::clicked, gallery,
            [gallery, position, infoTask, indefinite, percentage, eta] {
                if (*infoTask) (*infoTask)->close();
                *infoTask = ProgressInfoBar::info(
                    tr("Download"), tr("Preparing the release package"), Qt::Vertical, true, -1,
                    static_cast<InfoBarPosition>(position->currentIndex()), gallery->window());
                (*infoTask)->setRemainingTime(eta->text());
                if (!indefinite->isChecked()) (*infoTask)->setValue(percentage->value());
                connect(*infoTask, &InfoBar::closedSignal, gallery,
                        [infoTask] { *infoTask = nullptr; });
            });
    connect(indefinite, &QCheckBox::toggled, gallery, [infoTask, percentage](bool enabled) {
        percentage->setEnabled(!enabled);
        if (*infoTask) {
            if (enabled)
                (*infoTask)->setIndeterminate(true);
            else
                (*infoTask)->setValue(percentage->value());
        }
    });
    connect(percentage, &QSlider::valueChanged, percentageText,
            [percentageText, infoTask](int value) {
                percentageText->setText(tr("Progress: %1%").arg(value));
                if (*infoTask) (*infoTask)->setValue(value);
            });
    connect(eta, &QLineEdit::textChanged, gallery, [infoTask](const QString& text) {
        if (*infoTask) (*infoTask)->setRemainingTime(text);
    });
    connect(closeInfo, &QAbstractButton::clicked, gallery, [infoTask] {
        if (*infoTask) (*infoTask)->close();
    });
    auto* infoHint = new BodyLabel(
        tr("Set a percentage to switch to determinate progress. Remaining time is caller-provided "
           "text; reaching 100% does not dismiss the task or turn it into a success message."),
        infoHost);
    infoHint->setWordWrap(true);
    infoColumn->addWidget(indefinite);
    infoColumn->addWidget(percentage);
    infoColumn->addWidget(percentageText);
    infoColumn->addWidget(eta);
    infoColumn->addWidget(infoButtons);
    infoColumn->addWidget(infoHint);
    gallery->addExampleCard(tr("Task progress InfoBar"), infoHost, sourcePath(), 1);

    auto* timeline = new TimeLineWidget;
    auto* completed = timeline->addGroup(tr("Completed"), InfoBarIconEnum::Success);
    completed->addItem(tr("<s>Design approved</s>"), InfoBarIconEnum::Success);
    auto* today = timeline->addGroup(tr("Today"));
    auto* build = today->addItem(tr("Build gallery"));
    auto* tests = today->addItem(tr("Run tests"));
    auto* pending = timeline->addGroup(tr("Pending"), InfoBarIconEnum::Error);
    auto* release = pending->addItem(tr("Publish release"), InfoBarIconEnum::Error);
    for (auto* item : {build, tests, release}) {
        connect(item, &CardWidget::clicked, gallery, [gallery, item] {
            Toast::info(tr("Timeline action"), item->text(), 2000, true, ToastPosition::TopRight,
                        Qt::Vertical, gallery->window());
        });
    }
    gallery->addExampleCard(tr("Grouped timeline"), timeline, sourcePath(), 1);

    const QStringList titles = {tr("Article skeleton"), tr("Circular personal info skeleton"),
                                tr("Rectangular personal info skeleton"), tr("Custom skeleton")};
    QList<SkeletonWidget*> skeletons = {new ArticleSkeleton(), new CirclePersonalInfoSkeleton(),
                                        new RectanglePersonalInfoSkeleton(), new SkeletonWidget()};
    auto* custom = skeletons.last();
    custom->addEllipse(QRectF(0, 0, 100, 100));
    custom->addRect(QRectF(120, 10, 400, 30));
    custom->addRect(QRectF(120, 60, 400, 30));
    for (int i = 0; i < skeletons.size(); ++i) {
        auto* skeleton = skeletons.at(i);
        skeleton->setMaximumWidth(skeleton->sizeHint().width());
        skeleton->setFixedHeight(skeleton->sizeHint().height());
        gallery->addExampleCard(titles.at(i), skeleton, sourcePath(), 1);
    }
}
void ProContainerExamples::addText(GalleryInterface* gallery) {
    auto* acrylicHost=new QWidget;
    auto* acrylicLayout=new QVBoxLayout(acrylicHost);
    acrylicLayout->setContentsMargins(0,0,0,0); acrylicLayout->setSpacing(12);
    auto* acrylic=new AcrylicSamplingPreview;
    auto* sample=new CheckBox(tr("Limit blur sampling to 96 x 32"),acrylicHost);
    connect(sample,&QCheckBox::toggled,acrylic,[acrylic](bool checked) { acrylic->brush.setBlurPicSize(checked?QSize(96,32):QSize()); });
    acrylicLayout->addWidget(acrylic); acrylicLayout->addWidget(sample);
    gallery->addExampleCard(tr("Acrylic blur sampling size"),acrylicHost,sourcePath(),1);
    auto* container = new QWidget();
    container->setMaximumWidth(640);
    auto* layout = new QVBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);
    auto* content = new SimpleCardWidget(container);
    content->setMinimumHeight(220);
    auto* body = new QVBoxLayout(content);
    body->setContentsMargins(24, 24, 24, 24);
    body->setSpacing(16);
    body->addWidget(new SubtitleLabel(tr("Watermark over native widgets"), content));
    auto* label = new BodyLabel(tr("The watermark is a transparent overlay. You can still edit "
                                   "text and click the controls below."),
                                content);
    label->setWordWrap(true);
    body->addWidget(label);
    auto* input = new LineEdit(content);
    input->setPlaceholderText(tr("Type here to test mouse passthrough"));
    body->addWidget(input);
    body->addStretch();
    auto* mark = new Watermark(QStringLiteral("Fairy Oracle Sanctuary"), content);
    mark->setOpacity(0.14);
    auto* toggle = new PushButton(tr("Toggle watermark"), container);
    connect(toggle, &QAbstractButton::clicked, mark,
            [mark] { mark->setVisible(mark->isHidden()); });
    layout->addWidget(content);
    layout->addWidget(toggle, 0, Qt::AlignLeft);
    gallery->addExampleCard(tr("Text watermark"), container, sourcePath(), 1);
}
void ProContainerExamples::addView(GalleryInterface* gallery) {
    for (bool line : {false, true}) {
        auto* host = new QWidget;
        auto* column = new QVBoxLayout(host);
        column->setContentsMargins(0, 0, 0, 0);
        column->setSpacing(12);
        auto* mode = new ComboBox(host);
        mode->addItems({tr("Item-based table"), tr("Model-based table")});
        mode->setFixedWidth(220);
        auto* pages = new QStackedWidget(host);
        TableWidget* items = line ? static_cast<TableWidget*>(new LineTableWidget(pages))
                                  : new RoundTableWidget(pages);
        TableView* view =
            line ? static_cast<TableView*>(new LineTableView(pages)) : new RoundTableView(pages);
        const QStringList headers = {tr("Task"), tr("Status"), tr("Progress")};
        const QList<QStringList> rows = {{tr("Compile Gallery"), tr("Completed"), "100%"},
                                         {tr("Run tests"), tr("In progress"), "60%"},
                                         {tr("Publish build"), tr("Pending"), "0%"},
                                         {tr("Review changes"), tr("Pending"), "0%"},
                                         {tr("Update translations"), tr("Completed"), "100%"},
                                         {tr("Check layout"), tr("In progress"), "40%"}};
        items->setColumnCount(3);
        items->setRowCount(rows.size());
        items->setHorizontalHeaderLabels(headers);
        auto* model = new QStandardItemModel(view);
        model->setHorizontalHeaderLabels(headers);
        for (int row = 0; row < rows.size(); ++row) {
            QList<QStandardItem*> cells;
            for (int col = 0; col < 3; ++col) {
                auto* item = new QTableWidgetItem(rows.at(row).at(col));
                item->setFlags(item->flags() & ~Qt::ItemIsEditable);
                items->setItem(row, col, item);
                auto* cell = new QStandardItem(rows.at(row).at(col));
                cell->setEditable(false);
                cells.append(cell);
            }
            model->appendRow(cells);
        }
        view->setModel(model);
        for (auto* table : {static_cast<QTableView*>(items), static_cast<QTableView*>(view)}) {
            table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
            table->setFixedHeight(line ? 292 : 240);
            table->setSelectionMode(QAbstractItemView::ExtendedSelection);
            pages->addWidget(table);
        }
        items->selectRow(1);
        view->selectRow(1);
        connect(mode, qOverload<int>(&ComboBox::currentIndexChanged), pages,
                &QStackedWidget::setCurrentIndex);
        auto* sorting = new CheckBox(tr("Enable table sorting"), host);
        sorting->setVisible(line);
        connect(sorting, &QAbstractButton::toggled, host, [items, view](bool enabled) {
            items->setSortingEnabled(enabled);
            view->setSortingEnabled(enabled);
        });
        auto* selection = new BodyLabel(host);
        selection->setWordWrap(true);
        auto updateSelection = [items, view, pages, selection] {
            auto* current = pages->currentWidget() == items ? static_cast<QTableView*>(items)
                                                            : static_cast<QTableView*>(view);
            QStringList selected;
            for (const auto& index : current->selectionModel()->selectedRows())
                selected.append(index.data().toString());
            selection->setText(tr("Selected tasks: %1").arg(selected.join(QStringLiteral(", "))));
        };
        connect(items->selectionModel(), &QItemSelectionModel::selectionChanged, host,
                updateSelection);
        connect(view->selectionModel(), &QItemSelectionModel::selectionChanged, host,
                updateSelection);
        connect(pages, &QStackedWidget::currentChanged, host, updateSelection);
        updateSelection();
        auto* hint =
            new BodyLabel(tr("Select rows with Ctrl / Shift. Switch between item and model "
                             "tables to compare the same layout."),
                          host);
        hint->setWordWrap(true);
        column->addWidget(mode);
        column->addWidget(pages);
        column->addWidget(selection);
        if (line) column->addWidget(sorting);
        column->addWidget(hint);
        host->setMaximumWidth(720);
        gallery->addExampleCard(line ? tr("Line tables") : tr("Round tables"), host, sourcePath(),
                                1);
    }

    const QStringList listTitles = {tr("Round lists"), tr("Transparent round lists"),
                                    tr("Category card lists")};
    const QStringList listItems = {tr("Documents"), tr("Pictures"),  tr("Music"),
                                   tr("Videos"),    tr("Downloads"), tr("Favorites")};
    const QList<FluentIconEnum> listIcons = {FluentIconEnum::Document, FluentIconEnum::Photo,
                                             FluentIconEnum::Music,    FluentIconEnum::Video,
                                             FluentIconEnum::Download, FluentIconEnum::Heart};
    for (int variant = 0; variant < 3; ++variant) {
        auto* host = new QWidget;
        auto* column = new QVBoxLayout(host);
        column->setContentsMargins(0, 0, 0, 0);
        column->setSpacing(12);
        auto* row = new QHBoxLayout;
        row->setSpacing(16);
        RoundListWidget* items =
            variant == 0   ? new RoundListWidget(host)
            : variant == 1 ? static_cast<RoundListWidget*>(new TransparentRoundListWidget(host))
                           : new CategoryCardListWidget(host);
        RoundListView* view = variant == 0 ? new RoundListView(host)
                              : variant == 1
                                  ? static_cast<RoundListView*>(new TransparentRoundListView(host))
                                  : new CategoryCardListView(host);
        auto* model = new QStandardItemModel(view);
        for (int i = 0; i < listItems.size(); ++i) {
            items->addItem(listItems.at(i), listIcons.at(i));
            auto* item = new QStandardItem(
                RoundListItemDelegate::themedIcon(FluentIcon(listIcons.at(i))), listItems.at(i));
            item->setEditable(false);
            model->appendRow(item);
        }
        view->setModel(model);
        for (auto* list : {static_cast<QListView*>(items), static_cast<QListView*>(view)}) {
            list->setFixedHeight(202);
            list->setMinimumWidth(180);
            list->setCurrentIndex(list->model()->index(0, 0));
            auto* section = new QVBoxLayout;
            section->setSpacing(8);
            section->addWidget(new BodyLabel(
                list == items ? tr("Item-based list") : tr("Model-based list"), host));
            section->addWidget(list);
            row->addLayout(section, 1);
        }
        column->addLayout(row);
        auto* selection = new BodyLabel(host);
        selection->setWordWrap(true);
        auto updateSelection = [items, view, selection] {
            auto names = [](QListView* list) {
                QStringList result;
                for (const auto& index : list->selectionModel()->selectedIndexes())
                    result.append(index.data().toString());
                return result.join(QStringLiteral(", "));
            };
            selection->setText(tr("Items: %1; model: %2").arg(names(items), names(view)));
        };
        connect(items->selectionModel(), &QItemSelectionModel::selectionChanged, host,
                updateSelection);
        connect(view->selectionModel(), &QItemSelectionModel::selectionChanged, host,
                updateSelection);
        updateSelection();
        column->addWidget(selection);
        auto* multi = new CheckBox(tr("Enable multiple selection (Ctrl / Shift)"), host);
        connect(multi, &QAbstractButton::toggled, host, [items, view](bool enabled) {
            const auto mode =
                enabled ? QAbstractItemView::ExtendedSelection : QAbstractItemView::SingleSelection;
            items->setSelectionMode(mode);
            view->setSelectionMode(mode);
        });
        column->addWidget(multi);
        host->setMaximumWidth(640);
        gallery->addExampleCard(listTitles.at(variant), host, sourcePath(), 1);
    }

    auto* waveformHost = new QWidget;
    auto* waveformColumn = new QVBoxLayout(waveformHost);
    waveformColumn->setContentsMargins(0, 0, 0, 0);
    waveformColumn->setSpacing(12);
    auto* waveform = new AudioWaveformWidget(waveformHost);
    waveform->setFixedHeight(160);
    auto makeSamples = [](int offset, int count) {
        QVector<float> samples;
        samples.reserve(count);
        for (int i = offset; i < offset + count; ++i) {
            const auto envelope = .2 + .65 * std::pow(std::sin(i * .0017), 2);
            samples.append(float(envelope * std::sin(i * .16)));
        }
        return samples;
    };
    waveform->setSamples(makeSamples(0, 6000), 1000);
    auto* waveformPosition = new Slider(Qt::Horizontal, waveformHost);
    waveformPosition->setRange(0, int(waveform->duration()));
    auto* waveformTime = new BodyLabel(waveformHost);
    auto updateTime = [waveform, waveformTime] {
        waveformTime->setText(
            tr("Position: %1 / %2 ms").arg(waveform->position()).arg(waveform->duration()));
    };
    connect(waveformPosition, &QSlider::valueChanged, waveform, &AudioWaveformWidget::setPosition);
    connect(waveform, &AudioWaveformWidget::positionChanged, waveformPosition,
            [waveformPosition, updateTime](qint64 position) {
                QSignalBlocker blocker(waveformPosition);
                waveformPosition->setValue(int(position));
                updateTime();
            });
    connect(waveform, &AudioWaveformWidget::durationChanged, waveformPosition,
            [waveformPosition, updateTime](qint64 duration) {
                waveformPosition->setRange(0, int(duration));
                updateTime();
            });
    auto* waveformActions = new QWidget(waveformHost);
    auto* waveformFlow = new FlowLayout(waveformActions);
    waveformFlow->setContentsMargins(0, 0, 0, 0);
    waveformFlow->setHorizontalSpacing(10);
    waveformFlow->setVerticalSpacing(10);
    auto* simulate = new PushButton(tr("Simulate progress"), waveformActions);
    simulate->setCheckable(true);
    auto* append = new PushButton(tr("Append 2 seconds"), waveformActions);
    auto* reset = new PushButton(tr("Reset waveform"), waveformActions);
    auto* clear = new PushButton(tr("Clear waveform"), waveformActions);
    for (auto* button : {simulate, append, reset, clear}) waveformFlow->addWidget(button);
    auto* progressTimer = new QTimer(waveformHost);
    progressTimer->setInterval(50);
    connect(simulate, &QAbstractButton::toggled, waveform,
            [waveform, simulate, progressTimer](bool checked) {
                simulate->setText(checked ? tr("Pause progress") : tr("Simulate progress"));
                if (checked && waveform->sampleCount()) {
                    if (waveform->position() == waveform->duration()) waveform->setPosition(0);
                    progressTimer->start();
                } else {
                    progressTimer->stop();
                    if (checked) simulate->setChecked(false);
                }
            });
    connect(progressTimer, &QTimer::timeout, waveform, [waveform, simulate, gallery] {
        if (!gallery->isVisible()) {
            simulate->setChecked(false);
            return;
        }
        waveform->setPosition(waveform->position() + 50);
        if (waveform->position() == waveform->duration()) simulate->setChecked(false);
    });
    connect(append, &QAbstractButton::clicked, waveform, [waveform, makeSamples] {
        waveform->appendSamples(makeSamples(waveform->sampleCount(), 2000));
    });
    connect(reset, &QAbstractButton::clicked, waveform, [waveform, simulate, makeSamples] {
        simulate->setChecked(false);
        waveform->setSamples(makeSamples(0, 6000), 1000);
    });
    connect(clear, &QAbstractButton::clicked, waveform, [waveform, simulate] {
        simulate->setChecked(false);
        waveform->clear();
    });
    auto* allowSeek = new CheckBox(tr("Allow seeking"), waveformHost);
    allowSeek->setChecked(true);
    connect(allowSeek, &QAbstractButton::toggled, waveform, &AudioWaveformWidget::setSeekEnabled);
    auto* waveformHint = new BodyLabel(
        tr("Click or drag the waveform to seek. Arrow keys move by one second; Home/End jump to "
           "the boundaries. This demo displays samples only and does not play audio."),
        waveformHost);
    waveformHint->setWordWrap(true);
    updateTime();
    waveform->setPosition(2000);
    waveformColumn->addWidget(waveform);
    waveformColumn->addWidget(waveformPosition);
    waveformColumn->addWidget(waveformTime);
    waveformColumn->addWidget(waveformActions);
    waveformColumn->addWidget(allowSeek);
    waveformColumn->addWidget(waveformHint);
    waveformHost->setMaximumWidth(640);
    gallery->addExampleCard(tr("Audio waveform"), waveformHost, sourcePath(), 1);

    const QImage original(QStringLiteral(":/gallery/images/SBR.jpg"));
    auto* cropHost = new QWidget;
    auto* cropColumn = new QVBoxLayout(cropHost);
    cropColumn->setContentsMargins(0, 0, 0, 0);
    cropColumn->setSpacing(12);
    auto* shape = new ComboBox(cropHost);
    shape->addItems({tr("Rectangle"), tr("Circle"), tr("Rounded rectangle"), tr("Diamond")});
    shape->setFixedWidth(220);
    auto* showCrop = new PushButton(tr("Open image cropper"), cropHost);
    auto* preview = new ImageLabel(original, cropHost);
    preview->setScaledSize(original.size().scaled(QSize(200, 160), Qt::KeepAspectRatio));
    auto* cropResult = new BodyLabel(tr("No crop confirmed"), cropHost);
    auto* cropHint =
        new BodyLabel(tr("Drag the selection or its four handles. Rotate and flip use the source "
                         "image. Confirm returns an image; nothing is saved automatically."),
                      cropHost);
    cropHint->setWordWrap(true);
    connect(showCrop, &QAbstractButton::clicked, gallery,
            [gallery, shape, preview, cropResult, original] {
                auto* editor = new ImageCropper(original, gallery->window());
                if (shape->currentIndex() == 1)
                    editor->setCropShape(CropShape::CIRCLE);
                else if (shape->currentIndex() == 2)
                    editor->setCropPathFactory([](const QRectF& rect) {
                        QPainterPath path;
                        const auto radius = qMin(rect.width(), rect.height()) * .2;
                        path.addRoundedRect(rect, radius, radius);
                        return path;
                    });
                else if (shape->currentIndex() == 3)
                    editor->setCropPathFactory([](const QRectF& rect) {
                        QPainterPath path;
                        path.moveTo(rect.center().x(), rect.top());
                        path.lineTo(rect.right(), rect.center().y());
                        path.lineTo(rect.center().x(), rect.bottom());
                        path.lineTo(rect.left(), rect.center().y());
                        path.closeSubpath();
                        return path;
                    });
                connect(
                    editor, &ImageCropper::imageCropped, preview,
                    [preview, cropResult](const QImage& image) {
                        preview->setImage(image);
                        preview->setScaledSize(
                            image.size().scaled(QSize(200, 160), Qt::KeepAspectRatio));
                        cropResult->setText(
                            tr("Cropped image: %1 × %2 px").arg(image.width()).arg(image.height()));
                    });
                connect(editor, &QDialog::finished, editor, &QObject::deleteLater);
                editor->show();
            });
    cropColumn->addWidget(shape, 0, Qt::AlignLeft);
    cropColumn->addWidget(showCrop, 0, Qt::AlignLeft);
    cropColumn->addWidget(preview, 0, Qt::AlignLeft);
    cropColumn->addWidget(cropResult);
    cropColumn->addWidget(cropHint);
    cropHost->setMaximumWidth(640);
    gallery->addExampleCard(tr("Image cropper"), cropHost, sourcePath(), 1);

    auto* avatarHost = new QWidget;
    auto* avatarColumn = new QVBoxLayout(avatarHost);
    avatarColumn->setContentsMargins(0, 0, 0, 0);
    avatarColumn->setSpacing(12);
    auto* avatar = new AvatarPicker(original, avatarHost);
    auto* cropAvatar = new PushButton(tr("Crop sample avatar"), avatarHost);
    auto* avatarResult = new BodyLabel(tr("Avatar has not been changed"), avatarHost);
    auto* avatarHint =
        new BodyLabel(tr("Click the avatar or press Space/Enter to choose a local image, then "
                         "confirm its circular crop. Cancel leaves the current avatar unchanged."),
                      avatarHost);
    avatarHint->setWordWrap(true);
    connect(cropAvatar, &QAbstractButton::clicked, avatar,
            [avatar, original] { avatar->cropImage(original); });
    connect(avatar, &AvatarPicker::imageChanged, avatarResult, [avatarResult](const QImage& image) {
        avatarResult->setText(
            tr("Avatar updated: %1 × %2 px").arg(image.width()).arg(image.height()));
    });
    avatarColumn->addWidget(avatar, 0, Qt::AlignLeft);
    avatarColumn->addWidget(cropAvatar, 0, Qt::AlignLeft);
    avatarColumn->addWidget(avatarResult);
    avatarColumn->addWidget(avatarHint);
    avatarHost->setMaximumWidth(640);
    gallery->addExampleCard(tr("Avatar picker"), avatarHost, sourcePath(), 1);

    const QImage grayscale = original.convertToFormat(QImage::Format_Grayscale8);
    auto* comparisonHost = new QWidget;
    auto* comparisonColumn = new QVBoxLayout(comparisonHost);
    comparisonColumn->setContentsMargins(0, 0, 0, 0);
    comparisonColumn->setSpacing(12);
    auto* comparison = new ImageComparisonSlider(grayscale, original, comparisonHost);
    comparison->scaledToWidth(420);
    auto* divider = new Slider(Qt::Horizontal, comparisonHost);
    divider->setRange(0, 100);
    divider->setValue(50);
    divider->setFixedWidth(420);
    auto* fraction = new BodyLabel(tr("Before image: %1%").arg(50), comparisonHost);
    auto* comparisonHint =
        new BodyLabel(tr("Drag the divider to compare grayscale and color. Left/Right adjusts 1%; "
                         "Home/End moves to either edge."),
                      comparisonHost);
    comparisonHint->setWordWrap(true);
    connect(divider, &QSlider::valueChanged, comparison, &ImageComparisonSlider::setValue);
    connect(comparison, &ImageComparisonSlider::valueChanged, fraction,
            [divider, fraction](qreal value) {
                const QSignalBlocker blocker(divider);
                divider->setValue(qRound(value));
                fraction->setText(tr("Before image: %1%").arg(value, 0, 'f', 1));
            });
    comparisonColumn->addWidget(comparison, 0, Qt::AlignLeft);
    comparisonColumn->addWidget(divider, 0, Qt::AlignLeft);
    comparisonColumn->addWidget(fraction);
    comparisonColumn->addWidget(comparisonHint);
    comparisonHost->setMaximumWidth(640);
    gallery->addExampleCard(tr("Image comparison slider"), comparisonHost, sourcePath(), 1);

    auto* magnifierHost = new QWidget;
    auto* magnifierColumn = new QVBoxLayout(magnifierHost);
    magnifierColumn->setContentsMargins(0, 0, 0, 0);
    magnifierColumn->setSpacing(12);
    auto* magnifier = new ImageMagnifierWidget(original, magnifierHost);
    magnifier->scaledToWidth(420);
    magnifier->setBorderRadius(8, 8, 8, 8);
    auto* controls = new QWidget(magnifierHost);
    auto* flow = new FlowLayout(controls);
    flow->setContentsMargins(0, 0, 0, 0);
    flow->setHorizontalSpacing(12);
    flow->setVerticalSpacing(8);
    auto* enabled = new CheckBox(tr("Enable magnifier"), controls);
    enabled->setChecked(true);
    auto* factor = new DoubleSpinBox(controls);
    factor->setRange(1, 8);
    factor->setSingleStep(0.5);
    factor->setValue(2);
    factor->setFixedWidth(140);
    factor->setAccessibleName(tr("Magnification"));
    auto* radius = new SpinBox(controls);
    radius->setRange(8, 100);
    radius->setValue(50);
    radius->setFixedWidth(140);
    radius->setAccessibleName(tr("Lens radius"));
    flow->addWidget(enabled);
    flow->addWidget(new BodyLabel(tr("Magnification"), controls));
    flow->addWidget(factor);
    flow->addWidget(new BodyLabel(tr("Lens radius"), controls));
    flow->addWidget(radius);
    connect(enabled, &QAbstractButton::toggled, magnifier,
            &ImageMagnifierWidget::setMagnifierEnabled);
    connect(factor, QOverload<double>::of(&QDoubleSpinBox::valueChanged), magnifier,
            &ImageMagnifierWidget::setMagnification);
    connect(radius, QOverload<int>::of(&QSpinBox::valueChanged), magnifier,
            &ImageMagnifierWidget::setRadius);
    auto* magnifierHint =
        new BodyLabel(tr("Move over the image to inspect source details. Tab to focus the image, "
                         "then use arrow keys to move the lens."),
                      magnifierHost);
    magnifierHint->setWordWrap(true);
    magnifierColumn->addWidget(magnifier, 0, Qt::AlignLeft);
    magnifierColumn->addWidget(controls);
    magnifierColumn->addWidget(magnifierHint);
    magnifierHost->setMaximumWidth(640);
    gallery->addExampleCard(tr("Image magnifier"), magnifierHost, sourcePath(), 1);
}
void ProContainerExamples::addDialogs(GalleryInterface* gallery) {
    auto* confirmationHost = new QWidget;
    auto* confirmationColumn = new QVBoxLayout(confirmationHost);
    confirmationColumn->setContentsMargins(0, 0, 0, 0);
    confirmationColumn->setSpacing(12);
    auto* confirmationActions = new QWidget(confirmationHost);
    auto* confirmationFlow = new FlowLayout(confirmationActions);
    confirmationFlow->setContentsMargins(0, 0, 0, 0);
    confirmationFlow->setHorizontalSpacing(10);
    confirmationFlow->setVerticalSpacing(10);
    auto* showConfirmation = new PushButton(tr("Show confirmation flyout"), confirmationActions);
    auto* showInput = new PushButton(tr("Show input flyout"), confirmationActions);
    confirmationFlow->addWidget(showConfirmation);
    confirmationFlow->addWidget(showInput);
    auto* confirmationResult = new BodyLabel(tr("No action confirmed"), confirmationHost);
    auto activeConfirmation = std::make_shared<QPointer<Flyout>>();
    for (auto* button : {showConfirmation, showInput}) {
        connect(
            button, &QAbstractButton::clicked, gallery,
            [gallery, button, showInput, confirmationResult, activeConfirmation] {
                if (*activeConfirmation) {
                    (*activeConfirmation)->raise();
                    return;
                }
                const bool withInput = button == showInput;
                auto* dialog =
                    new FlyoutDialog(withInput ? tr("Save profile") : tr("Apply changes?"),
                                     withInput ? tr("Enter a profile name. Confirm returns your "
                                                    "input; this example does not write files.")
                                               : tr("Confirm applies this demo action. Cancel or "
                                                    "click outside to leave it unchanged."));
                if (withInput) {
                    auto* name = new LineEdit(dialog);
                    name->setPlaceholderText(tr("Profile name"));
                    name->setText(tr("My profile"));
                    name->setMinimumWidth(220);
                    dialog->addWidget(name, 0, Qt::Alignment());
                    connect(dialog, &FlyoutDialog::accepted, confirmationResult,
                            [confirmationResult, name] {
                                confirmationResult->setText(
                                    tr("Confirmed profile: %1").arg(name->text()));
                            });
                } else {
                    connect(dialog, &FlyoutDialog::accepted, confirmationResult,
                            [confirmationResult] {
                                confirmationResult->setText(tr("Action confirmed"));
                            });
                }
                connect(dialog, &FlyoutDialog::rejected, confirmationResult, [confirmationResult] {
                    confirmationResult->setText(tr("Cancelled; no changes applied"));
                });
                *activeConfirmation = dialog->showAt(button, gallery->window());
            });
    }
    auto* confirmationHint = new BodyLabel(
        tr("Confirm with Enter, cancel with Esc, or click outside the content (including its "
           "shadow). Custom widgets can be added above the compact footer."),
        confirmationHost);
    confirmationHint->setWordWrap(true);
    confirmationColumn->addWidget(confirmationActions);
    confirmationColumn->addWidget(confirmationResult);
    confirmationColumn->addWidget(confirmationHint);
    confirmationHost->setMaximumWidth(640);
    gallery->addExampleCard(tr("Compact confirmation flyout"), confirmationHost, sourcePath(), 1);

    auto* waitingHost = new QWidget;
    waitingHost->setMaximumWidth(640);
    auto* waitingColumn = new QVBoxLayout(waitingHost);
    waitingColumn->setContentsMargins(0, 0, 0, 0);
    waitingColumn->setSpacing(12);
    auto* automatic = new CheckBox(tr("Complete automatically after 3 seconds"), waitingHost);
    automatic->setChecked(true);
    auto* showWaiting = new PushButton(tr("Show waiting dialog"), waitingHost);
    auto* waitingResult = new BodyLabel(tr("Waiting task has not started"), waitingHost);
    auto* waitingHint = new BodyLabel(
        tr("Esc dismisses the window-modal dialog. Connect rejected() to your worker's "
           "cancellation handler if closing should cancel the task."),
        waitingHost);
    waitingHint->setWordWrap(true);
    connect(showWaiting, &QAbstractButton::clicked, gallery, [gallery, automatic, waitingResult] {
        auto* dialog = new WaitingDialog(gallery->window());
        dialog->setContent(tr("Preparing download..."));
        connect(dialog, &QDialog::accepted, waitingResult,
                [waitingResult] { waitingResult->setText(tr("Waiting task completed")); });
        connect(dialog, &QDialog::rejected, waitingResult,
                [waitingResult] { waitingResult->setText(tr("Waiting dialog dismissed")); });
        connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater);
        if (automatic->isChecked()) QTimer::singleShot(3000, dialog, &QDialog::accept);
        dialog->open();
    });
    waitingColumn->addWidget(automatic);
    waitingColumn->addWidget(showWaiting, 0, Qt::AlignLeft);
    waitingColumn->addWidget(waitingResult);
    waitingColumn->addWidget(waitingHint);
    gallery->addExampleCard(tr("Compact waiting dialog"), waitingHost, sourcePath(), 1);

    auto* container = new QWidget();
    auto* layout = new FlowLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setHorizontalSpacing(12);
    layout->setVerticalSpacing(12);
    const QStringList directions = {tr("Left"), tr("Right"), tr("Top"), tr("Bottom")};
    const DrawerPosition positions[] = {DrawerPosition::LEFT, DrawerPosition::RIGHT,
                                        DrawerPosition::TOP, DrawerPosition::BOTTOM};
    auto drawer = std::make_shared<QPointer<Drawer>>();
    for (int i = 0; i < directions.size(); ++i) {
        auto* button = new PushButton(directions.at(i), container);
        layout->addWidget(button);
        const auto position = positions[i];
        connect(button, &QAbstractButton::clicked, gallery, [gallery, drawer, position] {
            if (drawer->isNull()) {
                QWidget* target = gallery->parentWidget() ? gallery->parentWidget() : gallery;
                *drawer = new Drawer(tr("Notifications"), target);
                auto* label = new BodyLabel(tr("No more notifications"), *drawer);
                label->setAlignment(Qt::AlignCenter);
                (*drawer)->addWidget(label, 1);
            }
            (*drawer)->open(position);
        });
    }
    gallery->addExampleCard(tr("Drawer from four directions"), container, sourcePath(), 1);
}
void ProContainerExamples::addLayout(GalleryInterface* gallery) {
    auto* wrapHost=new QWidget;
    auto* wrapLayout=new QVBoxLayout(wrapHost);
    wrapLayout->setContentsMargins(0,0,0,0); wrapLayout->setSpacing(12);
    auto* groups=new GroupHeaderCardWidget(tr("Description wrapping"),wrapHost);
    auto* group=groups->addGroup(FluentIcon(FluentIconEnum::Info),tr("Per-group wrapping"),
        tr("Long descriptions wrap within the available card width and shrink again when more space is available."),new PushButton(tr("Apply")),0,true);
    auto* wrap=new CheckBox(tr("Wrap this group description"),wrapHost); wrap->setChecked(true);
    connect(wrap,&QCheckBox::toggled,group,&CardGroupWidget::setContentWordWrap);
    wrapLayout->addWidget(groups); wrapLayout->addWidget(wrap);
    gallery->addExampleCard(tr("Group card with configurable word wrap"),wrapHost,sourcePath(),1);
    auto* splitter = new Splitter(Qt::Horizontal);
    splitter->setMaximumWidth(640);
    splitter->setFixedHeight(180);
    splitter->setChildrenCollapsible(false);
    for (const auto& title : {tr("Left"), tr("Right")}) {
        auto* page = new SimpleCardWidget;
        page->setMinimumWidth(100);
        auto* content = new QVBoxLayout(page);
        content->setContentsMargins(16, 16, 16, 16);
        content->addWidget(new SubtitleLabel(title, page));
        auto* edit = new LineEdit(page);
        edit->setPlaceholderText(tr("Drag the divider to resize the panels"));
        content->addWidget(edit);
        content->addStretch();
        splitter->addWidget(page);
    }
    splitter->setSizes({300, 300});
    gallery->addExampleCard(tr("Fluent splitter"), splitter, sourcePath(), 1);

    auto* waterfallHost = new QWidget;
    waterfallHost->setMaximumWidth(640);
    auto* waterfallColumn = new QVBoxLayout(waterfallHost);
    waterfallColumn->setContentsMargins(0, 0, 0, 0);
    waterfallColumn->setSpacing(12);
    auto* waterfallArea = new ScrollArea(waterfallHost);
    waterfallArea->setWidgetResizable(true);
    waterfallArea->setFixedHeight(400);
    waterfallArea->setFrameShape(QFrame::NoFrame);
    waterfallArea->enableTransparentBackground();
    auto* stage = new QWidget;
    auto* waterfall = new WaterfallLayout(stage, true, true);
    waterfall->setContentsMargins(4, 4, 4, 4);
    waterfall->setColumnWidth(160);
    waterfall->setVerticalSpacing(12);
    waterfall->setHorizontalSpacing(12);
    waterfall->setAnimation(220, QEasingCurve::OutCubic);
    const int heights[] = {100, 150, 80, 120, 110, 90, 140, 100};
    QPointer<SimpleCardWidget> lastCard;
    for (int i = 0; i < 8; ++i) {
        auto* card = new SimpleCardWidget(stage);
        card->setFixedHeight(heights[i]);
        auto* content = new QVBoxLayout(card);
        content->setContentsMargins(16, 12, 16, 12);
        content->addWidget(new SubtitleLabel(tr("Card %1").arg(i + 1), card));
        auto* hint = new BodyLabel(tr("Height: %1 px").arg(heights[i]), card);
        hint->setWordWrap(true);
        content->addWidget(hint);
        content->addStretch();
        waterfall->addWidget(card);
        lastCard = card;
    }
    waterfallArea->setWidget(stage);
    auto* fixedWidth = new CheckBox(tr("Use fixed 160 px columns"), waterfallHost);
    fixedWidth->setChecked(true);
    auto* toggleCard = new PushButton(tr("Show / hide the last card"), waterfallHost);
    connect(fixedWidth, &QCheckBox::toggled, stage, [waterfall](bool fixed) {
        waterfall->setColumnWidth(fixed ? 160 : 0);
        waterfall->setColumnCount(3);
    });
    connect(toggleCard, &QAbstractButton::clicked, stage, [lastCard] {
        if (lastCard) lastCard->setVisible(lastCard->isHidden());
    });
    waterfallColumn->addWidget(waterfallArea);
    waterfallColumn->addWidget(fixedWidth);
    waterfallColumn->addWidget(toggleCard, 0, Qt::AlignLeft);
    gallery->addExampleCard(tr("Animated waterfall layout"), waterfallHost, sourcePath(), 1);

    auto* dashboard =
        new DashboardCardWidget(emojiIcon(QString::fromUtf8("🔓")), tr("File Locksmith"),
                                tr("Right-click a file or folder to show the processes using it."));
    dashboard->setMaximumWidth(400);
    dashboard->setChecked(true);
    dashboard->setCardBackgroundColor(QColor("#fff7e8"), QColor("#34312a"));
    gallery->addExampleCard(tr("Dashboard card"), dashboard, sourcePath(), 1);
    auto* custom =
        new DashboardCardWidget(emojiIcon(QString::fromUtf8("🌐")), tr("Hosts file editor"));
    custom->setMaximumWidth(400);
    custom->setCardBackgroundColor(QColor("#edf7fa"), QColor("#282e30"));
    auto* open = new PushButton(tr("Open file editor"), custom);
    custom->addWidget(open);
    connect(open, &QAbstractButton::clicked, custom,
            [custom] { custom->setContent(tr("Connect this button to your own application.")); });
    gallery->addExampleCard(tr("Custom dashboard card"), custom, sourcePath(), 1);

    auto* container = new QWidget();
    container->setMaximumWidth(420);
    auto* layout = new QVBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(16);
    auto* toolbox = new ToolBox(container);
    auto* blend = new QWidget();
    auto* blendLayout = new QVBoxLayout(blend);
    blendLayout->setContentsMargins(0, 0, 0, 0);
    blendLayout->setSpacing(12);
    blendLayout->addWidget(new BodyLabel(tr("Blend mode"), blend));
    auto* mode = new ComboBox(blend);
    mode->addItems({tr("Normal"), tr("Multiply"), tr("Screen")});
    mode->setCurrentIndex(0);
    mode->setFixedWidth(180);
    blendLayout->addWidget(mode, 0, Qt::AlignLeft);
    blendLayout->addWidget(new BodyLabel(tr("Opacity"), blend));
    auto* opacity = new Slider(Qt::Horizontal, blend);
    opacity->setRange(0, 100);
    opacity->setValue(30);
    blendLayout->addWidget(opacity);
    auto* buttons = new QHBoxLayout();
    auto* apply = new PrimaryPushButton(tr("Apply"), blend);
    auto* reset = new PushButton(tr("Reset"), blend);
    buttons->addWidget(apply);
    buttons->addWidget(reset);
    blendLayout->addLayout(buttons);
    toolbox->addItem(blend, tr("Blend"));
    const QStringList names = {tr("Hue"), tr("Sharpen")};
    for (int i = 0; i < names.size(); ++i) {
        auto* page = new QWidget();
        auto* pageLayout = new QVBoxLayout(page);
        pageLayout->setContentsMargins(0, 0, 0, 0);
        pageLayout->addWidget(new BodyLabel(names.at(i), page));
        auto* slider = new Slider(Qt::Horizontal, page);
        slider->setRange(i == 0 ? -180 : 0, i == 0 ? 180 : 100);
        slider->setValue(i == 0 ? 0 : 20);
        pageLayout->addWidget(slider);
        toolbox->addItem(page, names.at(i));
    }
    auto* status = new BodyLabel(tr("Only one tool can be expanded at a time."), container);
    status->setWordWrap(true);
    connect(toolbox, &ToolBox::currentChanged, status, [status, toolbox](int index) {
        status->setText(index < 0 ? tr("All tools are collapsed.")
                                  : tr("Current tool: %1").arg(toolbox->itemText(index)));
    });
    connect(reset, &QAbstractButton::clicked, opacity, [opacity, mode] {
        mode->setCurrentIndex(0);
        opacity->setValue(30);
    });
    connect(apply, &QAbstractButton::clicked, status,
            [status, opacity] { status->setText(tr("Opacity: %1%").arg(opacity->value())); });
    layout->addWidget(toolbox);
    layout->addWidget(status);
    gallery->addExampleCard(tr("ToolBox — exclusive animated panels"), container, sourcePath(), 1);
}
}  // namespace qfw
