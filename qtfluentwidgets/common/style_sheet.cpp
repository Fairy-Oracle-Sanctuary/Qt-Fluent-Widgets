#include "common/style_sheet.h"

#include <QApplication>
#include <QDebug>
#include <QDynamicPropertyChangeEvent>
#include <QElapsedTimer>
#include <QEvent>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QMessageLogContext>
#include <QRegion>
#include <QRegularExpression>
#include <QStyleFactory>
#include <QWidget>
#include <algorithm>
#include <utility>

namespace qfw {

static QString applyThemeColorTokens(const QString& qss);
static QString fixTypeSelectors(const QString& qss);

static QHash<QString, QString>& rawQssCache() {
    static QHash<QString, QString> c;
    return c;
}

static QHash<QString, QRegularExpression>& typeSelectorRegexCache() {
    static QHash<QString, QRegularExpression> c;
    return c;
}

static QHash<QString, QString>& renderedQssCache() {
    static QHash<QString, QString> c;
    return c;
}

static bool qssDebugEnabled() {
    static const bool enabled = (qEnvironmentVariableIntValue("QFW_QSS_DEBUG") != 0);
    return enabled;
}

struct QssPerfStats {
    qint64 updateMs = 0;
    int widgetsTotal = 0;
    int widgetsDeadRemoved = 0;
    int widgetsLazySkipped = 0;
    int widgetsApplied = 0;
    int widgetsNoSource = 0;

    qint64 getStyleMs = 0;
    int renderedCacheHit = 0;
    int renderedCacheMiss = 0;

