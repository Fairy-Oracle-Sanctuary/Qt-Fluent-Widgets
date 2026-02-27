#pragma once

#include <QFrame>
#include <QListWidgetItem>
#include <QTableWidgetItem>
#include <QTreeWidgetItem>

#include "components/widgets/table_view.h"
#include "gallery_interface.h"

class QHBoxLayout;

namespace qfw {

class ListWidget;
class TreeWidget;
class HorizontalFlipView;

class ViewFrame : public QFrame {
    Q_OBJECT

public:
    explicit ViewFrame(QWidget* parent = nullptr);

    void addWidget(QWidget* widget);

protected:
    QHBoxLayout* hBoxLayout_ = nullptr;
};

class ListFrame : public ViewFrame {
    Q_OBJECT

public:
    explicit ListFrame(QWidget* parent = nullptr);

private:
    ListWidget* listWidget_ = nullptr;
};

class TreeFrame : public ViewFrame {
    Q_OBJECT

public:
    explicit TreeFrame(QWidget* parent = nullptr, bool enableCheck = false);

private:
    TreeWidget* tree_ = nullptr;
};

class TableFrame : public TableWidget {
    Q_OBJECT

public:
    explicit TableFrame(QWidget* parent = nullptr);
};

class ViewInterface : public GalleryInterface {
    Q_OBJECT

public:
    explicit ViewInterface(QWidget* parent = nullptr);
};

}  // namespace qfw
