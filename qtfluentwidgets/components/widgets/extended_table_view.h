#pragma once

#include "components/widgets/table_view.h"

namespace qfw {

class RoundTableWidget : public TableWidget {
    Q_OBJECT
public:
    explicit RoundTableWidget(QWidget* parent = nullptr);
};
class RoundTableView : public TableView {
    Q_OBJECT
public:
    explicit RoundTableView(QWidget* parent = nullptr);
};
class LineTableWidget : public TableWidget {
    Q_OBJECT
public:
    explicit LineTableWidget(QWidget* parent = nullptr);
};
class LineTableView : public TableView {
    Q_OBJECT
public:
    explicit LineTableView(QWidget* parent = nullptr);
};

}  // namespace qfw
