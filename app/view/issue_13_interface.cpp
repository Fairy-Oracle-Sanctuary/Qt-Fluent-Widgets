#include "view/issue_13_interface.h"

#include <QList>
#include <QPair>
#include <QVBoxLayout>
#include <QWidget>

#include "common/icon.h"
#include "components/layout/flow_layout.h"
#include "components/widgets/card_widget.h"
#include "components/widgets/icon_widget.h"
#include "components/widgets/label.h"

namespace qfw {

namespace {

class Issue13Card : public ElevatedCardWidget {
public:
    Issue13Card(FluentIconEnum icon, const QString& title, QWidget* parent = nullptr)
        : ElevatedCardWidget(parent) {
        auto* iconWidget = new IconWidget(FluentIcon(icon), this);
        auto* titleLabel = new CaptionLabel(title, this);
        auto* layout = new QVBoxLayout(this);

        iconWidget->setFixedSize(56, 56);
        layout->setContentsMargins(16, 18, 16, 14);
        layout->setSpacing(10);
        layout->addStretch(1);
        layout->addWidget(iconWidget, 0, Qt::AlignCenter);
        layout->addWidget(titleLabel, 0, Qt::AlignHCenter | Qt::AlignBottom);

        setFixedSize(150, 140);
    }
};

class Issue13DemoWidget : public QWidget {
public:
    explicit Issue13DemoWidget(QWidget* parent = nullptr) : QWidget(parent) {
        auto* layout = new FlowLayout(this, true, true);
        layout->setContentsMargins(8, 8, 8, 8);
        layout->setHorizontalSpacing(16);
        layout->setVerticalSpacing(16);
        layout->setAnimation(300, QEasingCurve::OutCubic);

        const QList<QPair<FluentIconEnum, QString>> cards = {
            {FluentIconEnum::Game, tr("Game")},
            {FluentIconEnum::Music, tr("Music")},
            {FluentIconEnum::Photo, tr("Photo")},
            {FluentIconEnum::Movie, tr("Movie")},
            {FluentIconEnum::Cloud, tr("Cloud")},
            {FluentIconEnum::Folder, tr("Folder")},
            {FluentIconEnum::Code, tr("Code")},
            {FluentIconEnum::Palette, tr("Palette")},
        };

        for (const auto& card : cards) {
            layout->addWidget(new Issue13Card(card.first, card.second, this));
        }

        setMinimumHeight(320);
    }
};

}  // namespace

Issue13Interface::Issue13Interface(QWidget* parent)
    : GalleryInterface(tr("Issue #13"),
                       tr("Resize the window while hovering an elevated card to verify that "
                          "layout and hover animations no longer fight over its position."),
                       parent) {
    setObjectName(QStringLiteral("issue13Interface"));

    addExampleCard(tr("Animated FlowLayout with elevated cards"), new Issue13DemoWidget(this),
                   QStringLiteral("https://github.com/Fairy-Oracle-Sanctuary/"
                                  "Qt-Fluent-Widgets/issues/13"),
                   1);
}

}  // namespace qfw
