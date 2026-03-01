#include "common/config.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPalette>
#include <QSettings>

#include "common/style_sheet.h"

namespace qfw {

QString themeToString(Theme theme) {
    switch (theme) {
        case Theme::Light:
            return QStringLiteral("Light");
        case Theme::Dark:
            return QStringLiteral("Dark");
        case Theme::Auto:
            return QStringLiteral("Auto");
    }
    return QStringLiteral("Light");
}

Theme themeFromString(const QString& value, Theme fallback) {
    const QString v = value.trimmed();
    if (v.compare(QStringLiteral("Light"), Qt::CaseInsensitive) == 0) {
        return Theme::Light;
    }
    if (v.compare(QStringLiteral("Dark"), Qt::CaseInsensitive) == 0) {
        return Theme::Dark;
    }
    if (v.compare(QStringLiteral("Auto"), Qt::CaseInsensitive) == 0) {
        return Theme::Auto;
    }
    return fallback;
}

bool ConfigValidator::validate(const QVariant& value) const {
    Q_UNUSED(value);
    return true;
}

QVariant ConfigValidator::correct(const QVariant& value) const { return value; }

RangeValidator::RangeValidator(double min, double max) : min_(min), max_(max) {}

bool RangeValidator::validate(const QVariant& value) const {
    bool ok = false;
    const double v = value.toDouble(&ok);
    return ok && v >= min_ && v <= max_;
}

QVariant RangeValidator::correct(const QVariant& value) const {
    bool ok = false;
    const double v = value.toDouble(&ok);
    if (!ok) {
        return min_;
    }
    return std::min(std::max(v, min_), max_);
}

double RangeValidator::minimum() const { return min_; }
double RangeValidator::maximum() const { return max_; }

OptionsValidator::OptionsValidator(QList<QVariant> options) : options_(std::move(options)) {}

bool OptionsValidator::validate(const QVariant& value) const { return options_.contains(value); }

QVariant OptionsValidator::correct(const QVariant& value) const {
    if (validate(value)) {
        return value;
    }
    return options_.isEmpty() ? QVariant{} : options_.first();
}

const QList<QVariant>& OptionsValidator::options() const { return options_; }

BoolValidator::BoolValidator() : OptionsValidator({true, false}) {}

ColorValidator::ColorValidator(const QColor& def) : def_(def) {}

bool ColorValidator::validate(const QVariant& value) const {
    const QColor c(value.toString());
    return c.isValid();
}

QVariant ColorValidator::correct(const QVariant& value) const {
    const QColor c(value.toString());
    return c.isValid() ? QVariant(c) : QVariant(def_);
}

static QString normalizePath(const QString& path) {
    QString p = QDir::cleanPath(path);
    p.replace('\\', '/');
    return p;
}

bool FolderValidator::validate(const QVariant& value) const {
    const QString p = normalizePath(value.toString());
    return QDir(p).exists();
}

QVariant FolderValidator::correct(const QVariant& value) const {
    const QString p = normalizePath(value.toString());
    if (p.isEmpty()) {
        return QString();
    }
    QDir dir(p);
    if (!dir.exists()) {
        dir.mkpath(QStringLiteral("."));
    }
    return normalizePath(QFileInfo(dir.absolutePath()).absoluteFilePath());
}

bool FolderListValidator::validate(const QVariant& value) const {
    const QStringList list = value.toStringList();
    for (const QString& p0 : list) {
        const QString p = normalizePath(p0);
        if (!QDir(p).exists()) {
            return false;
        }
    }
    return true;
}

QVariant FolderListValidator::correct(const QVariant& value) const {
    const QStringList list = value.toStringList();
    QStringList ok;
    for (const QString& p0 : list) {
        const QString p = normalizePath(p0);
        if (p.isEmpty()) {
            continue;
        }
        if (QDir(p).exists()) {
            ok.append(normalizePath(QFileInfo(QDir(p).absolutePath()).absoluteFilePath()));
        }
    }
    return ok;
}

QVariant ConfigSerializer::serialize(const QVariant& value) const { return value; }

QVariant ConfigSerializer::deserialize(const QVariant& value) const { return value; }

QVariant ThemeSerializer::serialize(const QVariant& value) const {
    const Theme t = value.value<Theme>();
    return themeToString(t);
}

QVariant ThemeSerializer::deserialize(const QVariant& value) const {
    return QVariant::fromValue(themeFromString(value.toString(), Theme::Light));
}

QVariant ColorSerializer::serialize(const QVariant& value) const {
    const QColor c = value.value<QColor>();
    return c.name(QColor::HexArgb);
}

QVariant ColorSerializer::deserialize(const QVariant& value) const {
    const QColor c(value.toString());
    return QVariant(c);
}

QVariant StringListSerializer::serialize(const QVariant& value) const {
    if (value.canConvert<QStringList>()) {
        return value.toStringList();
    }
    if (value.typeId() == QMetaType::QString) {
        return QStringList{value.toString()};
    }
    return value;
}

QVariant StringListSerializer::deserialize(const QVariant& value) const {
    if (value.typeId() == QMetaType::QStringList) {
        return value.toStringList();
    }
    if (value.typeId() == QMetaType::QVariantList) {
        QStringList list;
        const QVariantList vl = value.toList();
        for (const QVariant& v : vl) {
            list.append(v.toString());
        }
        return list;
    }
    if (value.typeId() == QMetaType::QString) {
        return QStringList{value.toString()};
    }
    return QStringList{};
}

ConfigItem::ConfigItem(QString group, QString name, QVariant def,
                       std::shared_ptr<ConfigValidator> validator,
                       std::shared_ptr<ConfigSerializer> serializer, bool restart)
    : group_(std::move(group)),
      name_(std::move(name)),
      validator_(validator ? std::move(validator) : std::make_shared<ConfigValidator>()),
      serializer_(serializer ? std::move(serializer) : std::make_shared<ConfigSerializer>()),
      restart_(restart) {
    defaultValue_ = validator_->correct(def);
    setValue(def);
}

const QString& ConfigItem::group() const { return group_; }
const QString& ConfigItem::name() const { return name_; }

QString ConfigItem::key() const {
    if (name_.isEmpty()) {
        return group_;
    }
    return group_ + QStringLiteral("/") + name_;
}

QVariant ConfigItem::value() const { return value_; }

void ConfigItem::setValue(const QVariant& v) {
    const QVariant corrected = validator_->correct(v);
    if (value_ == corrected) {
        return;
    }
    value_ = corrected;
    emit valueChanged(value_);
}

QVariant ConfigItem::defaultValue() const { return defaultValue_; }
bool ConfigItem::restart() const { return restart_; }

QVariant ConfigItem::serialize() const { return serializer_->serialize(value_); }

void ConfigItem::deserializeFrom(const QVariant& value) {
    setValue(serializer_->deserialize(value));
}

RangeConfigItem::RangeConfigItem(QString group, QString name, QVariant def,
                                 std::shared_ptr<RangeValidator> validator,
                                 std::shared_ptr<ConfigSerializer> serializer, bool restart)
    : ConfigItem(std::move(group), std::move(name), std::move(def), validator, serializer, restart),
      rangeValidator_(std::move(validator)) {}

std::shared_ptr<RangeValidator> RangeConfigItem::rangeValidator() const { return rangeValidator_; }

OptionsConfigItem::OptionsConfigItem(QString group, QString name, QVariant def,
                                     std::shared_ptr<OptionsValidator> validator,
                                     std::shared_ptr<ConfigSerializer> serializer, bool restart)
    : ConfigItem(std::move(group), std::move(name), std::move(def), validator, serializer, restart),
      optionsValidator_(std::move(validator)) {}

std::shared_ptr<OptionsValidator> OptionsConfigItem::optionsValidator() const {
    return optionsValidator_;
}

ColorConfigItem::ColorConfigItem(QString group, QString name, const QColor& def, bool restart)
    : ConfigItem(std::move(group), std::move(name), QVariant(def),
                 std::make_shared<ColorValidator>(def), std::make_shared<ColorSerializer>(),
                 restart) {}

FolderConfigItem::FolderConfigItem(QString group, QString name, QString def, bool restart)
    : ConfigItem(std::move(group), std::move(name), QVariant(std::move(def)),
                 std::make_shared<FolderValidator>(), std::make_shared<ConfigSerializer>(),
                 restart) {}

FolderListConfigItem::FolderListConfigItem(QString group, QString name, QStringList def,
                                           bool restart)
    : ConfigItem(std::move(group), std::move(name), QVariant(std::move(def)),
                 std::make_shared<FolderListValidator>(), std::make_shared<StringListSerializer>(),
                 restart) {}

QConfig& QConfig::instance() {
    static QConfig inst;
    return inst;
}

QConfig::QConfig(QObject* parent)
    : QObject(parent),
      filePath_(QStringLiteral("config/config.json")),
      theme_(Theme::Light),
      themeColor_(QColor(QStringLiteral("#009faa"))) {
    registerBuiltInItems();
}

QString QConfig::settingsOrgName() { return QStringLiteral("qfluentwidgets"); }

QString QConfig::settingsAppName() {
    return qApp ? qApp->applicationName() : QStringLiteral("qfluentwidgets_cpp");
}

QString QConfig::keyThemeMode() { return QStringLiteral("QFluentWidgets/ThemeMode"); }

QString QConfig::keyThemeColor() { return QStringLiteral("QFluentWidgets/ThemeColor"); }

void QConfig::registerBuiltInItems() {
    if (themeMode_ && themeColorItem_) {
        return;
    }

    const QList<QVariant> options = {QVariant::fromValue(Theme::Light),
                                     QVariant::fromValue(Theme::Dark),
                                     QVariant::fromValue(Theme::Auto)};

    themeMode_ = new OptionsConfigItem(
        QStringLiteral("QFluentWidgets"), QStringLiteral("ThemeMode"),
        QVariant::fromValue(Theme::Light), std::make_shared<OptionsValidator>(options),
        std::make_shared<ThemeSerializer>(), false);
    themeMode_->setParent(this);

    themeColorItem_ =
        new ColorConfigItem(QStringLiteral("QFluentWidgets"), QStringLiteral("ThemeColor"),
                            QColor(QStringLiteral("#009faa")), false);
    themeColorItem_->setParent(this);

    registerItem(themeMode_);
    registerItem(themeColorItem_);
}

void QConfig::registerItem(ConfigItem* item) {
    if (!item) {
        return;
    }
    item->setParent(this);
    for (const auto& ptr : items_) {
        if (ptr.data() == item) {
            return;
        }
    }
    items_.append(item);
}

void QConfig::syncBuiltInFromItems(bool emitSignal) {
    if (themeMode_) {
        const Theme t = themeMode_->value().value<Theme>();
        setTheme(t, false);
        if (emitSignal) {
            emit themeChanged(t);
        }
    }
    if (themeColorItem_) {
        const QColor c = themeColorItem_->value().value<QColor>();
        if (c.isValid()) {
            setThemeColor(c, false);
            if (emitSignal) {
                emit themeColorChanged(c);
            }
        }
    }
}

void QConfig::load() { load(filePath_); }

void QConfig::load(const QString& filePath) {
    registerBuiltInItems();
    setConfigFilePath(filePath);

    loadFromJson(filePath_);
    syncBuiltInFromItems(false);

    if (!QFile::exists(filePath_)) {
        loadThemeAndColorFromSettings();
        if (themeMode_) {
            themeMode_->setValue(QVariant::fromValue(theme_));
        }
        if (themeColorItem_) {
            themeColorItem_->setValue(QVariant(themeColor_));
        }

        syncBuiltInFromItems(false);
    }
}

void QConfig::save() const { save(filePath_); }

void QConfig::save(const QString& filePath) const { saveToJson(filePath); }

QVariant QConfig::get(const ConfigItem& item) const { return item.value(); }

void QConfig::set(ConfigItem& item, const QVariant& value, bool saveToFile, bool emitSignal,
                  bool lazyUpdate) {
    if (item.value() == value) {
        return;
    }

    item.setValue(value);

    if (saveToFile) {
        save();
    }

    if (item.restart()) {
        emit appRestartSig();
    }

    if (themeMode_ && &item == themeMode_) {
        const Theme t = item.value().value<Theme>();
        StyleSheetManager::instance().setNextLazyUpdate(lazyUpdate);
        setTheme(t, emitSignal);  // setTheme() already emits themeChanged if emitSignal=true
    }

    if (themeColorItem_ && &item == themeColorItem_) {
        const QColor c = item.value().value<QColor>();
        if (c.isValid()) {
            StyleSheetManager::instance().setNextLazyUpdate(lazyUpdate);
            setThemeColor(c, emitSignal);
            if (emitSignal) {
                emit themeColorChanged(c);
            }
        }
    }
}

const QString& QConfig::configFilePath() const { return filePath_; }

void QConfig::setConfigFilePath(const QString& filePath) { filePath_ = filePath; }

OptionsConfigItem& QConfig::themeModeItem() {
    registerBuiltInItems();
    return *themeMode_;
}

ColorConfigItem& QConfig::themeColorItem() {
    registerBuiltInItems();
    return *themeColorItem_;
}

void QConfig::loadFromJson(const QString& filePath) {
    if (!QFile::exists(filePath)) {
        return;
    }

    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly)) {
        return;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    f.close();
    if (!doc.isObject()) {
        return;
    }

    const QJsonObject root = doc.object();
    for (const auto& ptr : items_) {
        ConfigItem* itemPtr = ptr.data();
        if (!itemPtr) {
            continue;
        }
        const QString group = itemPtr->group();
        const QString name = itemPtr->name();

        if (!root.contains(group)) {
            continue;
        }

        const QJsonValue groupValue = root.value(group);
        if (name.isEmpty()) {
            itemPtr->deserializeFrom(groupValue.toVariant());
            continue;
        }

        if (!groupValue.isObject()) {
            continue;
        }

        const QJsonObject obj = groupValue.toObject();
        if (!obj.contains(name)) {
            continue;
        }
        itemPtr->deserializeFrom(obj.value(name).toVariant());
    }
}

