#pragma once

#include <QAction>
#include <QColor>
#include <QIcon>
#include <QIconEngine>
#include <QPainter>
#include <QString>

#include "common/config.h"

namespace qfw {

// Forward declarations
class FluentIconBase;
class ColoredFluentIcon;

// ============================================================================
// Icon Engines
// ============================================================================

class FluentIconEngine : public QIconEngine {
public:
    FluentIconEngine(const FluentIconBase* icon, bool reverse = false);
    FluentIconEngine(const FluentIconBase& icon, bool reverse = false);
    ~FluentIconEngine() override;

    void paint(QPainter* painter, const QRect& rect, QIcon::Mode mode, QIcon::State state) override;
    QIconEngine* clone() const override;
    QPixmap pixmap(const QSize& size, QIcon::Mode mode, QIcon::State state) override;

private:
    const FluentIconBase* icon_;
    bool isThemeReversed_;
    bool ownsIcon_;
};

class SvgIconEngine : public QIconEngine {
public:
    explicit SvgIconEngine(const QString& svg);
    explicit SvgIconEngine(const QByteArray& svgData);

    void paint(QPainter* painter, const QRect& rect, QIcon::Mode mode, QIcon::State state) override;
    QIconEngine* clone() const override;
    QPixmap pixmap(const QSize& size, QIcon::Mode mode, QIcon::State state) override;

private:
    QByteArray svgData_;
};

class FontIconEngine : public QIconEngine {
public:
    FontIconEngine(const QString& fontFamily, const QString& character, const QColor& color,
                   bool bold);

    void paint(QPainter* painter, const QRect& rect, QIcon::Mode mode, QIcon::State state) override;
    QIconEngine* clone() const override;
    QPixmap pixmap(const QSize& size, QIcon::Mode mode, QIcon::State state) override;

private:
    QString fontFamily_;
    QString character_;
    QColor color_;
    bool isBold_;
};

// ============================================================================
// Helper Functions
// ============================================================================

void drawSvgIcon(const QString& iconPath, QPainter* painter, const QRect& rect);
void drawSvgIcon(const QByteArray& iconData, QPainter* painter, const QRect& rect);

QString writeSvg(const QString& iconPath, const QList<int>& indexes = QList<int>(),
                 const QVariantMap& attributes = QVariantMap());

QString getIconColor(Theme theme = Theme::Auto, bool reverse = false);

QIcon toQIcon(const FluentIconBase& icon);
QIcon toQIcon(const QString& iconPath);

// ============================================================================
// Fluent Icon Base Classes
// ============================================================================

class FluentIconBase {
public:
    virtual ~FluentIconBase() = default;

    // Get icon path based on theme
    virtual QString path(Theme theme = Theme::Auto) const = 0;

    // Create QIcon from this fluent icon
    virtual QIcon icon(Theme theme = Theme::Auto, const QColor& color = QColor()) const;

    // Create theme-aware QIcon that updates with app theme
    QIcon qicon(bool reverse = false) const;

    // Render the icon directly
    virtual void render(QPainter* painter, const QRect& rect, Theme theme = Theme::Auto,
                        const QVariantMap& attributes = {}) const;

    // Create colored version
    ColoredFluentIcon* colored(const QColor& lightColor, const QColor& darkColor) const;

    // Clone this icon (caller takes ownership)
    virtual FluentIconBase* clone() const = 0;
};

// ============================================================================
// Icon Wrapper
// ============================================================================

class Icon : public QIcon {
public:
    explicit Icon(const FluentIconBase& fluentIcon);
    explicit Icon(const FluentIconBase* fluentIcon);

    const FluentIconBase* fluentIcon() const { return fluentIcon_; }

private:
    const FluentIconBase* fluentIcon_ = nullptr;
};

// ============================================================================
// Colored Fluent Icon
// ============================================================================

class ColoredFluentIcon : public FluentIconBase {
public:
    ColoredFluentIcon(const FluentIconBase& icon, const QColor& lightColor,
                      const QColor& darkColor);
    ColoredFluentIcon(const FluentIconBase* icon, const QColor& lightColor,
                      const QColor& darkColor);

    QString path(Theme theme = Theme::Auto) const override;
    QIcon icon(Theme theme = Theme::Auto, const QColor& color = QColor()) const override;
    void render(QPainter* painter, const QRect& rect, Theme theme = Theme::Auto,
                const QVariantMap& attributes = {}) const override;
    FluentIconBase* clone() const override;

private:
    const FluentIconBase* fluentIcon_;
    QColor lightColor_;
    QColor darkColor_;
    bool ownsIcon_;
};

// ============================================================================
// Action Class
// ============================================================================

class Action : public QAction {
    Q_OBJECT

public:
    // Constructors matching Python version
    explicit Action(QObject* parent = nullptr);
    explicit Action(const QString& text, QObject* parent = nullptr);
    explicit Action(const QIcon& icon, const QString& text, QObject* parent = nullptr);
    explicit Action(const FluentIconBase& icon, const QString& text, QObject* parent = nullptr);

