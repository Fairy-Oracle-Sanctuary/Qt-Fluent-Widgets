#pragma once

#include <QColor>
#include <QImage>
#include <QLabel>
#include <QPointer>
#include <QPushButton>
#include <QUrl>

class QMovie;

namespace qfw {

class PixmapLabel : public QLabel {
    Q_OBJECT

public:
    explicit PixmapLabel(QWidget* parent = nullptr);

    void setPixmap(const QPixmap& pixmap);
    QPixmap pixmap() const;

protected:
    void paintEvent(QPaintEvent* e) override;

private:
    QPixmap pixmap_;
};

class FluentLabelBase : public QLabel {
    Q_OBJECT

public:
    explicit FluentLabelBase(QWidget* parent = nullptr);
    explicit FluentLabelBase(const QString& text, QWidget* parent = nullptr);

    void setTextColor(const QColor& light = QColor(0, 0, 0),
                      const QColor& dark = QColor(255, 255, 255));

    QColor lightColor() const;
    void setLightColor(const QColor& c);

    QColor darkColor() const;
    void setDarkColor(const QColor& c);

    int pixelFontSize() const;
    void setPixelFontSize(int size);

    bool strikeOut() const;
    void setStrikeOut(bool v);

    bool underline() const;
    void setUnderline(bool v);

protected:
    virtual QFont getFont() const = 0;
    void init();

private slots:
    void onContextMenuRequested(const QPoint& pos);

private:
    QColor lightColor_;
    QColor darkColor_;
};

class CaptionLabel : public FluentLabelBase {
    Q_OBJECT

public:
    explicit CaptionLabel(QWidget* parent = nullptr);
    explicit CaptionLabel(const QString& text, QWidget* parent = nullptr);

protected:
    QFont getFont() const override;
};

class BodyLabel : public FluentLabelBase {
    Q_OBJECT

public:
    explicit BodyLabel(QWidget* parent = nullptr);
    explicit BodyLabel(const QString& text, QWidget* parent = nullptr);

protected:
    QFont getFont() const override;
};

class StrongBodyLabel : public FluentLabelBase {
    Q_OBJECT

public:
    explicit StrongBodyLabel(QWidget* parent = nullptr);
    explicit StrongBodyLabel(const QString& text, QWidget* parent = nullptr);

protected:
    QFont getFont() const override;
};

class SubtitleLabel : public FluentLabelBase {
    Q_OBJECT

public:
    explicit SubtitleLabel(QWidget* parent = nullptr);
    explicit SubtitleLabel(const QString& text, QWidget* parent = nullptr);

protected:
    QFont getFont() const override;
};

class TitleLabel : public FluentLabelBase {
    Q_OBJECT

public:
    explicit TitleLabel(QWidget* parent = nullptr);
    explicit TitleLabel(const QString& text, QWidget* parent = nullptr);

protected:
    QFont getFont() const override;
};

class LargeTitleLabel : public FluentLabelBase {
    Q_OBJECT

public:
    explicit LargeTitleLabel(QWidget* parent = nullptr);
    explicit LargeTitleLabel(const QString& text, QWidget* parent = nullptr);

protected:
    QFont getFont() const override;
};

class DisplayLabel : public FluentLabelBase {
    Q_OBJECT

public:
    explicit DisplayLabel(QWidget* parent = nullptr);
    explicit DisplayLabel(const QString& text, QWidget* parent = nullptr);

protected:
    QFont getFont() const override;
};

class ImageLabel : public QLabel {
    Q_OBJECT
    Q_PROPERTY(int topLeftRadius READ topLeftRadius WRITE setTopLeftRadius)
    Q_PROPERTY(int topRightRadius READ topRightRadius WRITE setTopRightRadius)
    Q_PROPERTY(int bottomLeftRadius READ bottomLeftRadius WRITE setBottomLeftRadius)
    Q_PROPERTY(int bottomRightRadius READ bottomRightRadius WRITE setBottomRightRadius)

public:
    explicit ImageLabel(QWidget* parent = nullptr);
    explicit ImageLabel(const QString& imagePath, QWidget* parent = nullptr);
    explicit ImageLabel(const QImage& image, QWidget* parent = nullptr);
    explicit ImageLabel(const QPixmap& pixmap, QWidget* parent = nullptr);

    void setBorderRadius(int topLeft, int topRight, int bottomLeft, int bottomRight);

    void setImage(const QString& imagePath);
    void setImage(const QImage& image);
    void setImage(const QPixmap& pixmap);

    void scaledToWidth(int width);
    void scaledToHeight(int height);
    void setScaledSize(const QSize& size);

    bool isNull() const;

    QPixmap pixmap() const;

    int topLeftRadius() const;
    void setTopLeftRadius(int radius);

    int topRightRadius() const;
    void setTopRightRadius(int radius);

    int bottomLeftRadius() const;
    void setBottomLeftRadius(int radius);

    int bottomRightRadius() const;
    void setBottomRightRadius(int radius);

signals:
    void clicked();

protected:
    void mouseReleaseEvent(QMouseEvent* e) override;
    void paintEvent(QPaintEvent* e) override;

    virtual void postInit();

private slots:
    void onFrameChanged(int index);

private:
    void setMovie(QMovie* movie);

    QImage image_;
    QPointer<QMovie> movie_;

    int topLeftRadius_ = 0;
    int topRightRadius_ = 0;
    int bottomLeftRadius_ = 0;
    int bottomRightRadius_ = 0;
};

class AvatarWidget : public ImageLabel {
    Q_OBJECT
    Q_PROPERTY(int radius READ radius WRITE setRadius)

public:
    explicit AvatarWidget(QWidget* parent = nullptr);
    explicit AvatarWidget(const QString& imagePath, QWidget* parent = nullptr);
    explicit AvatarWidget(const QImage& image, QWidget* parent = nullptr);
    explicit AvatarWidget(const QPixmap& pixmap, QWidget* parent = nullptr);

    int radius() const;
    void setRadius(int radius);

    void setBackgroundColor(const QColor& light, const QColor& dark);

    void setImage(const QString& imagePath);
    void setImage(const QImage& image);
    void setImage(const QPixmap& pixmap);

protected:
    void postInit() override;
    void paintEvent(QPaintEvent* e) override;

private:
    void drawImageAvatar(QPainter& painter);
    void drawTextAvatar(QPainter& painter);

    int radius_ = 48;
    QColor lightBackgroundColor_;
    QColor darkBackgroundColor_;
};

class HyperlinkLabel : public QPushButton {
    Q_OBJECT
    Q_PROPERTY(QUrl url READ url WRITE setUrl)
    Q_PROPERTY(bool underlineVisible READ underlineVisible WRITE setUnderlineVisible)

public:
    explicit HyperlinkLabel(QWidget* parent = nullptr);
    explicit HyperlinkLabel(const QString& text, QWidget* parent = nullptr);
    explicit HyperlinkLabel(const QUrl& url, const QString& text, QWidget* parent = nullptr);

    QUrl url() const;
    void setUrl(const QUrl& url);
    void setUrl(const QString& url);

    bool underlineVisible() const;
    void setUnderlineVisible(bool visible);

private slots:
    void onClicked();

private:
    QUrl url_;
    bool underlineVisible_ = false;
};

}  // namespace qfw