    qint64 getWidgetStyleMs = 0;  // widget->styleSheet() 调用
    qint64 compareMs = 0;         // 字符串比较
    qint64 setStyleMs = 0;        // setStyleSheet() 调用
    qint64 loopOverheadMs = 0;    // time spent in loop itself
    qint64 visibleRegionMs = 0;   // time spent in visibleRegion() calls
};

static QssPerfStats& qssPerfStats() {
    static QssPerfStats s;
    return s;
}

static void invalidateQssCaches() {
    rawQssCache().clear();
    renderedQssCache().clear();
    typeSelectorRegexCache().clear();

    if (qssDebugEnabled()) {
        qDebug() << "[qfw][qss] caches invalidated";
    }
}

static bool setStyleSheetIfChanged(QWidget* widget, const QString& qss) {
    if (!widget) {
        return false;
    }

    QElapsedTimer timer;
    if (qssDebugEnabled()) {
        timer.start();
    }

    const QString currentStyle = widget->styleSheet();
    if (qssDebugEnabled()) {
        qssPerfStats().getWidgetStyleMs += timer.elapsed();
        timer.restart();
    }

    const bool same = (qss.trimmed() == currentStyle.trimmed());
    if (qssDebugEnabled()) {
        qssPerfStats().compareMs += timer.elapsed();
    }

    if (same) {
        return false;
    }

    widget->setStyleSheet(qss);
    if (qssDebugEnabled()) {
        qssPerfStats().setStyleMs += timer.elapsed();
    }
    return true;
}

static QSet<QString>& externalQssClassTypes() {
    static QSet<QString> s;
    return s;
}

void registerQssClassType(const QString& typeName) {
    if (typeName.trimmed().isEmpty()) {
        return;
    }
    externalQssClassTypes().insert(typeName.trimmed());
    invalidateQssCaches();
}

void registerQssClassTypes(const QStringList& typeNames) {
    bool added = false;
    for (const QString& t : typeNames) {
        const QString trimmed = t.trimmed();
        if (trimmed.isEmpty()) {
            continue;
        }
        if (!externalQssClassTypes().contains(trimmed)) {
            externalQssClassTypes().insert(trimmed);
            added = true;
        }
    }
    if (added) {
        invalidateQssCaches();
    }
}

#ifndef NDEBUG
static thread_local QWidget* g_qfwStyleApplyingWidget = nullptr;
static thread_local QString g_qfwStyleApplyingSource;
static thread_local QString g_qfwStyleApplyingQssPreview;

static QtMessageHandler g_prevQtMessageHandler = nullptr;

static void installStyleParseDebugHandlerOnce();
static void applyStyleSheetWithContext(QWidget* widget, const QString& qss, const QString& source);
#endif

static QString themeFolder(Theme theme) {
    Theme t = theme;
    if (t == Theme::Auto) {
        t = QConfig::instance().theme();
    }

    if (t == Theme::Auto) {
        t = isDarkTheme() ? Theme::Dark : Theme::Light;
    }

    return (t == Theme::Dark) ? QStringLiteral("dark") : QStringLiteral("light");
}

static QString renderedCacheKeyForPath(const QString& themedPath) {
    const QColor c = QConfig::instance().themeColor();
    const QString theme = themeFolder(Theme::Auto);
    return themedPath + QStringLiteral("|theme=") + theme + QStringLiteral("|color=") + c.name();
}

QString getStyleSheet(const StyleSheetBase& source, Theme theme) {
    if (const auto* file = dynamic_cast<const StyleSheetFile*>(&source)) {
        return getStyleSheet(file->path(), theme);
    }
    if (const auto* fluent = dynamic_cast<const FluentStyleSheetSource*>(&source)) {
        return getStyleSheet(styleSheetPath(fluent->sheet(), theme), theme);
    }

    if (const auto* compose = dynamic_cast<const StyleSheetCompose*>(&source)) {
        QString result;
        for (const auto& s : compose->sources()) {
            if (!s) {
                continue;
            }
            const QString part = getStyleSheet(*s, theme);
            if (part.isEmpty()) {
                continue;
            }
            if (!result.isEmpty()) {
                result += QLatin1Char('\n');
            }
            result += part;
        }
        return result;
    }

    return fixTypeSelectors(applyThemeColorTokens(source.content(theme)));
}

static QString sheetName(FluentStyleSheet sheet) {
    switch (sheet) {
        case FluentStyleSheet::Menu:
            return QStringLiteral("menu");
        case FluentStyleSheet::Label:
            return QStringLiteral("label");
        case FluentStyleSheet::Pivot:
            return QStringLiteral("pivot");
        case FluentStyleSheet::Button:
            return QStringLiteral("button");
        case FluentStyleSheet::Dialog:
            return QStringLiteral("dialog");
        case FluentStyleSheet::Slider:
            return QStringLiteral("slider");
        case FluentStyleSheet::InfoBar:
            return QStringLiteral("info_bar");
        case FluentStyleSheet::SpinBox:
            return QStringLiteral("spin_box");
        case FluentStyleSheet::TabView:
            return QStringLiteral("tab_view");
        case FluentStyleSheet::ToolTip:
            return QStringLiteral("tool_tip");
        case FluentStyleSheet::CheckBox:
            return QStringLiteral("check_box");
        case FluentStyleSheet::ComboBox:
            return QStringLiteral("combo_box");
        case FluentStyleSheet::FlipView:
            return QStringLiteral("flip_view");
        case FluentStyleSheet::LineEdit:
            return QStringLiteral("line_edit");
        case FluentStyleSheet::ListView:
            return QStringLiteral("list_view");
        case FluentStyleSheet::TreeView:
            return QStringLiteral("tree_view");
        case FluentStyleSheet::InfoBadge:
            return QStringLiteral("info_badge");
        case FluentStyleSheet::PipsPager:
            return QStringLiteral("pips_pager");
        case FluentStyleSheet::TableView:
            return QStringLiteral("table_view");
        case FluentStyleSheet::CardWidget:
            return QStringLiteral("card_widget");
        case FluentStyleSheet::TimePicker:
            return QStringLiteral("time_picker");
        case FluentStyleSheet::ColorDialog:
            return QStringLiteral("color_dialog");
        case FluentStyleSheet::MediaPlayer:
            return QStringLiteral("media_player");
        case FluentStyleSheet::SettingCard:
            return QStringLiteral("setting_card");
        case FluentStyleSheet::TeachingTip:
            return QStringLiteral("teaching_tip");
        case FluentStyleSheet::FluentWindow:
            return QStringLiteral("fluent_window");
        case FluentStyleSheet::SwitchButton:
            return QStringLiteral("switch_button");
        case FluentStyleSheet::MessageDialog:
            return QStringLiteral("message_dialog");
        case FluentStyleSheet::StateToolTip:
            return QStringLiteral("state_tool_tip");
        case FluentStyleSheet::CalendarPicker:
            return QStringLiteral("calendar_picker");
        case FluentStyleSheet::FolderListDialog:
            return QStringLiteral("folder_list_dialog");
        case FluentStyleSheet::SettingCardGroup:
            return QStringLiteral("setting_card_group");
        case FluentStyleSheet::ExpandSettingCard:
            return QStringLiteral("expand_setting_card");
        case FluentStyleSheet::NavigationInterface:
            return QStringLiteral("navigation_interface");
        case FluentStyleSheet::ToastToolTip:
            return QStringLiteral("toast_tool_tip");
    }

    return QStringLiteral("button");
}

QString styleSheetPath(FluentStyleSheet sheet, Theme theme) {
    const QString folder = themeFolder(theme);
    return QStringLiteral(":/qfluentwidgets/qss/") + folder + QStringLiteral("/") +
           sheetName(sheet) + QStringLiteral(".qss");
}

static QString readAllText(const QString& filePath) {
    const auto it = rawQssCache().constFind(filePath);
    if (it != rawQssCache().constEnd()) {
        return it.value();
    }

    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open qss:" << filePath
                   << ", exists=" << QFileInfo::exists(filePath) << ", error=" << f.errorString();
        return {};
    }

    const QByteArray data = f.readAll();
    const QString text = QString::fromUtf8(data);
    rawQssCache().insert(filePath, text);
    return text;
}

StyleSheetFile::StyleSheetFile(QString path) : path_(std::move(path)) {}

QString StyleSheetFile::content(Theme theme) const {
    Q_UNUSED(theme);
    return readAllText(path_);
}

const QString& StyleSheetFile::path() const { return path_; }

