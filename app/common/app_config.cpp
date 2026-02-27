#include "app_config.h"

#include <QCoreApplication>
#include <QSettings>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace qfw {

// Static constants
const char* AppConfig::AUTHOR = "baby2016";
const char* AppConfig::VERSION = "1.0.0";  // TODO: sync with library version
const char* AppConfig::HELP_URL = "https://github.com/Fairy-Oracle-Sanctuary/Qt-Fluent-Widgets";
const char* AppConfig::REPO_URL = "https://github.com/Fairy-Oracle-Sanctuary/Qt-Fluent-Widgets";
const char* AppConfig::EXAMPLE_URL = "https://github.com/Fairy-Oracle-Sanctuary/Qt-Fluent-Widgets";
const char* AppConfig::FEEDBACK_URL =
    "https://github.com/Fairy-Oracle-Sanctuary/Qt-Fluent-Widgets/issues";
const char* AppConfig::RELEASE_URL =
    "https://github.com/Fairy-Oracle-Sanctuary/Qt-Fluent-Widgets/releases/latest";
const char* AppConfig::ZH_SUPPORT_URL =
    "https://github.com/Fairy-Oracle-Sanctuary/Qt-Fluent-Widgets";
const char* AppConfig::EN_SUPPORT_URL =
    "https://github.com/Fairy-Oracle-Sanctuary/Qt-Fluent-Widgets";

// LanguageSerializer implementation
QVariant LanguageSerializer::serialize(const QVariant& value) const {
    Language lang = static_cast<Language>(value.toInt());
    switch (lang) {
        case Language::Auto:
            return QStringLiteral("Auto");
        case Language::ChineseSimplified:
            return QLocale(QLocale::Chinese, QLocale::China).name();
        case Language::ChineseTraditional:
            return QLocale(QLocale::Chinese, QLocale::HongKong).name();
        case Language::English:
            return QLocale(QLocale::English).name();
        default:
            return QStringLiteral("Auto");
    }
}

QVariant LanguageSerializer::deserialize(const QVariant& value) const {
    QString str = value.toString();
    if (str == QStringLiteral("Auto")) {
        return static_cast<int>(Language::Auto);
    }

    QLocale locale(str);
    if (locale.language() == QLocale::Chinese) {
        if (locale.territory() == QLocale::China) {
            return static_cast<int>(Language::ChineseSimplified);
        } else if (locale.territory() == QLocale::HongKong) {
            return static_cast<int>(Language::ChineseTraditional);
        }
    } else if (locale.language() == QLocale::English) {
        return static_cast<int>(Language::English);
    }

    return static_cast<int>(Language::Auto);
}

// isWin11 helper
bool isWin11() {
#ifdef Q_OS_WIN
    return QSysInfo::productVersion() >= QStringLiteral("10") &&
           QSysInfo::kernelVersion().split('.').value(2).toInt() >= 22000;
#else
    return false;
#endif
}

// AppConfig implementation
AppConfig& AppConfig::instance() {
    static AppConfig instance;
    return instance;
}

