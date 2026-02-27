#pragma once

#include <QMap>
#include <QObject>
#include <QString>
#include <QVector>
#include <QWidget>

class QFont;

namespace qfw {

class BreadcrumbWidget : public QWidget {
    Q_OBJECT

public:
    explicit BreadcrumbWidget(QWidget* parent = nullptr);

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void enterEvent(QEnterEvent* e) override;
    void leaveEvent(QEvent* e) override;

protected:
    bool isHover_ = false;
    bool isPressed_ = false;
};

class ElideButton : public BreadcrumbWidget {
    Q_OBJECT

public:
    explicit ElideButton(QWidget* parent = nullptr);

    void clearState();

protected:
    void paintEvent(QPaintEvent* e) override;
};

class BreadcrumbItem : public BreadcrumbWidget {
    Q_OBJECT

public:
    BreadcrumbItem(const QString& routeKey, const QString& text, int index, QWidget* parent = nullptr);

    QString routeKey() const { return routeKey_; }
    QString text() const { return text_; }

    void setText(const QString& text);

    bool isRoot() const;

    void setSelected(bool selected);

    void setSpacing(int spacing);
    int spacing() const { return spacing_; }

    void setFont(const QFont& font);

protected:
    void paintEvent(QPaintEvent* e) override;

private:
    QString text_;
    QString routeKey_;
    bool isSelected_ = false;
    int index_ = 0;
    int spacing_ = 5;
};

class BreadcrumbBar : public QWidget {
    Q_OBJECT

    Q_PROPERTY(int spacing READ spacing WRITE setSpacing)

public:
    explicit BreadcrumbBar(QWidget* parent = nullptr);

    void addItem(const QString& routeKey, const QString& text);

    void setCurrentIndex(int index);
    void setCurrentItem(const QString& routeKey);

    void setItemText(const QString& routeKey, const QString& text);

    BreadcrumbItem* item(const QString& routeKey) const;
    BreadcrumbItem* itemAt(int index) const;

    int currentIndex() const { return currentIndex_; }
    BreadcrumbItem* currentItem() const;

    void clear();
    void popItem();
    int count() const;

    int spacing() const { return spacing_; }
    void setSpacing(int spacing);

signals:
    void currentItemChanged(const QString& routeKey);
    void currentIndexChanged(int index);

protected:
    void resizeEvent(QResizeEvent* e) override;
    void setFont(const QFont& font);

private slots:
    void showHiddenItemsMenu();

private:
    void updateGeometryInternal();
    bool isElideVisible() const;

private:
    QMap<QString, BreadcrumbItem*> itemMap_;
    QVector<BreadcrumbItem*> items_;
    QVector<BreadcrumbItem*> hiddenItems_;

    int spacing_ = 10;
    int currentIndex_ = -1;

    ElideButton* elideButton_ = nullptr;
};

}  // namespace qfw