void QConfig::saveToJson(const QString& filePath) const {
    QJsonObject root;
    for (const auto& ptr : items_) {
        const ConfigItem* itemPtr = ptr.data();
        if (!itemPtr) {
            continue;
        }
        const QString group = itemPtr->group();
        const QString name = itemPtr->name();
        const QVariant v = itemPtr->serialize();

        if (name.isEmpty()) {
            root.insert(group, QJsonValue::fromVariant(v));
            continue;
        }

        QJsonObject groupObj;
        if (root.contains(group) && root.value(group).isObject()) {
            groupObj = root.value(group).toObject();
        }
        groupObj.insert(name, QJsonValue::fromVariant(v));
        root.insert(group, groupObj);
    }

    QFileInfo fi(filePath);
    QDir().mkpath(fi.dir().absolutePath());

    QFile f(filePath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return;
    }
    f.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    f.close();
}

void QConfig::loadThemeAndColorFromSettings() {
    QSettings s(settingsOrgName(), settingsAppName());

    const Theme t =
        themeFromString(s.value(keyThemeMode(), themeToString(theme_)).toString(), theme_);
    const QColor c = QColor(s.value(keyThemeColor(), themeColor_.name()).toString().trimmed());

    setTheme(t, false);
    if (c.isValid()) {
        setThemeColor(c, false);
    }
}

