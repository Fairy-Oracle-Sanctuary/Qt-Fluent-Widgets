#include "window/splash_screen.h"

#include <QEvent>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QResizeEvent>

#include "common/config.h"
#include "common/icon.h"
#include "common/style_sheet.h"
#include "components/widgets/icon_widget.h"
#include "components/window/title_bar.h"

namespace qfw {

SplashScreen::SplashScreen(const QIcon& icon, QWidget* parent, bool enableShadow)
    : QWidget(parent) {
    initialize(QVariant::fromValue(icon), enableShadow);
}

SplashScreen::SplashScreen(const QString& icon, QWidget* parent, bool enableShadow)
    : QWidget(parent) {
    initialize(QVariant::fromValue(icon), enableShadow);
}

SplashScreen::SplashScreen(const FluentIconBase& icon, QWidget* parent, bool enableShadow)
    : QWidget(parent) {
    icon_ = QVariant::fromValue(icon.icon());
    titleBar_ = new TitleBar(this);
    // Keep the fluent icon so IconWidget can render it theme-aware.
    iconWidget_ = new IconWidget(icon, this);
    setup(enableShadow);
}

void SplashScreen::initialize(const QVariant& icon, bool enableShadow) {
    icon_ = icon;
    titleBar_ = new TitleBar(this);
    iconWidget_ = new IconWidget(this);
    iconWidget_->setIcon(icon);
    setup(enableShadow);
}

void SplashScreen::setup(bool enableShadow) {
    shadowEffect_ = new QGraphicsDropShadowEffect(this);

    iconWidget_->setFixedSize(iconSize_);
    shadowEffect_->setColor(QColor(0, 0, 0, 50));
    shadowEffect_->setBlurRadius(15);
    shadowEffect_->setOffset(0, 4);

    qfw::setStyleSheet(titleBar_, qfw::FluentStyleSheet::FluentWindow);

    if (enableShadow) {
        iconWidget_->setGraphicsEffect(shadowEffect_);
    }

    if (parentWidget()) {
        parentWidget()->installEventFilter(this);
    }

#ifdef Q_OS_MACOS
    titleBar_->hide();
#endif
}

void SplashScreen::setIcon(const QIcon& icon) {
    // Mirrors the Python implementation: only the stored icon is replaced and
    // the widget is repainted, the icon widget itself is left untouched.
    icon_ = QVariant::fromValue(icon);
    update();
}

void SplashScreen::setIcon(const QString& icon) {
    icon_ = QVariant::fromValue(icon);
    update();
}

void SplashScreen::setIcon(const FluentIconBase& icon) {
    icon_ = QVariant::fromValue(icon.icon());
    update();
}

QIcon SplashScreen::icon() const {
    if (icon_.canConvert<QIcon>()) {
        return icon_.value<QIcon>();
    }

    if (icon_.canConvert<QString>()) {
        return QIcon(icon_.value<QString>());
    }

    return QIcon();
}

void SplashScreen::setIconSize(const QSize& size) {
    iconSize_ = size;
    if (iconWidget_) {
        iconWidget_->setFixedSize(size);
    }
    update();
}

QSize SplashScreen::iconSize() const { return iconSize_; }

void SplashScreen::setTitleBar(QWidget* titleBar) {
    if (titleBar_) {
        titleBar_->deleteLater();
    }

    titleBar_ = titleBar;
    if (!titleBar_) {
        return;
    }

    titleBar_->setParent(this);
    titleBar_->raise();
    titleBar_->resize(width(), titleBar_->height());
}

QWidget* SplashScreen::titleBar() const { return titleBar_; }

bool SplashScreen::eventFilter(QObject* obj, QEvent* e) {
    if (obj == parent()) {
        if (e->type() == QEvent::Resize) {
            resize(static_cast<QResizeEvent*>(e)->size());
        } else if (e->type() == QEvent::ChildAdded) {
            raise();
        }
    }

    return QWidget::eventFilter(obj, e);
}

void SplashScreen::resizeEvent(QResizeEvent* e) {
    QWidget::resizeEvent(e);

    const int iw = iconSize_.width();
    const int ih = iconSize_.height();
    if (iconWidget_) {
        iconWidget_->move(width() / 2 - iw / 2, height() / 2 - ih / 2);
    }

    if (titleBar_) {
        titleBar_->resize(width(), titleBar_->height());
    }
}

void SplashScreen::finish() { close(); }

void SplashScreen::paintEvent(QPaintEvent* e) {
    Q_UNUSED(e);

    QPainter painter(this);
    painter.setPen(Qt::NoPen);

    // draw background
    const int c = isDarkTheme() ? 32 : 255;
    painter.setBrush(QColor(c, c, c));
    painter.drawRect(rect());
}

}  // namespace qfw
