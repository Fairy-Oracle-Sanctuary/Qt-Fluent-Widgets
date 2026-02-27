#include <QApplication>
#include <QIcon>
#include <QResource>
#include <QSettings>
#include <QTranslator>

#ifdef MessageBox
#undef MessageBox
#endif

#include "common/app_config.h"
#include "qtfluentwidgets.h"
#include "view/main_window.h"

int main(int argc, char* argv[]) {
    // Load config early to check dpi scale setting
    qfw::appConfig().load();

    // Enable dpi scale before QApplication creation
    QString dpiScale = qfw::appConfig().getDpiScale();
    if (dpiScale != QStringLiteral("Auto")) {
        qputenv("QT_ENABLE_HIGHDPI_SCALING", "0");
        qputenv("QT_SCALE_FACTOR", dpiScale.toUtf8());
    }

    QApplication app(argc, argv);

    // Set application icon
    app.setWindowIcon(QIcon(QStringLiteral(":/gallery/images/logo.png")));

    // Initialize qtfluentwidgets library resources (icons at :/qfluentwidgets/)
    Q_INIT_RESOURCE(resource);
    // Initialize app's gallery resources (at :/gallery/)
    Q_INIT_RESOURCE(gallery);

    // Load library translator (zh_CN)
    auto* libTranslator = new QTranslator();
    auto* appTranslator = new QTranslator();

    qfw::Language lang = qfw::appConfig().getLanguage();
    bool loadChinese = false;

    if (lang == qfw::Language::ChineseSimplified) {
        loadChinese = true;
    } else if (lang == qfw::Language::Auto) {
        // Auto-detect system locale
        QLocale locale;
        if (locale.language() == QLocale::Chinese) {
            if (locale.territory() == QLocale::China || locale.territory() == QLocale::Singapore) {
                loadChinese = true;
            }
        }
    }

    if (loadChinese) {
        if (libTranslator->load(":/qfluentwidgets/i18n/qtfluentwidgets.zh_CN.qm")) {
            app.installTranslator(libTranslator);
        } else {
            delete libTranslator;
        }
        if (appTranslator->load(":/gallery/i18n/gallery.zh_CN.qm")) {
            app.installTranslator(appTranslator);
        } else {
            delete appTranslator;
        }
    } else {
        delete libTranslator;
        delete appTranslator;
    }

    qfw::registerQssClassTypes({
        QStringLiteral("SettingInterface"),
        QStringLiteral("HomeInterface"),
        QStringLiteral("BannerWidget"),
        QStringLiteral("GalleryInterface"),
        QStringLiteral("ToolBar"),
        QStringLiteral("ExampleCard"),
        QStringLiteral("GallerySeparatorWidget"),
        QStringLiteral("IconInterface"),
        QStringLiteral("IconCardView"),
        QStringLiteral("IconCard"),
        QStringLiteral("IconInfoPanel"),
        QStringLiteral("BasicInputInterface"),
        QStringLiteral("LinkCard"),
        QStringLiteral("LinkCardView"),
        QStringLiteral("SampleCard"),
        QStringLiteral("SampleCardView"),
    });

    auto* window = new qfw::MainWindow();
    window->show();

    return app.exec();
}