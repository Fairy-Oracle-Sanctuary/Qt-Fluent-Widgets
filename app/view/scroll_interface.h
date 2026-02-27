#pragma once

#include "gallery_interface.h"

namespace qfw {

class ScrollInterface : public GalleryInterface {
    Q_OBJECT

public:
    explicit ScrollInterface(QWidget* parent = nullptr);
};

}  // namespace qfw
