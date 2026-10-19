#pragma once

#include "components/widgets/button.h"

class QGraphicsDropShadowEffect;
class QPropertyAnimation;

namespace qfw {

class ExtendedButtonRenderer;

// Internal painting styles shared by the push/tool variants.
enum class ExtendedButtonKind { Filled, Text, Outlined, Round, Hyperlink, Lumina };

class ExtendedPushButton : public PushButton {
    Q_OBJECT
    Q_PROPERTY(QString colorScheme READ colorScheme WRITE setColorScheme)

public:
    QString colorScheme() const;
    void setColorScheme(const QString& scheme);
    QColor backgroundColor() const;
    QColor borderColor() const;
    QColor textColor() const;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    explicit ExtendedPushButton(ExtendedButtonKind kind, QWidget* parent);
    void paintEvent(QPaintEvent* event) override;

private:
    ExtendedButtonRenderer* renderer_;
};

class ExtendedToolButton : public ToolButton {
    Q_OBJECT
    Q_PROPERTY(QString colorScheme READ colorScheme WRITE setColorScheme)
    Q_PROPERTY(bool hasBorder READ hasBorder WRITE setHasBorder)

public:
    QString colorScheme() const;
    void setColorScheme(const QString& scheme);
    bool hasBorder() const;
    void setHasBorder(bool hasBorder);
    QColor backgroundColor() const;
    QColor borderColor() const;
    QColor textColor() const;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    explicit ExtendedToolButton(ExtendedButtonKind kind, QWidget* parent);
    void paintEvent(QPaintEvent* event) override;

private:
    ExtendedButtonRenderer* renderer_;
};

class FilledPushButton : public ExtendedPushButton {
    Q_OBJECT
public:
    explicit FilledPushButton(QWidget* parent = nullptr);
    explicit FilledPushButton(const QString& text, QWidget* parent = nullptr);
    explicit FilledPushButton(const QIcon& icon, const QString& text, QWidget* parent = nullptr);
    explicit FilledPushButton(const FluentIconBase& icon, const QString& text,
                              QWidget* parent = nullptr);
};

class FilledToolButton : public ExtendedToolButton {
    Q_OBJECT
public:
    explicit FilledToolButton(QWidget* parent = nullptr);
    explicit FilledToolButton(const QIcon& icon, QWidget* parent = nullptr);
    explicit FilledToolButton(const FluentIconBase& icon, QWidget* parent = nullptr);
};

class TextPushButton : public ExtendedPushButton {
    Q_OBJECT
public:
    explicit TextPushButton(QWidget* parent = nullptr);
    explicit TextPushButton(const QString& text, QWidget* parent = nullptr);
    explicit TextPushButton(const QIcon& icon, const QString& text, QWidget* parent = nullptr);
    explicit TextPushButton(const FluentIconBase& icon, const QString& text,
                            QWidget* parent = nullptr);
};

class TextToolButton : public ExtendedToolButton {
    Q_OBJECT
public:
    explicit TextToolButton(QWidget* parent = nullptr);
    explicit TextToolButton(const QIcon& icon, QWidget* parent = nullptr);
    explicit TextToolButton(const FluentIconBase& icon, QWidget* parent = nullptr);
};

class OutlinedPushButton : public ExtendedPushButton {
    Q_OBJECT
public:
    explicit OutlinedPushButton(QWidget* parent = nullptr);
    explicit OutlinedPushButton(const QString& text, QWidget* parent = nullptr);
    explicit OutlinedPushButton(const QIcon& icon, const QString& text, QWidget* parent = nullptr);
    explicit OutlinedPushButton(const FluentIconBase& icon, const QString& text,
                                QWidget* parent = nullptr);
};

class OutlinedToolButton : public ExtendedToolButton {
    Q_OBJECT
public:
    explicit OutlinedToolButton(QWidget* parent = nullptr);
    explicit OutlinedToolButton(const QIcon& icon, QWidget* parent = nullptr);
    explicit OutlinedToolButton(const FluentIconBase& icon, QWidget* parent = nullptr);
};

class RoundPushButton : public ExtendedPushButton {
    Q_OBJECT
public:
    explicit RoundPushButton(QWidget* parent = nullptr);
    explicit RoundPushButton(const QString& text, QWidget* parent = nullptr);
    explicit RoundPushButton(const QIcon& icon, const QString& text, QWidget* parent = nullptr);
    explicit RoundPushButton(const FluentIconBase& icon, const QString& text,
                             QWidget* parent = nullptr);
};

class RoundToolButton : public ExtendedToolButton {
    Q_OBJECT
public:
    explicit RoundToolButton(QWidget* parent = nullptr);
    explicit RoundToolButton(const QIcon& icon, QWidget* parent = nullptr);
    explicit RoundToolButton(const FluentIconBase& icon, QWidget* parent = nullptr);
};

class HyperlinkToolButton : public ExtendedToolButton {
    Q_OBJECT
    Q_PROPERTY(QUrl url READ url WRITE setUrl)
public:
    explicit HyperlinkToolButton(QWidget* parent = nullptr);
    explicit HyperlinkToolButton(const QIcon& icon, QWidget* parent = nullptr);
    explicit HyperlinkToolButton(const FluentIconBase& icon, QWidget* parent = nullptr);
    HyperlinkToolButton(const QIcon& icon, const QString& url, QWidget* parent = nullptr);
    HyperlinkToolButton(const FluentIconBase& icon, const QString& url, QWidget* parent = nullptr);
    QUrl url() const;
    void setUrl(const QUrl& url);
    void setUrl(const QString& url);

private:
    QUrl url_;
};

class LuminaPushButton : public ExtendedPushButton {
    Q_OBJECT
    Q_PROPERTY(QColor glowColor READ glowColor WRITE setGlowColor)
    Q_PROPERTY(int glowAlpha READ glowAlpha WRITE setGlowAlpha)
public:
    explicit LuminaPushButton(QWidget* parent = nullptr);
    explicit LuminaPushButton(const QString& text, QWidget* parent = nullptr);
    explicit LuminaPushButton(const QIcon& icon, const QString& text, QWidget* parent = nullptr);
    explicit LuminaPushButton(const FluentIconBase& icon, const QString& text,
                              QWidget* parent = nullptr);
    QColor glowColor() const;
    void setGlowColor(const QColor& color);
    int glowAlpha() const;
    void setGlowAlpha(int alpha);

protected:
    bool event(QEvent* event) override;

private:
    void updateGlow();
    void updateGlowColor();
    QGraphicsDropShadowEffect* glowEffect_ = nullptr;
    QPropertyAnimation* alphaAnimation_ = nullptr;
    QPropertyAnimation* blurAnimation_ = nullptr;
    bool customGlowColor_ = false;
    bool glowHovered_ = false;
};

}  // namespace qfw