FluentStyleSheetSource::FluentStyleSheetSource(FluentStyleSheet sheet) : sheet_(sheet) {}

QString FluentStyleSheetSource::content(Theme theme) const {
    return readAllText(styleSheetPath(sheet_, theme));
}

FluentStyleSheet FluentStyleSheetSource::sheet() const { return sheet_; }

const char* CustomStyleSheet::LIGHT_QSS_KEY = "lightCustomQss";
const char* CustomStyleSheet::DARK_QSS_KEY = "darkCustomQss";

CustomStyleSheet::CustomStyleSheet(QWidget* widget) : widget_(widget) {}

QString CustomStyleSheet::content(Theme theme) const {
    if (!widget_) {
        return {};
    }

    Theme t = theme;
    if (t == Theme::Auto) {
        t = QConfig::instance().theme();
    }
    if (t == Theme::Auto) {
        t = isDarkTheme() ? Theme::Dark : Theme::Light;
    }

    const char* key = (t == Theme::Light) ? LIGHT_QSS_KEY : DARK_QSS_KEY;
    return widget_->property(key).toString();
}

void CustomStyleSheet::setCustomStyleSheet(const QString& lightQss, const QString& darkQss) {
    setLightStyleSheet(lightQss);
    setDarkStyleSheet(darkQss);
}

void CustomStyleSheet::setLightStyleSheet(const QString& qss) {
    if (widget_) {
        widget_->setProperty(LIGHT_QSS_KEY, qss);
    }
}

void CustomStyleSheet::setDarkStyleSheet(const QString& qss) {
    if (widget_) {
        widget_->setProperty(DARK_QSS_KEY, qss);
    }
}

StyleSheetCompose::StyleSheetCompose() = default;

StyleSheetCompose::StyleSheetCompose(QList<QSharedPointer<StyleSheetBase>> sources)
    : sources_(std::move(sources)) {}

const QList<QSharedPointer<StyleSheetBase>>& StyleSheetCompose::sources() const { return sources_; }

QString StyleSheetCompose::content(Theme theme) const {
    QString result;
    for (const auto& s : sources_) {
        if (!s) {
            continue;
        }
        const QString part = s->content(theme);
        if (part.isEmpty()) {
            continue;
        }
        if (!result.isEmpty()) {
            result += QLatin1Char('\n');
        }
        result += part;
    }
    return result;
}

#ifndef NDEBUG
static void qfwStyleParseMessageHandler(QtMsgType type, const QMessageLogContext& ctx,
                                        const QString& msg) {
    if (type == QtWarningMsg &&
        msg.contains(QStringLiteral("Could not parse stylesheet of object"))) {
        if (g_qfwStyleApplyingWidget) {
            qWarning().noquote() << msg;
            qWarning().noquote()
                << QStringLiteral("[qfw] stylesheet source: %1").arg(g_qfwStyleApplyingSource);
            if (!g_qfwStyleApplyingQssPreview.isEmpty()) {
                qWarning().noquote()
                    << QStringLiteral("[qfw] qss preview: %1").arg(g_qfwStyleApplyingQssPreview);
            }
            return;
        }
    }

    // default: forward to previous handler
    if (g_prevQtMessageHandler) {
        g_prevQtMessageHandler(type, ctx, msg);
        return;
    }

    // If no previous handler, fallback to Qt's default behavior-like output
    QByteArray s = msg.toLocal8Bit();
    fprintf(stderr, "%s\n", s.constData());
}

static void installStyleParseDebugHandlerOnce() {
    static bool installed = false;
    if (installed) {
        return;
    }
    installed = true;
    g_prevQtMessageHandler = qInstallMessageHandler(qfwStyleParseMessageHandler);
}

static void applyStyleSheetWithContext(QWidget* widget, const QString& qss, const QString& source) {
    if (!widget) {
        return;
    }

    installStyleParseDebugHandlerOnce();
    g_qfwStyleApplyingWidget = widget;
    g_qfwStyleApplyingSource = source;
    g_qfwStyleApplyingQssPreview = qss.left(240).simplified();
    widget->setStyleSheet(qss);
    g_qfwStyleApplyingWidget = nullptr;
    g_qfwStyleApplyingSource.clear();
    g_qfwStyleApplyingQssPreview.clear();
}
#endif

void StyleSheetCompose::add(const QSharedPointer<StyleSheetBase>& source) {
    if (!source) {
        return;
    }
    for (const auto& s : sources_) {
        if (s == source) {
            return;
        }
    }
    sources_.append(source);
}

void StyleSheetCompose::remove(const QSharedPointer<StyleSheetBase>& source) {
    sources_.removeAll(source);
}

