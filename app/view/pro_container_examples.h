#pragma once
#include <QObject>

namespace qfw {
class GalleryInterface;
class ProContainerExamples : public QObject {
    Q_OBJECT
public:
    static void addLayout(GalleryInterface* gallery);
    static void addStatus(GalleryInterface* gallery);
    static void addText(GalleryInterface* gallery);
    static void addView(GalleryInterface* gallery);
    static void addDialogs(GalleryInterface* gallery);
};
}  // namespace qfw
