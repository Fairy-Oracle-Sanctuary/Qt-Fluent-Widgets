#pragma once

#include <QObject>

namespace qfw {
class GalleryInterface;
class ProNavigationExamples : public QObject {
    Q_OBJECT
public:
    static void addNavigation(GalleryInterface* gallery);
};
}  // namespace qfw