static QColor themedColor(const QColor& base, bool darkTheme, const QString& token) {
    float h = 0, s = 0, v = 0, a = 0;
    base.getHsvF(&h, &s, &v, &a);

    if (darkTheme) {
        s *= 0.84;
        v = 1;
        if (token == QStringLiteral("ThemeColorDark1")) {
            v *= 0.9;
        } else if (token == QStringLiteral("ThemeColorDark2")) {
            s *= 0.977;
            v *= 0.82;
        } else if (token == QStringLiteral("ThemeColorDark3")) {
            s *= 0.95;
            v *= 0.7;
        } else if (token == QStringLiteral("ThemeColorLight1")) {
            s *= 0.92;
        } else if (token == QStringLiteral("ThemeColorLight2")) {
            s *= 0.78;
        } else if (token == QStringLiteral("ThemeColorLight3")) {
            s *= 0.65;
        }
    } else {
        if (token == QStringLiteral("ThemeColorDark1")) {
            v *= 0.75;
        } else if (token == QStringLiteral("ThemeColorDark2")) {
            s *= 1.05;
            v *= 0.5;
        } else if (token == QStringLiteral("ThemeColorDark3")) {
            s *= 1.1;
            v *= 0.4;
        } else if (token == QStringLiteral("ThemeColorLight1")) {
            v *= 1.05;
        } else if (token == QStringLiteral("ThemeColorLight2")) {
            s *= 0.75;
            v *= 1.05;
        } else if (token == QStringLiteral("ThemeColorLight3")) {
            s *= 0.65;
            v *= 1.05;
        }
    }

    return QColor::fromHsvF(static_cast<qreal>(h), static_cast<qreal>(std::min(s, 1.0f)),
                            static_cast<qreal>(std::min(v, 1.0f)), static_cast<qreal>(a));
}

static QString applyThemeColorTokens(const QString& qss) {
    const QColor base = QConfig::instance().themeColor();
    const bool dark = isDarkTheme();

    QString result = qss;
    result.replace(QStringLiteral("--FontFamilies"),
                   QStringLiteral("'Segoe UI','Microsoft YaHei','PingFang SC'"));

    result.replace(QStringLiteral("--ThemeColorPrimary"), base.name());
    result.replace(QStringLiteral("--ThemeColorDark1"),
                   themedColor(base, dark, QStringLiteral("ThemeColorDark1")).name());
    result.replace(QStringLiteral("--ThemeColorDark2"),
                   themedColor(base, dark, QStringLiteral("ThemeColorDark2")).name());
    result.replace(QStringLiteral("--ThemeColorDark3"),
                   themedColor(base, dark, QStringLiteral("ThemeColorDark3")).name());
    result.replace(QStringLiteral("--ThemeColorLight1"),
                   themedColor(base, dark, QStringLiteral("ThemeColorLight1")).name());
    result.replace(QStringLiteral("--ThemeColorLight2"),
                   themedColor(base, dark, QStringLiteral("ThemeColorLight2")).name());
    result.replace(QStringLiteral("--ThemeColorLight3"),
                   themedColor(base, dark, QStringLiteral("ThemeColorLight3")).name());

    return result;
}

