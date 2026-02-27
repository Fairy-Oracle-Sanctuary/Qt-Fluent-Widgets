#pragma once

#include <QColor>
#include <QMetaType>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <memory>

namespace qfw {

enum class Theme { Light, Dark, Auto };

QString themeToString(Theme theme);
Theme themeFromString(const QString& value, Theme fallback = Theme::Light);

class ConfigValidator {
public:
    virtual ~ConfigValidator() = default;
    virtual bool validate(const QVariant& value) const;
    virtual QVariant correct(const QVariant& value) const;
};

class RangeValidator final : public ConfigValidator {
public:
    RangeValidator(double min, double max);

    bool validate(const QVariant& value) const override;
    QVariant correct(const QVariant& value) const override;

    double min() const;
    double max() const;

private:
    double min_;
    double max_;
};

class OptionsValidator : public ConfigValidator {
public:
    explicit OptionsValidator(QList<QVariant> options);

    bool validate(const QVariant& value) const override;
    QVariant correct(const QVariant& value) const override;

    const QList<QVariant>& options() const;

private:
    QList<QVariant> options_;
};

class BoolValidator final : public OptionsValidator {
public:
    BoolValidator();
};

class ColorValidator final : public ConfigValidator {
public:
    explicit ColorValidator(const QColor& def);
    bool validate(const QVariant& value) const override;
    QVariant correct(const QVariant& value) const override;

private:
    QColor def_;
};

class FolderValidator final : public ConfigValidator {
public:
    bool validate(const QVariant& value) const override;
    QVariant correct(const QVariant& value) const override;
};

class FolderListValidator final : public ConfigValidator {
public:
    bool validate(const QVariant& value) const override;
    QVariant correct(const QVariant& value) const override;
};

class ConfigSerializer {
public:
    virtual ~ConfigSerializer() = default;
    virtual QVariant serialize(const QVariant& value) const;
    virtual QVariant deserialize(const QVariant& value) const;
};

class ThemeSerializer final : public ConfigSerializer {
public:
    QVariant serialize(const QVariant& value) const override;
    QVariant deserialize(const QVariant& value) const override;
};

class ColorSerializer final : public ConfigSerializer {
public:
    QVariant serialize(const QVariant& value) const override;
    QVariant deserialize(const QVariant& value) const override;
};

class StringListSerializer final : public ConfigSerializer {
public:
    QVariant serialize(const QVariant& value) const override;
    QVariant deserialize(const QVariant& value) const override;
};

class ConfigItem : public QObject {
    Q_OBJECT

public:
    ConfigItem(QString group, QString name, QVariant def,
               std::shared_ptr<ConfigValidator> validator = {},
               std::shared_ptr<ConfigSerializer> serializer = {}, bool restart = false);

    const QString& group() const;
    const QString& name() const;
    QString key() const;

    QVariant value() const;
    void setValue(const QVariant& v);

    QVariant defaultValue() const;
    bool restart() const;

    QVariant serialize() const;
    void deserializeFrom(const QVariant& value);

signals:
    void valueChanged(const QVariant& value);

private:
    QString group_;
    QString name_;
    std::shared_ptr<ConfigValidator> validator_;
    std::shared_ptr<ConfigSerializer> serializer_;
    QVariant value_;
    QVariant defaultValue_;
    bool restart_ = false;
};

class RangeConfigItem final : public ConfigItem {
    Q_OBJECT

public:
    RangeConfigItem(QString group, QString name, QVariant def,
                    std::shared_ptr<RangeValidator> validator,
                    std::shared_ptr<ConfigSerializer> serializer = {}, bool restart = false);

    std::shared_ptr<RangeValidator> rangeValidator() const;

private:
    std::shared_ptr<RangeValidator> rangeValidator_;
};

class OptionsConfigItem final : public ConfigItem {
    Q_OBJECT

public:
    OptionsConfigItem(QString group, QString name, QVariant def,
                      std::shared_ptr<OptionsValidator> validator,
                      std::shared_ptr<ConfigSerializer> serializer = {}, bool restart = false);

    std::shared_ptr<OptionsValidator> optionsValidator() const;

private:
    std::shared_ptr<OptionsValidator> optionsValidator_;
};

class ColorConfigItem final : public ConfigItem {
    Q_OBJECT

public:
    ColorConfigItem(QString group, QString name, const QColor& def, bool restart = false);
};

class FolderConfigItem final : public ConfigItem {
    Q_OBJECT

public:
    FolderConfigItem(QString group, QString name, QString def, bool restart = false);
};

class FolderListConfigItem final : public ConfigItem {
    Q_OBJECT

public:
    FolderListConfigItem(QString group, QString name, QStringList def, bool restart = false);
};

class QConfig final : public QObject {
    Q_OBJECT

public:
    static QConfig& instance();

    QVariant value(const QString& key, const QVariant& defaultValue = {}) const;
    void setValue(const QString& key, const QVariant& value, bool emitSignal = true);

    void load();
    void load(const QString& filePath);
    void save() const;
    void save(const QString& filePath) const;

    QVariant get(const ConfigItem& item) const;
    void set(ConfigItem& item, const QVariant& value, bool saveToFile = true,
             bool emitSignal = true, bool lazyUpdate = false);

    const QString& configFilePath() const;
    void setConfigFilePath(const QString& filePath);

    OptionsConfigItem& themeModeItem();
    ColorConfigItem& themeColorItem();

    void registerItem(ConfigItem* item);

    Theme theme() const;
    void setTheme(Theme theme, bool emitSignal = true);

    void notifyThemeChangedFinished();

    QColor themeColor() const;
    void setThemeColor(const QColor& color, bool emitSignal = true);

signals:
    void appRestartSig();
    void themeChanged(qfw::Theme theme);
    void themeChangedFinished();
    void themeColorChanged(const QColor& color);

private:
    explicit QConfig(QObject* parent = nullptr);

    void loadThemeAndColorFromSettings();
    void saveThemeToSettings(Theme theme);
    void saveThemeColorToSettings(const QColor& color);

    static QString settingsOrgName();
    static QString settingsAppName();
    static QString keyThemeMode();
    static QString keyThemeColor();

    void registerBuiltInItems();
    void syncBuiltInFromItems(bool emitSignal);
    void loadFromJson(const QString& filePath);
    void saveToJson(const QString& filePath) const;

    QString filePath_;
    QPointer<OptionsConfigItem> themeMode_;
    QPointer<ColorConfigItem> themeColorItem_;
    QList<QPointer<ConfigItem>> items_;

    Theme theme_;
    QColor themeColor_;
};

bool isDarkTheme();

Theme theme();
bool isDarkThemeMode(Theme theme = Theme::Auto);

}  // namespace qfw

Q_DECLARE_METATYPE(qfw::Theme)
