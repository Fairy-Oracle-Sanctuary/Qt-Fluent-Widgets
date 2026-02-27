#pragma once

#include <QLocale>
#include <QObject>
#include <QString>
#include <QStringList>

#include "common/config.h"

namespace qfw {

// Language enumeration for gallery app
enum class Language {
    Auto,
    ChineseSimplified,
    ChineseTraditional,
    English
};

// Language serializer for config
class LanguageSerializer : public ConfigSerializer {
public:
    QVariant serialize(const QVariant& value) const override;
    QVariant deserialize(const QVariant& value) const override;
};

// Helper to check if running on Windows 11
bool isWin11();

// Gallery application configuration
class AppConfig : public QObject {
    Q_OBJECT

public:
    static AppConfig& instance();

    // Config items
    ConfigItem* musicFolders();
    ConfigItem* downloadFolder();
    ConfigItem* micaEnabled();
    ConfigItem* dpiScale();
    ConfigItem* language();
    ConfigItem* blurRadius();
    ConfigItem* checkUpdateAtStartUp();

    // Getters
    QStringList getMusicFolders() const;
    QString getDownloadFolder() const;
    bool getMicaEnabled() const;
    QString getDpiScale() const;
    Language getLanguage() const;
    int getBlurRadius() const;
    bool getCheckUpdateAtStartUp() const;

    // Setters
    void setMusicFolders(const QStringList& value);
    void setDownloadFolder(const QString& value);
    void setMicaEnabled(bool value);
    void setDpiScale(const QString& value);
    void setLanguage(Language value);
    void setBlurRadius(int value);
    void setCheckUpdateAtStartUp(bool value);

    // Load from JSON file
    void load(const QString& filePath = QStringLiteral("app/config/config.json"));

    // Constants from Python config
    static constexpr int YEAR = 2023;
    static const char* AUTHOR;
    static const char* VERSION;
    static const char* HELP_URL;
    static const char* REPO_URL;
    static const char* EXAMPLE_URL;
    static const char* FEEDBACK_URL;
    static const char* RELEASE_URL;
    static const char* ZH_SUPPORT_URL;
    static const char* EN_SUPPORT_URL;

signals:
    void musicFoldersChanged(const QStringList& value);
    void downloadFolderChanged(const QString& value);
    void micaEnabledChanged(bool value);
    void dpiScaleChanged(const QString& value);
    void languageChanged(Language value);
    void blurRadiusChanged(int value);
    void checkUpdateAtStartUpChanged(bool value);

private:
    explicit AppConfig(QObject* parent = nullptr);

    ConfigItem* musicFolders_ = nullptr;
    ConfigItem* downloadFolder_ = nullptr;
    ConfigItem* micaEnabled_ = nullptr;
    ConfigItem* dpiScale_ = nullptr;
    ConfigItem* language_ = nullptr;
    ConfigItem* blurRadius_ = nullptr;
    ConfigItem* checkUpdateAtStartUp_ = nullptr;

    std::shared_ptr<LanguageSerializer> languageSerializer_;
};

// Global accessor
AppConfig& appConfig();

}  // namespace qfw
