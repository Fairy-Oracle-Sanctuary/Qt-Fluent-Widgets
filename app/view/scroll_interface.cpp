#include "view/scroll_interface.h"

#include <QScrollBar>

#include "common/translator.h"
#include "components/widgets/label.h"
#include "components/widgets/pips_pager.h"
#include "components/widgets/scroll_area.h"

namespace qfw {

ScrollInterface::ScrollInterface(QWidget* parent)
    : GalleryInterface(
          []() {
              Translator t;
              return t.scroll();
          }(),
          "qtfluentwidgets.components.widgets", parent) {
    setObjectName("scrollInterface");

    // Scroll area
    auto* scrollArea = new ScrollArea(this);
    auto* label1 = new ImageLabel(":/gallery/images/chidanta2.jpg", this);
    label1->scaledToWidth(775);
    label1->setBorderRadius(8, 8, 8, 8);

    scrollArea->horizontalScrollBar()->setValue(0);
    scrollArea->setWidget(label1);
    scrollArea->setFixedSize(775, 430);

    auto* card1 = addExampleCard(tr("Smooth scroll area"), scrollArea,
                                 "https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/"
                                 "examples/scroll/scroll_area/demo.py");
    card1->setToolTip(tr("Chitanda Eru is too hot 🥵"));
    card1->setToolTipDuration(2000);

    // Smooth scroll area
    auto* smoothScrollArea = new SmoothScrollArea(this);
    auto* label2 = new ImageLabel(":/gallery/images/chidanta3.jpg", this);
    label2->setBorderRadius(8, 8, 8, 8);

    smoothScrollArea->setWidget(label2);
    smoothScrollArea->setFixedSize(660, 540);

    auto* card2 =
        addExampleCard(tr("Smooth scroll area implemented by animation"), smoothScrollArea,
                       "https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/examples/"
                       "scroll/scroll_area/demo.py");
    card2->setToolTip(tr("Chitanda Eru is so hot 🥵🥵"));
    card2->setToolTipDuration(2000);

    // Single direction scroll area
    auto* singleScrollArea = new SingleDirectionScrollArea(this, Qt::Horizontal);
    auto* label3 = new ImageLabel(":/gallery/images/chidanta4.jpg", this);
    label3->setBorderRadius(8, 8, 8, 8);

    singleScrollArea->setWidget(label3);
    singleScrollArea->setFixedSize(660, 498);

    auto* card3 = addExampleCard(tr("Single direction scroll scroll area"), singleScrollArea,
                                 "https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/"
                                 "examples/scroll/scroll_area/demo.py");
    card3->setToolTip(tr("Chitanda Eru is so hot 🥵🥵🥵"));
    card3->setToolTipDuration(2000);

    // Pips pager
    auto* pager = new HorizontalPipsPager(this);
    pager->setPageNumber(15);
    pager->setPreviousButtonDisplayMode(PipsScrollButtonDisplayMode::Always);
    pager->setNextButtonDisplayMode(PipsScrollButtonDisplayMode::Always);

    auto* card4 = addExampleCard(tr("Pips pager"), pager,
                                 "https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/"
                                 "examples/scroll/pips_pager/demo.py");
    // Note: card4->topLayout()->setContentsMargins(12, 20, 12, 20) would require access to internal
    // layout
}

}  // namespace qfw
