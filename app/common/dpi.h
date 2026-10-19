#pragma once
#include <QGuiApplication>
#include <QString>

namespace qfw {
// Must run before QApplication: Qt 5 does not enable system scaling by default.
inline void configureGalleryDpi(const QString& scale) {
    const bool automatic=scale==QStringLiteral("Auto");
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling,automatic);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    if(!automatic) {
        qputenv("QT_ENABLE_HIGHDPI_SCALING","0");
        qputenv("QT_SCALE_FACTOR",scale.toUtf8());
    }
}
}
