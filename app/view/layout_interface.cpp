#include "view/layout_interface.h"
#include "view/pro_container_examples.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QEvent>

#include "common/translator.h"
#include "components/layout/flow_layout.h"
#include "components/widgets/button.h"
#include "components/widgets/card_widget.h"
#include "components/widgets/label.h"
#include "components/widgets/slider.h"
#include "components/widgets/check_box.h"

namespace qfw {
namespace {
class AdaptiveExampleHost : public QWidget {
public:
    AdaptiveExampleHost(bool animation, QWidget* parent) : QWidget(parent) {
        flow = new AdaptiveFlowLayout(this, animation, true);
        flow->setContentsMargins(0, 0, 0, 0);
        flow->setAnimation(250, QEasingCurve::OutQuad);
        setFixedWidth(620);
    }
    AdaptiveFlowLayout* flow = nullptr;
protected:
    bool event(QEvent* event) override {
        const auto type = event->type();
        const bool result = QWidget::event(event);
        if (flow && isVisible() && (type == QEvent::Show || type == QEvent::Resize || type == QEvent::LayoutRequest)) {
            const int desired = qMax(1, flow->heightForWidth(width()));
            if (height() != desired) setFixedHeight(desired);
        }
        return result;
    }
};
} // namespace

LayoutInterface::LayoutInterface(QWidget* parent)
    : GalleryInterface(Translator().layout(),
                       QStringLiteral("qtfluentwidgets.components.layout"), parent) {
    setObjectName(QStringLiteral("layoutInterface"));
    ProContainerExamples::addLayout(this);

    for (bool animation : {false, true}) {
        auto* example = new QWidget(this);
        auto* outer = new QVBoxLayout(example);
        outer->setContentsMargins(0, 0, 0, 0);
        outer->setSpacing(12);
        auto* host = new AdaptiveExampleHost(animation, example);
        SimpleCardWidget* last = nullptr;
        const QStringList titles = {tr("Design"), tr("Development"), tr("Testing"), tr("Documentation"), tr("Release")};
        for (int i = 0; i < titles.size(); ++i) {
            auto* card = new SimpleCardWidget(host);
            card->setFixedHeight(i % 2 ? 110 : 90);
            auto* content = new QVBoxLayout(card);
            content->setContentsMargins(16, 12, 16, 12);
            content->setSpacing(6);
            content->addWidget(new StrongBodyLabel(titles.at(i), card));
            auto* description = new CaptionLabel(tr("Equal width, natural height"), card);
            description->setWordWrap(true);
            content->addWidget(description);
            content->addStretch();
            host->flow->addWidget(card);
            last = card;
        }
        auto* width = new Slider(Qt::Horizontal, example);
        width->setRange(280, 720);
        width->setValue(620);
        width->setMaximumWidth(620);
        width->setAccessibleName(tr("Preview width"));
        auto* widthLabel = new BodyLabel(tr("Preview width: %1 px").arg(620), example);
        auto* cap = new CheckBox(tr("Limit card width to 250 px"), example);
        auto* hideLast = new CheckBox(tr("Hide the last card"), example);
        connect(width, &QSlider::valueChanged, host, [host, widthLabel, this](int value) {
            host->setFixedWidth(value);
            widthLabel->setText(tr("Preview width: %1 px").arg(value));
        });
        connect(cap, &QCheckBox::toggled, host, [host](bool checked) {
            host->flow->setWidgetMaximumWidth(checked ? 250 : -1);
        });
        connect(hideLast, &QCheckBox::toggled, last, [last](bool checked) { last->setVisible(!checked); });
        outer->addWidget(widthLabel);
        outer->addWidget(width);
        outer->addWidget(cap);
        outer->addWidget(hideLast);
        outer->addWidget(host, 0, Qt::AlignLeft);
        addExampleCard(animation ? tr("Adaptive flow layout with animation") : tr("Adaptive flow layout without animation"),
                       example, QStringLiteral("https://github.com/Fairy-Oracle-Sanctuary/PySide6-Fluent-Widgets-Pro"), 1);
    }

    // Flow layout without animation
    addExampleCard(tr("Flow layout without animation"), createWidget(false),
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/"
                                  "examples/layout/flow_layout/demo.py"),
                   1);

    // Flow layout with animation
    addExampleCard(tr("Flow layout with animation"), createWidget(true),
                   QStringLiteral("https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/"
                                  "examples/layout/flow_layout/demo.py"),
                   1);
}

QWidget* LayoutInterface::createWidget(bool animation) {
    const QStringList texts = {
        tr("Star Platinum"),      tr("Hierophant Green"), tr("Silver Chariot"),
        tr("Crazy diamond"),      tr("Heaven's Door"),    tr("Killer Queen"),
        tr("Gold Experience"),    tr("Sticky Fingers"),   tr("Sex Pistols"),
        tr("Dirty Deeds Done Dirt Cheap"),
    };

    auto* widget = new QWidget();
    auto* layout = new FlowLayout(widget, animation);

    layout->setContentsMargins(0, 0, 0, 0);
    layout->setVerticalSpacing(20);
    layout->setHorizontalSpacing(10);

    for (const QString& text : texts) {
        layout->addWidget(new PushButton(text, widget));
    }

    return widget;
}

}  // namespace qfw
