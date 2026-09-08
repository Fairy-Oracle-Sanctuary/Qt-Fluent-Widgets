#pragma once

#include "view/gallery_interface.h"

namespace qfw {

class Issue13Interface : public GalleryInterface {
    Q_OBJECT

public:
    explicit Issue13Interface(QWidget* parent = nullptr);
};

}  // namespace qfw
