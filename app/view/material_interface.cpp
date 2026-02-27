#include "view/material_interface.h"

#include <QColor>
#include <QSize>

#include "common/app_config.h"
#include "common/translator.h"
#include "components/widgets/acrylic_label.h"

namespace qfw {

MaterialInterface::MaterialInterface(QWidget* parent)
    : GalleryInterface(Translator().material(), QStringLiteral("qtfluentwidgets.components.widgets"),
                       parent) {
    setObjectName(QStringLiteral("materialInterface"));

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