static QString fixTypeSelectors(const QString& qss) {
    QString result = qss;

    // Generic qssClass replacements: ClassName -> *[qssClass="ClassName"]
    static const QStringList qssClassTypes = {
        QStringLiteral("LineEdit"),
        QStringLiteral("TextEdit"),
        QStringLiteral("PlainTextEdit"),
        QStringLiteral("TextBrowser"),
        QStringLiteral("InfoBadge"),
        QStringLiteral("PrimaryPushButton"),
        QStringLiteral("PrimaryToolButton"),
        QStringLiteral("PrimaryDropDownPushButton"),
        QStringLiteral("PrimaryDropDownToolButton"),
        QStringLiteral("TransparentPushButton"),
        QStringLiteral("TransparentToolButton"),
        QStringLiteral("TransparentTogglePushButton"),
        QStringLiteral("TransparentToggleToolButton"),
        QStringLiteral("TransparentDropDownPushButton"),
        QStringLiteral("TransparentDropDownToolButton"),
        QStringLiteral("ToggleButton"),
        QStringLiteral("ToggleToolButton"),
        QStringLiteral("TogglePushButton"),
        QStringLiteral("DropDownPushButton"),
        QStringLiteral("DropDownToolButton"),
        QStringLiteral("HyperlinkButton"),
        QStringLiteral("HyperlinkLabel"),
        QStringLiteral("ToolButton"),
        QStringLiteral("RadioButton"),
        QStringLiteral("PillPushButton"),
        QStringLiteral("PillToolButton"),
        QStringLiteral("SplitDropButton"),
        QStringLiteral("PrimarySplitDropButton"),
        QStringLiteral("PushButton"),
        QStringLiteral("TabBar"),
        QStringLiteral("SimpleCardWidget"),
        QStringLiteral("ElevatedCardWidget"),
        QStringLiteral("HeaderCardWidget"),
        QStringLiteral("GroupHeaderCardWidget"),
        QStringLiteral("StateToolTip"),
        QStringLiteral("ToastToolTip"),
        QStringLiteral("SpinBox"),
        QStringLiteral("DoubleSpinBox"),
        QStringLiteral("CompactSpinBox"),
        QStringLiteral("CompactDoubleSpinBox"),
        QStringLiteral("SpinButton"),
        QStringLiteral("DateEdit"),
        QStringLiteral("DateTimeEdit"),
        QStringLiteral("TimeEdit"),
        QStringLiteral("PivotItem"),
        QStringLiteral("Pivot"),
        QStringLiteral("SegmentedItem"),
        QStringLiteral("SegmentedToolItem"),
        QStringLiteral("SegmentedWidget"),
        QStringLiteral("SegmentedToolWidget"),
        QStringLiteral("SegmentedToggleToolItem"),
        QStringLiteral("SegmentedToggleToolWidget"),
        QStringLiteral("ScrollButton"),
        QStringLiteral("CalendarPicker"),
        QStringLiteral("CalendarViewBase"),
        QStringLiteral("ScrollViewBase"),
        QStringLiteral("CycleListWidget"),
        QStringLiteral("PickerPanel"),
        QStringLiteral("SeparatorWidget"),
        QStringLiteral("ItemMaskWidget"),
        QStringLiteral("PickerBase"),
        QStringLiteral("DatePicker"),
        QStringLiteral("ZhDatePicker"),
        QStringLiteral("SettingCard"),
        QStringLiteral("SwitchSettingCard"),
        QStringLiteral("RangeSettingCard"),
        QStringLiteral("PushSettingCard"),
        QStringLiteral("PrimaryPushSettingCard"),
        QStringLiteral("HyperlinkCard"),
        QStringLiteral("ColorPickerButton"),
        QStringLiteral("ColorSettingCard"),
        QStringLiteral("ComboBoxSettingCard"),
        QStringLiteral("SettingCardGroup"),
        QStringLiteral("ExpandSettingCard"),
        QStringLiteral("ExpandGroupSettingCard"),
        QStringLiteral("SimpleExpandGroupSettingCard"),
        QStringLiteral("CustomColorSettingCard"),
        QStringLiteral("OptionsSettingCard"),
        QStringLiteral("FolderListSettingCard"),
        QStringLiteral("StackedWidget"),
        // Title bar types - need qssClass because metaObject className includes namespace prefix
        QStringLiteral("FluentTitleBar"),
        QStringLiteral("SplitTitleBar"),
        QStringLiteral("MSFluentTitleBar"),
        // Label types - need qssClass for type selector matching
        QStringLiteral("FluentLabelBase"),
        QStringLiteral("CaptionLabel"),
        QStringLiteral("BodyLabel"),
        QStringLiteral("StrongBodyLabel"),
        QStringLiteral("SubtitleLabel"),
        QStringLiteral("TitleLabel"),
        QStringLiteral("LargeTitleLabel"),
        QStringLiteral("DisplayLabel"),
    };

    QSet<QString> allTypes;
    for (const QString& t : qssClassTypes) {
        allTypes.insert(t);
    }
    allTypes.unite(externalQssClassTypes());

    for (const QString& type : std::as_const(allTypes)) {
        if (!result.contains(type)) {
            continue;
        }
        // Avoid replacing inside existing qssClass attribute values, e.g.
        // *[qssClass="SettingInterface"]  (otherwise we'd produce invalid nested selectors)
        auto it = typeSelectorRegexCache().constFind(type);
        if (it == typeSelectorRegexCache().constEnd()) {
            it = typeSelectorRegexCache().insert(
                type, QRegularExpression(QStringLiteral("(?<!qssClass=\")\\b") + type +
                                         QStringLiteral("\\b")));
        }
        result.replace(it.value(), QStringLiteral("*[qssClass=\"") + type + QStringLiteral("\"]"));
    }

    // Special replacements with custom selectors
    result.replace(QRegularExpression(QStringLiteral("\\bRoundMenu\\b")),
                   QStringLiteral("QMenu[qssClass=\"RoundMenu\"]"));
    result.replace(QRegularExpression(QStringLiteral("\\bMenuActionListWidget\\b")),
                   QStringLiteral("QListWidget[qssClass=\"MenuActionListWidget\"]"));
    result.replace(QRegularExpression(QStringLiteral("\\bComboBox\\b")),
                   QStringLiteral("*[comboBox=true]"));
    result.replace(QRegularExpression(QStringLiteral("\\bModelComboBox\\b")),
                   QStringLiteral("*[modelComboBox=true]"));
    result.replace(QRegularExpression(QStringLiteral("\\bInfoBar\\b")),
                   QStringLiteral("*[infoBar=true]"));

    return result;
}

static QString resolveThemeQssPath(const QString& sourcePath) {
    static const QRegularExpression re(QStringLiteral("^:/qfluentwidgets/qss/(light|dark)/(.+)$"));
    const QRegularExpressionMatch m = re.match(sourcePath);
    if (!m.hasMatch()) {
        return sourcePath;
    }

    const QString rest = m.captured(2);
    const QString folder = themeFolder(Theme::Auto);
    return QStringLiteral(":/qfluentwidgets/qss/") + folder + QStringLiteral("/") + rest;
}

