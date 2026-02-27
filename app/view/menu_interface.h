#pragma once

#include <QActionGroup>
#include <QPoint>
#include <QWidget>

#include "view/gallery_interface.h"

class QLabel;
class QVBoxLayout;

namespace qfw {

class Action;
class AvatarWidget;
class BodyLabel;
class CaptionLabel;
class CheckableMenu;
class CommandBar;
class CommandBarView;
class HyperlinkButton;
class ImageLabel;
class PushButton;
class RoundMenu;
class TransparentDropDownPushButton;

class ProfileCard : public QWidget {
    Q_OBJECT

public:
    explicit ProfileCard(const QString& avatarPath, const QString& name, const QString& email,
                        QWidget* parent = nullptr);

private:
    AvatarWidget* avatar_ = nullptr;
    BodyLabel* nameLabel_ = nullptr;
    CaptionLabel* emailLabel_ = nullptr;
    HyperlinkButton* logoutButton_ = nullptr;
};

class MenuInterface : public GalleryInterface {
    Q_OBJECT

public:
    explicit MenuInterface(QWidget* parent = nullptr);

private slots:
    void onButton1Clicked();
    void onButton2Clicked();
    void onButton3Clicked();
    void onImageLabelClicked();
    void onSaveImage();

private:
    void createActions();
    RoundMenu* createMenu(const QPoint& pos);
    RoundMenu* createCustomWidgetMenu(const QPoint& pos);
    CheckableMenu* createCheckableMenu(const QPoint& pos = QPoint());
    CommandBar* createCommandBar();
    void createCommandBarFlyout();

    PushButton* button1_ = nullptr;
    PushButton* button2_ = nullptr;
    PushButton* button3_ = nullptr;
    ImageLabel* imageLabel_ = nullptr;

    Action* createTimeAction_ = nullptr;
    Action* shootTimeAction_ = nullptr;
    Action* modifiedTimeAction_ = nullptr;
    Action* nameAction_ = nullptr;
    QActionGroup* actionGroup1_ = nullptr;

    Action* ascendAction_ = nullptr;
    Action* descendAction_ = nullptr;
    QActionGroup* actionGroup2_ = nullptr;
};

}  // namespace qfw
