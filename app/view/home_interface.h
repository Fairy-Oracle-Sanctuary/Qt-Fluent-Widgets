#pragma once

#include <QPixmap>
#include <QPointer>

#include "components/widgets/scroll_area.h"

class QLabel;
class QVBoxLayout;
class QWidget;

namespace qfw {

class LinkCardView;
class SampleCardView;

class BannerWidget : public QWidget {
    Q_OBJECT

public:
    explicit BannerWidget(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* e) override;

private:
    QLabel* galleryLabel_ = nullptr;
    LinkCardView* linkCardView_ = nullptr;
    QPointer<QVBoxLayout> vBoxLayout_;
    QPixmap banner_;
};

class HomeInterface : public ScrollArea {
    Q_OBJECT

public:
    explicit HomeInterface(QWidget* parent = nullptr);

private:
    void initWidget();
    void loadSamples();

    BannerWidget* banner_ = nullptr;
    QWidget* view_ = nullptr;
    QPointer<QVBoxLayout> vBoxLayout_;
};

}  // namespace qfw