QString getStyleSheet(const QString& sourcePath, Theme theme) {
    const QString themedPath = resolveThemeQssPath(sourcePath);
    Q_UNUSED(theme);

    QElapsedTimer timer;
    if (qssDebugEnabled()) {
        timer.start();
    }

    const QString cacheKey = renderedCacheKeyForPath(themedPath);
    const auto it = renderedQssCache().constFind(cacheKey);
    if (it != renderedQssCache().constEnd()) {
        if (qssDebugEnabled()) {
            qssPerfStats().renderedCacheHit++;
            qssPerfStats().getStyleMs += timer.elapsed();
        }
        return it.value();
    }

    if (qssDebugEnabled()) {
        qssPerfStats().renderedCacheMiss++;
    }

    const QString qss = readAllText(themedPath);
    const QString rendered = fixTypeSelectors(applyThemeColorTokens(qss));
    renderedQssCache().insert(cacheKey, rendered);

    if (qssDebugEnabled()) {
        qssPerfStats().getStyleMs += timer.elapsed();
    }
    return rendered;
}

QString getStyleSheet(FluentStyleSheet sheet, Theme theme) {
    return getStyleSheet(styleSheetPath(sheet, theme), theme);
}

void setStyleSheet(QWidget* widget, const QString& sourcePath, Theme theme, bool registerWidget) {
    if (!widget) {
        return;
    }

    if (registerWidget) {
        StyleSheetManager::instance().registerWidget(widget, sourcePath, true);
    }

#ifndef NDEBUG
    applyStyleSheetWithContext(widget, getStyleSheet(sourcePath, theme), sourcePath);
#else
    setStyleSheetIfChanged(widget, getStyleSheet(sourcePath, theme));
#endif
}

void setStyleSheet(QWidget* widget, FluentStyleSheet sheet, Theme theme, bool registerWidget) {
    if (!widget) {
        return;
    }

    FluentStyleSheetSource source(sheet);
    if (registerWidget) {
        StyleSheetManager::instance().registerWidget(
            widget, QSharedPointer<StyleSheetBase>(new FluentStyleSheetSource(sheet)), true);
    }

#ifndef NDEBUG
    applyStyleSheetWithContext(widget, getStyleSheet(source, theme), styleSheetPath(sheet, theme));
#else
    setStyleSheetIfChanged(widget, getStyleSheet(source, theme));
#endif
}

void setStyleSheet(QWidget* widget, const StyleSheetBase& source, Theme theme,
                   bool registerWidget) {
    if (!widget) {
        return;
    }

    if (registerWidget) {
        if (const auto* file = dynamic_cast<const StyleSheetFile*>(&source)) {
            StyleSheetManager::instance().registerWidget(
                widget, QSharedPointer<StyleSheetBase>(new StyleSheetFile(file->path())), true);
        } else if (const auto* fluent = dynamic_cast<const FluentStyleSheetSource*>(&source)) {
            StyleSheetManager::instance().registerWidget(
                widget, QSharedPointer<StyleSheetBase>(new FluentStyleSheetSource(fluent->sheet())),
                true);
        } else if (dynamic_cast<const CustomStyleSheet*>(&source)) {
            StyleSheetManager::instance().registerWidget(
                widget, QSharedPointer<StyleSheetBase>(new CustomStyleSheet(widget)), true);
        }
    }

    setStyleSheetIfChanged(widget, getStyleSheet(source, theme));
}

void setCustomStyleSheet(QWidget* widget, const QString& lightQss, const QString& darkQss) {
    CustomStyleSheet(widget).setCustomStyleSheet(lightQss, darkQss);
}

void addStyleSheet(QWidget* widget, const StyleSheetBase& source, Theme theme,
                   bool registerWidget) {
    if (!widget) {
        return;
    }

    if (registerWidget) {
        if (const auto* file = dynamic_cast<const StyleSheetFile*>(&source)) {
            StyleSheetManager::instance().registerWidget(
                widget, QSharedPointer<StyleSheetBase>(new StyleSheetFile(file->path())), false);
        } else if (const auto* fluent = dynamic_cast<const FluentStyleSheetSource*>(&source)) {
            StyleSheetManager::instance().registerWidget(
                widget, QSharedPointer<StyleSheetBase>(new FluentStyleSheetSource(fluent->sheet())),
                false);
        } else if (dynamic_cast<const CustomStyleSheet*>(&source)) {
            StyleSheetManager::instance().registerWidget(
                widget, QSharedPointer<StyleSheetBase>(new CustomStyleSheet(widget)), false);
        }

        if (auto s = StyleSheetManager::instance().source(widget)) {
#ifndef NDEBUG
            applyStyleSheetWithContext(widget, getStyleSheet(*s, theme),
                                       QStringLiteral("StyleSheetCompose(add)"));
#else
            widget->setStyleSheet(getStyleSheet(*s, theme));
#endif
        }
    } else {
#ifndef NDEBUG
        applyStyleSheetWithContext(
            widget, widget->styleSheet() + QStringLiteral("\n") + getStyleSheet(source, theme),
            QStringLiteral("inline-addStyleSheet"));
#else
        widget->setStyleSheet(widget->styleSheet() + QStringLiteral("\n") +
                              getStyleSheet(source, theme));
#endif
    }
}

