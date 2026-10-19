#pragma once
#include <QObject>

namespace qfw {
class GalleryInterface;
class ProPickerExamples : public QObject {
    Q_OBJECT
public:
    static void addBasicInput(GalleryInterface* gallery);
};
}  // namespace qfw
