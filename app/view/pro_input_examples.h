#pragma once
#include <QObject>
namespace qfw {
class GalleryInterface;
class ProInputExamples : public QObject {
    Q_OBJECT
public:
    static void addBasicInput(GalleryInterface* gallery);
    static void addText(GalleryInterface* gallery);
    static void addDateTime(GalleryInterface* gallery);
    static void addDropWidgets(GalleryInterface* gallery);
};
}  // namespace qfw