StyleSheetManager& StyleSheetManager::instance() {
    static StyleSheetManager inst;
    return inst;
}

StyleSheetManager::StyleSheetManager(QObject* parent) : QObject(parent) {
    QObject::connect(&QConfig::instance(), &QConfig::themeChanged, this, [this](Theme) {
        const bool lazy = nextLazyUpdate_;
        if (qssDebugEnabled()) {
            qDebug() << "[qfw][qss] themeChanged signal received, nextLazyUpdate_=" << lazy;
        }
        nextLazyUpdate_ = false;
        updateStyleSheet(lazy);
        QConfig::instance().notifyThemeChangedFinished();
    });

    QObject::connect(
        &QConfig::instance(), &QConfig::themeColorChanged, this, [this](const QColor&) {
            const bool lazy = nextLazyUpdate_;
            if (qssDebugEnabled()) {
                qDebug() << "[qfw][qss] themeColorChanged signal received, nextLazyUpdate_="
                         << lazy;
            }
            nextLazyUpdate_ = false;
            updateStyleSheet(lazy);
        });
}

static bool ensureWatchersInstalled(QWidget* widget) {
    if (!widget) {
        return false;
    }
    if (!widget->property("qfw_watchers_installed").toBool()) {
        widget->installEventFilter(new CustomStyleSheetWatcher(widget));
        widget->installEventFilter(new DirtyStyleSheetWatcher(widget));
        widget->setProperty("qfw_watchers_installed", true);
    }
    return true;
}

void StyleSheetManager::registerWidget(QWidget* widget, const QString& sourcePath, bool reset) {
    registerWidget(widget, QSharedPointer<StyleSheetBase>(new StyleSheetFile(sourcePath)), reset);
}

void StyleSheetManager::registerWidget(QWidget* widget,
                                       const QSharedPointer<StyleSheetBase>& source, bool reset) {
    if (!widget) {
        return;
    }

    ensureWatchersInstalled(widget);

    for (auto& item : items_) {
        if (item.widget == widget) {
            if (!item.source) {
                item.source = QSharedPointer<StyleSheetCompose>(new StyleSheetCompose(
                    {source, QSharedPointer<StyleSheetBase>(new CustomStyleSheet(widget))}));
            } else if (reset) {
                item.source = QSharedPointer<StyleSheetCompose>(new StyleSheetCompose(
                    {source, QSharedPointer<StyleSheetBase>(new CustomStyleSheet(widget))}));
            } else {
                item.source->add(source);
            }
            return;
        }
    }

    Item item;
    item.widget = widget;
    item.source = QSharedPointer<StyleSheetCompose>(new StyleSheetCompose(
        {source, QSharedPointer<StyleSheetBase>(new CustomStyleSheet(widget))}));
    items_.append(item);

    QObject::connect(widget, &QObject::destroyed, this,
                     [this, widget]() { deregisterWidget(widget); });
}

void StyleSheetManager::deregisterWidget(QWidget* widget) {
    if (!widget) {
        return;
    }

    for (int i = items_.size() - 1; i >= 0; --i) {
        if (items_[i].widget == widget) {
            items_.removeAt(i);
        }
    }
}

QSharedPointer<StyleSheetCompose> StyleSheetManager::source(QWidget* widget) const {
    if (!widget) {
        return {};
    }
    for (const auto& item : items_) {
        if (item.widget == widget) {
            return item.source;
        }
    }
    return {};
}

void StyleSheetManager::setNextLazyUpdate(bool lazy) {
    if (qssDebugEnabled()) {
        qDebug() << "[qfw][qss] setNextLazyUpdate lazy=" << lazy;
    }
    nextLazyUpdate_ = lazy;
}

