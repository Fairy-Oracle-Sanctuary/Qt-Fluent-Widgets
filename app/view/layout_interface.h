#pragma once

#include "view/gallery_interface.h"

namespace qfw {

class LayoutInterface : public GalleryInterface {
    Q_OBJECT

public:
    explicit LayoutInterface(QWidget* parent = nullptr);

private:
    QWidget* createWidget(bool animation = false);
};

}  // namespace qfw
