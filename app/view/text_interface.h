#pragma once

#include "gallery_interface.h"

namespace qfw {

class TextInterface : public GalleryInterface {
    Q_OBJECT

public:
    explicit TextInterface(QWidget* parent = nullptr);
};

}  // namespace qfw