void QConfig::saveThemeToSettings(Theme theme) {
    QSettings s(settingsOrgName(), settingsAppName());
    s.setValue(keyThemeMode(), themeToString(theme));
}

void QConfig::saveThemeColorToSettings(const QColor& color) {
    QSettings s(settingsOrgName(), settingsAppName());
    s.setValue(keyThemeColor(), color.name(QColor::HexArgb));
}

QVariant QConfig::value(const QString& key, const QVariant& defaultValue) const {
    QSettings s(settingsOrgName(), settingsAppName());
    return s.value(key, defaultValue);
}

void QConfig::setValue(const QString& key, const QVariant& value, bool emitSignal) {
    QSettings s(settingsOrgName(), settingsAppName());
    s.setValue(key, value);

    if (key == keyThemeMode()) {
        setTheme(themeFromString(value.toString(), theme_), emitSignal);
        return;
    }
    if (key == keyThemeColor()) {
        const QColor c(value.toString());
        if (c.isValid()) {
            setThemeColor(c, emitSignal);
        }
        return;
    }
}

Theme QConfig::theme() const { return theme_; }

void QConfig::setTheme(Theme theme, bool emitSignal) {
    if (theme_ == theme) {
        return;
    }

    theme_ = theme;

    saveThemeToSettings(theme_);

    if (emitSignal) {
        emit themeChanged(theme_);
    }
}

void QConfig::notifyThemeChangedFinished() { emit themeChangedFinished(); }

QColor QConfig::themeColor() const { return themeColor_; }

void QConfig::setThemeColor(const QColor& color, bool emitSignal) {
    if (themeColor_ == color) {
        return;
    }

    themeColor_ = color;

    saveThemeColorToSettings(themeColor_);

    if (emitSignal) {
        emit themeColorChanged(themeColor_);
    }
}

bool isDarkTheme() {
    auto t = QConfig::instance().theme();
    if (t == Theme::Dark) {
        return true;
    }

    if (t == Theme::Light) {
        return false;
    }

    const QColor c = qApp->palette().color(QPalette::Window);
    return c.lightnessF() < 0.5;
}

Theme theme() { return QConfig::instance().theme(); }

bool isDarkThemeMode(Theme t) {
    if (t == Theme::Auto) {
        return isDarkTheme();
    }
    return t == Theme::Dark;
}

}  // namespace qfw
