#pragma once

#include <QList>
#include <QPointer>

#include "common/icon.h"
#include "components/widgets/icon_widget.h"
#include "components/widgets/scroll_area.h"
#include "view/gallery_interface.h"

class QFrame;
class QHBoxLayout;
class QLabel;
class QMouseEvent;
class QVBoxLayout;
class QWidget;

namespace qfw {

class FlowLayout;
class SearchLineEdit;
class SmoothScrollArea;
class StrongBodyLabel;
class Trie;

class IconSmoothScrollArea : public SmoothScrollArea {
    Q_OBJECT

public:
    using SmoothScrollArea::SmoothScrollArea;

    void setViewportMarginsPublic(int left, int top, int right, int bottom) {
        setViewportMargins(left, top, right, bottom);
    }
};

class IconCard : public QFrame {
    Q_OBJECT
    Q_PROPERTY(bool isSelected READ isSelected WRITE setSelected)

public:
    explicit IconCard(FluentIconEnum icon, QWidget* parent = nullptr);

    bool isSelected() const { return isSelected_; }

public slots:
    void setSelected(bool isSelected);
    void setSelected(bool isSelected, bool force);

signals:
    void clicked(qfw::FluentIconEnum icon);

protected:
    void mouseReleaseEvent(QMouseEvent* e) override;

private:
    FluentIconEnum icon_;
    bool isSelected_ = false;

    IconWidget* iconWidget_ = nullptr;
    QLabel* nameLabel_ = nullptr;
    QPointer<QVBoxLayout> vBoxLayout_;
};

class IconInfoPanel : public QFrame {
    Q_OBJECT

public:
    explicit IconInfoPanel(FluentIconEnum icon, QWidget* parent = nullptr);

public slots:
    void setIcon(qfw::FluentIconEnum icon);

private:
    QLabel* nameLabel_ = nullptr;
    IconWidget* iconWidget_ = nullptr;
    QLabel* iconNameTitleLabel_ = nullptr;
    QLabel* iconNameLabel_ = nullptr;
    QLabel* enumNameTitleLabel_ = nullptr;
    QLabel* enumNameLabel_ = nullptr;

    QPointer<QVBoxLayout> vBoxLayout_;
};

class IconCardView : public QWidget {
    Q_OBJECT

public:
    explicit IconCardView(QWidget* parent = nullptr);

private slots:
    void onSearchTextChanged(const QString& text);
    void setSelectedIcon(qfw::FluentIconEnum icon);
    void showAllIcons();

private:
    void initWidget();
    void applyQss();
    void addIcon(qfw::FluentIconEnum icon);

    Trie* trie_ = nullptr;

    StrongBodyLabel* iconLibraryLabel_ = nullptr;
    SearchLineEdit* searchLineEdit_ = nullptr;

    QFrame* view_ = nullptr;
    IconSmoothScrollArea* scrollArea_ = nullptr;
    QWidget* scrollWidget_ = nullptr;
    IconInfoPanel* infoPanel_ = nullptr;

    QPointer<QVBoxLayout> vBoxLayout_;
    QPointer<QHBoxLayout> hBoxLayout_;
    FlowLayout* flowLayout_ = nullptr;

    QList<IconCard*> cards_;
    QList<FluentIconEnum> icons_;
    int currentIndex_ = -1;
};

class IconInterface : public GalleryInterface {
    Q_OBJECT

public:
    explicit IconInterface(QWidget* parent = nullptr);

private:
    IconCardView* iconView_ = nullptr;
};

}  // namespace qfw
