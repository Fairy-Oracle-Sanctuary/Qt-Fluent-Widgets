#pragma once
#include <QObject>

namespace qfw {
class GalleryInterface;
class ProProgressExamples : public QObject {
    Q_OBJECT
public:
    static void addStatus(GalleryInterface* gallery);
};
}  // namespace qfw