void StyleSheetManager::updateStyleSheet(bool lazy) {
    QElapsedTimer timer;
    QElapsedTimer stepTimer;
    qint64 tGetStyleSheet = 0;
    qint64 tSetIfChanged = 0;
    qint64 tLoopOverhead = 0;
    qint64 tVisibleRegion = 0;
    if (qssDebugEnabled()) {
        timer.start();
        qssPerfStats() = QssPerfStats{};
    }

    // 暂停重绘，避免中间状态的重绘开销
    QWidgetList topLevelWidgets = QApplication::topLevelWidgets();
    for (QWidget* w : topLevelWidgets) {
        w->setUpdatesEnabled(false);
    }

    for (int i = items_.size() - 1; i >= 0; --i) {
        if (qssDebugEnabled()) stepTimer.start();

        auto& item = items_[i];
        if (qssDebugEnabled()) {
            qssPerfStats().widgetsTotal++;
            tLoopOverhead += stepTimer.elapsed();
        }

        if (!item.widget) {
            items_.removeAt(i);
            if (qssDebugEnabled()) qssPerfStats().widgetsDeadRemoved++;
            continue;
        }

        if (qssDebugEnabled()) stepTimer.start();
        if (lazy && item.widget->visibleRegion().isNull()) {
            if (qssDebugEnabled()) tVisibleRegion += stepTimer.elapsed();
            item.widget->setProperty("dirty-qss", true);
            if (qssDebugEnabled()) qssPerfStats().widgetsLazySkipped++;
            continue;
        }
        if (qssDebugEnabled()) tVisibleRegion += stepTimer.elapsed();

        if (item.source) {
            if (qssDebugEnabled()) stepTimer.start();
            QString qss = getStyleSheet(*item.source, Theme::Auto);
            if (qssDebugEnabled()) tGetStyleSheet += stepTimer.elapsed();

            if (qssDebugEnabled()) stepTimer.start();
            // Debug 和 Release 都使用 setStyleSheetIfChanged 避免重复设置
            bool changed = setStyleSheetIfChanged(item.widget, qss);
            if (qssDebugEnabled()) tSetIfChanged += stepTimer.elapsed();

            if (changed) {
                if (qssDebugEnabled()) qssPerfStats().widgetsApplied++;
            }
        } else if (qssDebugEnabled()) {
            qssPerfStats().widgetsNoSource++;
        }
    }

    // 恢复重绘
    for (QWidget* w : topLevelWidgets) {
        w->setUpdatesEnabled(true);
    }

    if (qssDebugEnabled()) {
        qssPerfStats().updateMs = timer.elapsed();
        qDebug() << "[qfw][qss] updateStyleSheet lazy=" << lazy
                 << " total=" << qssPerfStats().widgetsTotal
                 << " removed=" << qssPerfStats().widgetsDeadRemoved
                 << " lazySkipped=" << qssPerfStats().widgetsLazySkipped
                 << " applied=" << qssPerfStats().widgetsApplied
                 << " noSource=" << qssPerfStats().widgetsNoSource
                 << " getStyleSheetMs=" << tGetStyleSheet << " setStyleTotalMs=" << tSetIfChanged
                 << " visibleRegionMs=" << tVisibleRegion << " loopOverheadMs=" << tLoopOverhead
                 << " cacheHit=" << qssPerfStats().renderedCacheHit
                 << " cacheMiss=" << qssPerfStats().renderedCacheMiss
                 << " updateMs=" << qssPerfStats().updateMs;
    }
}

void setTheme(Theme theme, bool save, bool lazy) {
    QElapsedTimer timer;
    if (qssDebugEnabled()) {
        timer.start();
        qDebug() << "[qfw][qss] setTheme start theme=" << static_cast<int>(theme);
    }

    QConfig::instance().set(QConfig::instance().themeModeItem(), QVariant::fromValue(theme), save,
                            true, lazy);
    if (qssDebugEnabled()) {
        qDebug() << "[qfw][qss] QConfig::set done ms=" << timer.elapsed();
    }
}

void toggleTheme(bool save, bool lazy) {
    const Theme next = isDarkTheme() ? Theme::Light : Theme::Dark;
    setTheme(next, save, lazy);
}

void setTheme(Theme theme, bool lazy) { setTheme(theme, false, lazy); }

void setThemeColor(const QColor& color, bool save, bool lazy) {
    StyleSheetManager::instance().setNextLazyUpdate(lazy);
    QConfig::instance().set(QConfig::instance().themeColorItem(), QVariant(color), save, true);
}

void setThemeColor(const QColor& color, bool lazy) { setThemeColor(color, false, lazy); }

QColor themeColor() { return QConfig::instance().themeColor(); }

void updateDynamicStyle(QWidget* widget) {
    if (!widget) {
        return;
    }

#ifdef Q_OS_WIN
#if QT_VERSION < QT_VERSION_CHECK(6, 8, 0)
    widget->setStyle(QApplication::style());
#else
    widget->setStyle(QStyleFactory::create(QStringLiteral("windowsvista")));
#endif
#else
    widget->setStyle(QApplication::style());
#endif
}

bool CustomStyleSheetWatcher::eventFilter(QObject* obj, QEvent* e) {
    auto* w = qobject_cast<QWidget*>(obj);
    if (!w || e->type() != QEvent::DynamicPropertyChange) {
        return QObject::eventFilter(obj, e);
    }

    const auto* de = static_cast<QDynamicPropertyChangeEvent*>(e);
    const QByteArray name = de->propertyName();
    if (name == CustomStyleSheet::LIGHT_QSS_KEY || name == CustomStyleSheet::DARK_QSS_KEY) {
        addStyleSheet(w, CustomStyleSheet(w), Theme::Auto, true);
    }

    return QObject::eventFilter(obj, e);
}

bool DirtyStyleSheetWatcher::eventFilter(QObject* obj, QEvent* e) {
    auto* w = qobject_cast<QWidget*>(obj);
    if (!w || e->type() != QEvent::Paint || !w->property("dirty-qss").toBool()) {
        return QObject::eventFilter(obj, e);
    }

    w->setProperty("dirty-qss", false);
    if (auto s = StyleSheetManager::instance().source(w)) {
        setStyleSheetIfChanged(w, getStyleSheet(*s, Theme::Auto));
    }

    return QObject::eventFilter(obj, e);
}

}  // namespace qfw
