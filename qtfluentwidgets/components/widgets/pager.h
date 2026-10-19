#pragma once

#include <QAbstractButton>
#include <QList>
#include <QWidget>

#include "common/qtcompat.h"

class QHBoxLayout;
class QIntValidator;

namespace qfw {
class BodyLabel;
class LineEdit;
class TransparentToolButton;

class PageButton : public QAbstractButton {
    Q_OBJECT
    Q_PROPERTY(int page READ page WRITE setPage)
    Q_PROPERTY(bool selected READ isSelected WRITE setSelected)
public:
    explicit PageButton(int page, bool selected = false, QWidget* parent = nullptr);
    int page() const;
    void setPage(int page);
    void setSelected(bool selected);
    bool isSelected() const;
    QSize sizeHint() const override;
signals:
    void clicked(int page);

protected:
    void nextCheckState() override;
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(enterEvent_QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    int page_;
    bool hovered_ = false;
};

class Pager : public QWidget {
    Q_OBJECT
    Q_PROPERTY(int pages READ pages WRITE setPages)
    Q_PROPERTY(int currentPage READ currentPage WRITE setCurrentPage NOTIFY currentPageChanged)
    Q_PROPERTY(int maxVisible READ maxVisible WRITE setMaxVisible)
public:
    explicit Pager(QWidget* parent = nullptr);
    explicit Pager(int pages, int maxVisible = 5, QWidget* parent = nullptr);
    int pages() const;
    int currentPage() const;
    int maxVisible() const;
    void setPages(int pages);
    void setCurrentPage(int page);
    void setMaxVisible(int number);
    QList<int> visiblePages() const;
    PageButton* pageButton(int page) const;

    TransparentToolButton* topButton;
    TransparentToolButton* bottomButton;
    TransparentToolButton* previousButton;
    TransparentToolButton* nextButton;
    BodyLabel* topLabel;
    BodyLabel* bottomLabel;
    BodyLabel* jumpLabel;
    BodyLabel* countLabel;
    LineEdit* jumpEdit;
public slots:
    void jumpToPage();
signals:
    void currentPageChanged(int page);

protected:
    void changeEvent(QEvent* event) override;

private:
    void rebuild();
    void updateLabels();
    void updateDirection();
    int pages_;
    int currentPage_;
    int maxVisible_;
    QHBoxLayout* layout_;
    QIntValidator* validator_;
    QList<PageButton*> buttons_;
    QList<int> visiblePages_;
};
}  // namespace qfw
