#pragma once

#include <QColor>
#include <QLabel>
#include <QPointer>
#include <QSize>
#include <QSizePolicy>

#include "common/icon.h"

namespace qfw {

enum class InfoLevel {
    Info = 0,
    Success = 1,
    Attention = 2,
    Warning = 3,
    Error = 4,
};

enum class InfoBadgePosition {
    TopRight = 0,
    BottomRight = 1,
    Right = 2,
    TopLeft = 3,
    BottomLeft = 4,
    Left = 5,
    NavigationItem = 6,
};

class InfoBadgeManager;

class InfoBadge : public QLabel {
    Q_OBJECT

public:
    explicit InfoBadge(QWidget* parent = nullptr, InfoLevel level = InfoLevel::Attention);
    explicit InfoBadge(const QString& text, QWidget* parent = nullptr,
                       InfoLevel level = InfoLevel::Attention);
    explicit InfoBadge(int num, QWidget* parent = nullptr, InfoLevel level = InfoLevel::Attention);
    explicit InfoBadge(double num, QWidget* parent = nullptr,
                       InfoLevel level = InfoLevel::Attention);

    void setLevel(InfoLevel level);
    InfoLevel level() const;

    void setCustomBackgroundColor(const QColor& light, const QColor& dark);

    static InfoBadge* make(const QString& text, QWidget* parent = nullptr,
                           InfoLevel level = InfoLevel::Info, QWidget* target = nullptr,
                           InfoBadgePosition position = InfoBadgePosition::TopRight);

    static InfoBadge* info(const QString& text, QWidget* parent = nullptr,
                           QWidget* target = nullptr,
                           InfoBadgePosition position = InfoBadgePosition::TopRight);
    static InfoBadge* success(const QString& text, QWidget* parent = nullptr,
                              QWidget* target = nullptr,
                              InfoBadgePosition position = InfoBadgePosition::TopRight);
    static InfoBadge* attention(const QString& text, QWidget* parent = nullptr,
                                QWidget* target = nullptr,
                                InfoBadgePosition position = InfoBadgePosition::TopRight);
    static InfoBadge* warning(const QString& text, QWidget* parent = nullptr,
                              QWidget* target = nullptr,
                              InfoBadgePosition position = InfoBadgePosition::TopRight);
    static InfoBadge* error(const QString& text, QWidget* parent = nullptr,
                            QWidget* target = nullptr,
                            InfoBadgePosition position = InfoBadgePosition::TopRight);

    static InfoBadge* custom(const QString& text, const QColor& light, const QColor& dark,
                             QWidget* parent = nullptr, QWidget* target = nullptr,
                             InfoBadgePosition position = InfoBadgePosition::TopRight);

protected:
    void paintEvent(QPaintEvent* e) override;

    QColor backgroundColor() const;

protected:
    QPointer<InfoBadgeManager> manager_;

private:
    void init(InfoLevel level);

    InfoLevel level_ = InfoLevel::Info;
    QColor lightBackgroundColor_;
    QColor darkBackgroundColor_;

    friend class InfoBadgeManager;
};

class DotInfoBadge : public InfoBadge {
    Q_OBJECT

public:
    explicit DotInfoBadge(QWidget* parent = nullptr, InfoLevel level = InfoLevel::Attention);

    static DotInfoBadge* make(QWidget* parent = nullptr, InfoLevel level = InfoLevel::Info,
                              QWidget* target = nullptr,
                              InfoBadgePosition position = InfoBadgePosition::TopRight);

protected:
    void paintEvent(QPaintEvent* e) override;
};

class IconInfoBadge : public InfoBadge {
    Q_OBJECT

public:
    explicit IconInfoBadge(QWidget* parent = nullptr, InfoLevel level = InfoLevel::Attention);
    explicit IconInfoBadge(const QVariant& icon, QWidget* parent = nullptr,
                           InfoLevel level = InfoLevel::Attention);

    void setIcon(const QVariant& icon);
    QVariant icon() const;

    QSize iconSize() const;
    void setIconSize(const QSize& size);

    static IconInfoBadge* make(const QVariant& icon, QWidget* parent = nullptr,
                               InfoLevel level = InfoLevel::Info, QWidget* target = nullptr,
                               InfoBadgePosition position = InfoBadgePosition::TopRight);

protected:
    void paintEvent(QPaintEvent* e) override;

private:
    QVariant icon_;
    QSize iconSize_ = QSize(8, 8);
};

class InfoBadgeManager : public QObject {
    Q_OBJECT

public:
    explicit InfoBadgeManager(QWidget* target, InfoBadge* badge);

    static InfoBadgeManager* make(InfoBadgePosition position, QWidget* target, InfoBadge* badge);

    QPoint position() const;

protected:
    bool eventFilter(QObject* watched, QEvent* e) override;

private:
    QRect targetRectInBadgeParent() const;

    InfoBadgePosition position_ = InfoBadgePosition::TopRight;
    QPointer<QWidget> target_;
    QPointer<InfoBadge> badge_;
};

}  // namespace qfw
