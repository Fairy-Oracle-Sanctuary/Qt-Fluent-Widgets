#pragma once

#include "view/gallery_interface.h"

namespace qfw {

class AcrylicLabel;

class MaterialInterface : public GalleryInterface {
    Q_OBJECT

public:
    explicit MaterialInterface(QWidget* parent = nullptr);

private slots:
    void onBlurRadiusChanged(int radius);

private:
    AcrylicLabel* label_ = nullptr;
};

}  // namespace qfw
