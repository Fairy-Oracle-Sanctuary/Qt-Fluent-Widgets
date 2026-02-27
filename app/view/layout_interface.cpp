#include "view/layout_interface.h"

#include <QWidget>

#include "common/translator.h"
#include "components/layout/flow_layout.h"
#include "components/widgets/button.h"

namespace qfw {

LayoutInterface::LayoutInterface(QWidget* parent)
    : GalleryInterface(Translator().layout(),
                       QStringLiteral("qtfluentwidgets.components.layout"), parent) {
    setObjectName(QStringLiteral("layoutInterface"));

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
