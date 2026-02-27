#pragma once

#include <QList>
#include <QObject>
#include <QPointer>
#include <QSet>
#include <QSharedPointer>
#include <QString>
#include <QStringList>

#include "common/config.h"

class QWidget;

class QEvent;

namespace qfw {

enum class FluentStyleSheet {
    Menu,
    Label,
    Pivot,
    Button,
    Dialog,
    Slider,
    InfoBar,
    SpinBox,
    TabView,
    ToolTip,
    CheckBox,
    ComboBox,
    FlipView,
    LineEdit,
    ListView,
    TreeView,
    InfoBadge,
    PipsPager,
    TableView,
    CardWidget,
    TimePicker,
    ColorDialog,
    MediaPlayer,
    SettingCard,
    TeachingTip,
    FluentWindow,
    SwitchButton,
    MessageDialog,
    StateToolTip,
    ToastToolTip,
    CalendarPicker,
    FolderListDialog,
    SettingCardGroup,
    ExpandSettingCard,
    NavigationInterface
};

class StyleSheetBase {
public:
    virtual ~StyleSheetBase() = default;
    virtual QString content(Theme theme = Theme::Auto) const = 0;
};

class StyleSheetFile final : public StyleSheetBase {
public:
    explicit StyleSheetFile(QString path);
    QString content(Theme theme = Theme::Auto) const override;

    const QString& path() const;

private:
    QString path_;
};

class FluentStyleSheetSource final : public StyleSheetBase {
public:
    explicit FluentStyleSheetSource(FluentStyleSheet sheet);
    QString content(Theme theme = Theme::Auto) const override;
    FluentStyleSheet sheet() const;

private:
    FluentStyleSheet sheet_;
};

class CustomStyleSheet final : public StyleSheetBase {
public:
    static const char* LIGHT_QSS_KEY;
    static const char* DARK_QSS_KEY;

    explicit CustomStyleSheet(QWidget* widget);

    QString content(Theme theme = Theme::Auto) const override;

    void setCustomStyleSheet(const QString& lightQss, const QString& darkQss);
    void setLightStyleSheet(const QString& qss);
    void setDarkStyleSheet(const QString& qss);

private:
    QPointer<QWidget> widget_;
};

class StyleSheetCompose final : public StyleSheetBase {
public:
    StyleSheetCompose();
    explicit StyleSheetCompose(QList<QSharedPointer<StyleSheetBase>> sources);

    QString content(Theme theme = Theme::Auto) const override;
    const QList<QSharedPointer<StyleSheetBase>>& sources() const;
    void add(const QSharedPointer<StyleSheetBase>& source);
    void remove(const QSharedPointer<StyleSheetBase>& source);

private:
    QList<QSharedPointer<StyleSheetBase>> sources_;
};

class CustomStyleSheetWatcher final : public QObject {
    Q_OBJECT
public:
    using QObject::QObject;

protected:
    bool eventFilter(QObject* obj, QEvent* e) override;
};

class DirtyStyleSheetWatcher final : public QObject {
    Q_OBJECT
public:
    using QObject::QObject;

protected:
    bool eventFilter(QObject* obj, QEvent* e) override;
};

QString styleSheetPath(FluentStyleSheet sheet, Theme theme = Theme::Auto);
QString getStyleSheet(const QString& sourcePath, Theme theme = Theme::Auto);
QString getStyleSheet(FluentStyleSheet sheet, Theme theme = Theme::Auto);

QString getStyleSheet(const StyleSheetBase& source, Theme theme = Theme::Auto);

void setStyleSheet(QWidget* widget, const QString& sourcePath, Theme theme = Theme::Auto,
                   bool registerWidget = true);
void setStyleSheet(QWidget* widget, FluentStyleSheet sheet, Theme theme = Theme::Auto,
                   bool registerWidget = true);

void setStyleSheet(QWidget* widget, const StyleSheetBase& source, Theme theme = Theme::Auto,
                   bool registerWidget = true);

void setCustomStyleSheet(QWidget* widget, const QString& lightQss, const QString& darkQss);

void addStyleSheet(QWidget* widget, const StyleSheetBase& source, Theme theme = Theme::Auto,
                   bool registerWidget = true);

void setTheme(Theme theme, bool lazy = false);
void setThemeColor(const QColor& color, bool lazy = false);

void setTheme(Theme theme, bool save, bool lazy);
void toggleTheme(bool save = false, bool lazy = false);
void setThemeColor(const QColor& color, bool save, bool lazy);

QColor themeColor();

void registerQssClassType(const QString& typeName);
void registerQssClassTypes(const QStringList& typeNames);

void updateDynamicStyle(QWidget* widget);

class StyleSheetManager final : public QObject {
    Q_OBJECT

public:
    static StyleSheetManager& instance();

    void registerWidget(QWidget* widget, const QString& sourcePath, bool reset = true);
    void registerWidget(QWidget* widget, const QSharedPointer<StyleSheetBase>& source,
                        bool reset = true);
    void deregisterWidget(QWidget* widget);

    QSharedPointer<StyleSheetCompose> source(QWidget* widget) const;
    void setNextLazyUpdate(bool lazy);

public slots:
    void updateStyleSheet(bool lazy = false);

private:
    explicit StyleSheetManager(QObject* parent = nullptr);

    struct Item {
        QPointer<QWidget> widget;
        QSharedPointer<StyleSheetCompose> source;
    };

    QList<Item> items_;
    bool nextLazyUpdate_ = false;
};

}  // namespace qfw
