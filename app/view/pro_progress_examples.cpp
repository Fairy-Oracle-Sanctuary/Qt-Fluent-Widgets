#include "view/pro_progress_examples.h"

#include "qtfluentwidgets.h"
#include "view/gallery_interface.h"

namespace qfw {
namespace {
QString sourcePath() {
    return QStringLiteral(
        "https://github.com/Fairy-Oracle-Sanctuary/Qt-Fluent-Widgets/blob/main/app/view/"
        "pro_progress_examples.cpp");
}
QWidget* column(int width) {
    auto* host = new QWidget;
    host->setMaximumWidth(width);
    auto* layout = new QVBoxLayout(host);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);
    return host;
}
}  // namespace
void ProProgressExamples::addStatus(GalleryInterface* gallery) {
    auto* host = column(520);
    auto* layout = static_cast<QVBoxLayout*>(host->layout());
    auto* row = new QHBoxLayout;
    row->setSpacing(40);
    auto* bar = new FilledProgressBar(FluentIcon(FluentIconEnum::Volume), host);
    auto* gauge = new RadialGauge(host);
    gauge->setFixedSize(110, 110);
    auto* progress = new Slider(Qt::Horizontal, host);
    progress->setRange(0, 100);
    progress->setFixedWidth(360);
    auto* paused = new CheckBox(tr("Paused"), host);
    auto* error = new CheckBox(tr("Error"), host);
    row->addWidget(bar, 0, Qt::AlignVCenter);
    row->addWidget(gauge, 0, Qt::AlignVCenter);
    row->addStretch();
    auto* controls = new QHBoxLayout;
    controls->addWidget(paused);
    controls->addWidget(error);
    controls->addStretch();
    connect(progress, &QSlider::valueChanged, bar, &ProgressBar::setValue);
    connect(progress, &QSlider::valueChanged, gauge, &ProgressBar::setValue);
    connect(paused, &QCheckBox::toggled, bar, &ProgressBar::setPaused);
    connect(paused, &QCheckBox::toggled, gauge, &ProgressBar::setPaused);
    connect(error, &QCheckBox::toggled, host, [bar, gauge, paused](bool checked) {
        bar->setError(checked);
        gauge->setError(checked);
        bar->setPaused(paused->isChecked());
        gauge->setPaused(paused->isChecked());
    });
    progress->setValue(65);
    layout->addLayout(row);
    layout->addWidget(new BodyLabel(tr("Progress"), host));
    layout->addWidget(progress, 0, Qt::AlignLeft);
    layout->addLayout(controls);
    gallery->addExampleCard(tr("Filled progress bar and radial gauge"), host, sourcePath(), 1);

    auto* segmentHost = column(520);
    auto* segmentLayout = new QHBoxLayout(segmentHost);
    segmentLayout->setContentsMargins(0, 0, 0, 0);
    segmentLayout->setSpacing(24);
    auto* ring = new MultiSegmentProgressRing(segmentHost);
    ring->setFixedSize(120, 120);
    ring->setStrokeWidth(10);
    ring->setSegments({{45, QColor("#0ea5e9")}, {30, QColor("#22c55e")}, {25, QColor("#f59e0b")}});
    ring->setText(tr("Storage"));
    auto* legend = new BodyLabel(tr("Documents: 45%\nImages: 30%\nOther: 25%"), segmentHost);
    legend->setWordWrap(true);
    segmentLayout->addWidget(ring);
    segmentLayout->addWidget(legend, 1);
    gallery->addExampleCard(tr("Multi-segment progress ring"), segmentHost, sourcePath(), 1);

    auto* stepHost = column(620);
    auto* stepLayout = static_cast<QVBoxLayout*>(stepHost->layout());
    auto* steps = new StepProgressBar(FluentIcon(FluentIconEnum::Accept), 4, stepHost);
    steps->setStepNames({tr("Prepare"), tr("Configure"), tr("Install"), tr("Finish")});
    steps->setCurrent(1);
    auto* status =
        new BodyLabel(tr("Current step: %1").arg(steps->stepName(steps->current())), stepHost);
    auto* buttons = new QHBoxLayout;
    auto* previous = new PushButton(tr("Previous"), stepHost);
    auto* next = new PushButton(tr("Next"), stepHost);
    auto* readOnly = new CheckBox(tr("Non-interactive"), stepHost);
    previous->setFixedWidth(110);
    next->setFixedWidth(110);
    buttons->addWidget(previous);
    buttons->addWidget(next);
    buttons->addWidget(readOnly);
    buttons->addStretch();
    connect(previous, &QAbstractButton::clicked, steps, &StepProgressBar::previousStep);
    connect(next, &QAbstractButton::clicked, steps, &StepProgressBar::nextStep);
    connect(readOnly, &QCheckBox::toggled, steps, &StepProgressBar::setNonInteractive);
    connect(steps, &StepProgressBar::currentChanged, status,
            [steps, status, previous, next](int index) {
                status->setText(tr("Current step: %1").arg(steps->stepName(index)));
                previous->setEnabled(index > 0);
                next->setEnabled(index + 1 < steps->count());
            });
    stepLayout->addWidget(steps);
    stepLayout->addWidget(status);
    stepLayout->addLayout(buttons);
    gallery->addExampleCard(tr("Step progress bar"), stepHost, sourcePath(), 1);
}
}  // namespace qfw
