#pragma once

#include <QPointer>

#include "components/widgets/scroll_area.h"

class QFrame;
class QHBoxLayout;
class QLabel;
class QEvent;
class QPaintEvent;
class QVBoxLayout;
class QWidget;

namespace qfw {

class BodyLabel;
class CaptionLabel;
class IconWidget;
class PushButton;
class StrongBodyLabel;
class TitleLabel;
class ToolButton;

class GallerySeparatorWidget : public QWidget {
    Q_OBJECT

public:
    explicit GallerySeparatorWidget(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* e) override;
};

class ToolBar : public QWidget {
    Q_OBJECT

public:
    explicit ToolBar(const QString& title, const QString& subtitle, QWidget* parent = nullptr);

private:
    void initWidget();

    TitleLabel* titleLabel_ = nullptr;
    CaptionLabel* subtitleLabel_ = nullptr;

    PushButton* documentButton_ = nullptr;
    PushButton* sourceButton_ = nullptr;
    ToolButton* themeButton_ = nullptr;
    GallerySeparatorWidget* separator_ = nullptr;
    ToolButton* supportButton_ = nullptr;
    ToolButton* feedbackButton_ = nullptr;

    QPointer<QVBoxLayout> vBoxLayout_;
    QPointer<QHBoxLayout> buttonLayout_;
};

class ExampleCard : public QWidget {
    Q_OBJECT

public:
    explicit ExampleCard(const QString& title, QWidget* widget, const QString& sourcePath,
                         int stretch = 0, QWidget* parent = nullptr);

protected:
    bool eventFilter(QObject* obj, QEvent* e) override;

private:
    void initWidget();
    void initLayout();

    QWidget* widget_ = nullptr;
    int stretch_ = 0;

    StrongBodyLabel* titleLabel_ = nullptr;
    QFrame* card_ = nullptr;

    QFrame* sourceWidget_ = nullptr;
    QString sourcePath_;
    BodyLabel* sourcePathLabel_ = nullptr;
    IconWidget* linkIcon_ = nullptr;

    QPointer<QVBoxLayout> vBoxLayout_;
    QPointer<QVBoxLayout> cardLayout_;
    QPointer<QHBoxLayout> topLayout_;
    QPointer<QHBoxLayout> bottomLayout_;
};

class GalleryInterface : public ScrollArea {
    Q_OBJECT

public:
    explicit GalleryInterface(const QString& title, const QString& subtitle,
                              QWidget* parent = nullptr);

    ExampleCard* addExampleCard(const QString& title, QWidget* widget, const QString& sourcePath,
                                int stretch = 0);

    void scrollToCard(int index);

protected:
    QVBoxLayout* contentLayout() const;

protected:
    void resizeEvent(QResizeEvent* e) override;

private:
    QWidget* view_ = nullptr;
    ToolBar* toolBar_ = nullptr;
    QPointer<QVBoxLayout> vBoxLayout_;
};

}  // namespace qfw