    ~Action();

    // Icon handling
    QIcon icon() const;
    void setIcon(const QIcon& icon);
    void setIcon(const FluentIconBase& icon);

    const FluentIconBase* fluentIcon() const { return fluentIcon_; }

private:
    const FluentIconBase* fluentIcon_ = nullptr;
    bool ownsIcon_ = false;
};

enum class FluentIconEnum {
    // Navigation
    Up,
    Down,
    ChevronDown,
    ChevronRight,
    CareUpSolid,
    CareDownSolid,
    CareLeftSolid,
    CareRightSolid,
    LeftArrow,
    RightArrow,
    ArrowDown,

    // Actions
    Add,
    Remove,
    Delete,
    Edit,
    Save,
    Cancel,
    Accept,
    Copy,
    Paste,
    Cut,
    Search,
    Sync,
    Settings,
    Update,
    Return,
    Connect,
    History,
    Filter,
    Scroll,
    IOT,
    Tag,
    VPN,
    Hide,
    Unit,
    View,

    // Media
    Play,
    Pause,
    Volume,
    Mute,
    SkipBack,
    SkipForward,
    PlaySolid,
    SpeedOff,
    SpeedHigh,
    SpeedMedium,

    // UI
    Menu,
    More,
    Close,
    BackToWindow,
    FullScreen,
    Minimize,
    Home,
    Info,
    Layout,
    Alignment,
    PageLeft,
    PageRight,

    // Common
    Folder,
    File,
    Download,
    Link,
    Share,
    Print,
    Mail,
    Message,
    Calendar,
    Clock,
    DateTime,
    Document,
    Language,
    Question,
    Speakers,
    FontSize,

    // Misc
    Setting,
    Help,
    Github,
    QrCode,
    Wifi,
    Bluetooth,
    Airplane,
    Train,
    Car,
    Bus,
    Cafe,
    Chat,
    Code,
    Game,
    Flag,
    Font,
    Leaf,
    Movie,
    Music,
    Robot,
    Photo,
    Phone,
    Ringer,
    Rotate,
    Zoom,
    ZoomIn,
    ZoomOut,
    Album,
    Brush,
    Broom,
    Cloud,
    Embed,
    Globe,
    Heart,
    Label,
    Media,
    PauseBold,
    PencilInk,
    Tiles,
    Unpin,
    Video,
    AddTo,
    Camera,
    Market,
    People,
    Frigid,
    SaveAs,
    Palette,
    FitPage,
    BasketBall,
    Brightness,
    Dictionary,
    Microphone,
    Headphone,
    Megaphone,
    Projector,
    Education,
    EraseTool,
    BookShelf,
    Highlight,
    FolderAdd,
    PieSingle,
    QuickNote,
    StopWatch,
    ZipFolder,
    Application,
    Certificate,
    Transparent,
    ImageExport,
    LibraryFill,
    MusicFolder,
    PowerButton,
    AcceptMedium,
    CancelMedium,
    ChevronDownMed,
    ChevronRightMed,
    EmojiTabSymbols,
    ExpressiveInputEntry,
    ClippingTool,
    SearchMirror,
    ShoppingCart,
    FontIncrease,
    CommandPrompt,
    CloudDownload,
    DictionaryAdd,
    ClearSelection,
    DeveloperTools,
    BackgroundColor,
    MixVolumes,
    RemoveFrom,
    QuietHours,
    Fingerprint,
    CheckBox,
    HomeFill,
    SaveCopy,
    SendFill,
    Feedback,
    Asterisk,
    Calories,
    Constract,
    Move,
    Send,
    Pin
};

class FluentIcon : public FluentIconBase {
public:
    explicit FluentIcon(FluentIconEnum icon);

    QString path(Theme theme = Theme::Auto) const override;
    QIcon icon(Theme theme = Theme::Auto, const QColor& color = QColor()) const override;
    void render(QPainter* painter, const QRect& rect, Theme theme = Theme::Auto,
                const QVariantMap& attributes = {}) const override;
    FluentIconBase* clone() const override;

    static QString enumToString(FluentIconEnum icon);

private:
    FluentIconEnum icon_;
};

}  // namespace qfw