AppConfig::AppConfig(QObject* parent)
    : QObject(parent), languageSerializer_(std::make_shared<LanguageSerializer>()) {
    // Create config items matching Python version

    // Folders
    musicFolders_ = new FolderListConfigItem(QStringLiteral("Folders"),
                                             QStringLiteral("LocalMusic"), QStringList(), false);

    downloadFolder_ = new FolderConfigItem(QStringLiteral("Folders"), QStringLiteral("Download"),
                                           QStringLiteral("app/download"), false);

    // Main window
    micaEnabled_ = new ConfigItem(QStringLiteral("MainWindow"), QStringLiteral("MicaEnabled"),
                                  isWin11(), std::make_shared<BoolValidator>(), nullptr, false);

    // DPI scale options: 1, 1.25, 1.5, 1.75, 2, "Auto"
    QList<QVariant> dpiOptions;
    dpiOptions << 1 << 1.25 << 1.5 << 1.75 << 2 << QStringLiteral("Auto");
    dpiScale_ = new OptionsConfigItem(QStringLiteral("MainWindow"), QStringLiteral("DpiScale"),
                                      QStringLiteral("Auto"),
                                      std::make_shared<OptionsValidator>(dpiOptions), nullptr,
                                      true  // restart required
    );

    // Language options
    QList<QVariant> langOptions;
    langOptions << static_cast<int>(Language::Auto) << static_cast<int>(Language::ChineseSimplified)
                << static_cast<int>(Language::English);
    language_ = new OptionsConfigItem(
        QStringLiteral("MainWindow"), QStringLiteral("Language"), static_cast<int>(Language::Auto),
        std::make_shared<OptionsValidator>(langOptions), languageSerializer_,
        true  // restart required
    );

    // Material
    blurRadius_ =
        new RangeConfigItem(QStringLiteral("Material"), QStringLiteral("AcrylicBlurRadius"), 15,
                            std::make_shared<RangeValidator>(0, 40), nullptr, false);

    // Software update
    checkUpdateAtStartUp_ =
        new ConfigItem(QStringLiteral("Update"), QStringLiteral("CheckUpdateAtStartUp"), true,
                       std::make_shared<BoolValidator>(), nullptr, false);

    // Register items with QConfig
    QConfig::instance().registerItem(musicFolders_);
    QConfig::instance().registerItem(downloadFolder_);
    QConfig::instance().registerItem(micaEnabled_);
    QConfig::instance().registerItem(dpiScale_);
    QConfig::instance().registerItem(language_);
    QConfig::instance().registerItem(blurRadius_);
    QConfig::instance().registerItem(checkUpdateAtStartUp_);

    // Set theme mode to Auto like Python
    QConfig::instance().setTheme(Theme::Auto, false);

    // Connect value changes
    connect(musicFolders_, &ConfigItem::valueChanged, this,
            [this](const QVariant& v) { emit musicFoldersChanged(v.toStringList()); });
    connect(downloadFolder_, &ConfigItem::valueChanged, this,
            [this](const QVariant& v) { emit downloadFolderChanged(v.toString()); });
    connect(micaEnabled_, &ConfigItem::valueChanged, this,
            [this](const QVariant& v) { emit micaEnabledChanged(v.toBool()); });
    connect(dpiScale_, &ConfigItem::valueChanged, this,
            [this](const QVariant& v) { emit dpiScaleChanged(v.toString()); });
    connect(language_, &ConfigItem::valueChanged, this,
            [this](const QVariant& v) { emit languageChanged(static_cast<Language>(v.toInt())); });
    connect(blurRadius_, &ConfigItem::valueChanged, this,
            [this](const QVariant& v) { emit blurRadiusChanged(v.toInt()); });
    connect(checkUpdateAtStartUp_, &ConfigItem::valueChanged, this,
            [this](const QVariant& v) { emit checkUpdateAtStartUpChanged(v.toBool()); });
}

ConfigItem* AppConfig::musicFolders() { return musicFolders_; }
ConfigItem* AppConfig::downloadFolder() { return downloadFolder_; }
ConfigItem* AppConfig::micaEnabled() { return micaEnabled_; }
ConfigItem* AppConfig::dpiScale() { return dpiScale_; }
ConfigItem* AppConfig::language() { return language_; }
ConfigItem* AppConfig::blurRadius() { return blurRadius_; }
ConfigItem* AppConfig::checkUpdateAtStartUp() { return checkUpdateAtStartUp_; }

QStringList AppConfig::getMusicFolders() const { return musicFolders_->value().toStringList(); }

QString AppConfig::getDownloadFolder() const { return downloadFolder_->value().toString(); }

bool AppConfig::getMicaEnabled() const { return micaEnabled_->value().toBool(); }

QString AppConfig::getDpiScale() const {
    QString v = dpiScale_->value().toString();
    if (v.isEmpty()) return QStringLiteral("Auto");
    return v;
}

Language AppConfig::getLanguage() const {
    return static_cast<Language>(language_->value().toInt());
}

int AppConfig::getBlurRadius() const { return blurRadius_->value().toInt(); }

bool AppConfig::getCheckUpdateAtStartUp() const { return checkUpdateAtStartUp_->value().toBool(); }

void AppConfig::setMusicFolders(const QStringList& value) {
    QConfig::instance().set(*musicFolders_, value);
}

void AppConfig::setDownloadFolder(const QString& value) {
    QConfig::instance().set(*downloadFolder_, value);
}

void AppConfig::setMicaEnabled(bool value) { QConfig::instance().set(*micaEnabled_, value); }

void AppConfig::setDpiScale(const QString& value) { QConfig::instance().set(*dpiScale_, value); }

void AppConfig::setLanguage(Language value) {
    QConfig::instance().set(*language_, static_cast<int>(value));
}

void AppConfig::setBlurRadius(int value) { QConfig::instance().set(*blurRadius_, value); }

void AppConfig::setCheckUpdateAtStartUp(bool value) {
    QConfig::instance().set(*checkUpdateAtStartUp_, value);
}

void AppConfig::load(const QString& filePath) { QConfig::instance().load(filePath); }

AppConfig& appConfig() { return AppConfig::instance(); }

}  // namespace qfw
