#pragma once

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QUrl>
#include <QVBoxLayout>

#include "common/icon.h"
#include "components/widgets/scroll_area.h"

namespace qfw {

class IconWidget;

// LinkCard - A clickable card with icon, title, content, and link icon
class LinkCard : public QFrame {
    Q_OBJECT

public:
    explicit LinkCard(const FluentIconBase& icon, const QString& title, const QString& content,
                      const QString& url, QWidget* parent = nullptr);
    explicit LinkCard(FluentIconEnum icon, const QString& title, const QString& content,
                      const QString& url, QWidget* parent = nullptr);
    explicit LinkCard(const QString& icon, const QString& title, const QString& content,
                      const QString& url, QWidget* parent = nullptr);

protected:
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    void initWidget();

    QUrl url_;
    IconWidget* iconWidget_ = nullptr;
    QLabel* titleLabel_ = nullptr;
    QLabel* contentLabel_ = nullptr;
    IconWidget* urlWidget_ = nullptr;
    QVBoxLayout* vBoxLayout_ = nullptr;
};

// LinkCardView - A horizontal scrollable container for LinkCards
class LinkCardView : public SingleDirectionScrollArea {
    Q_OBJECT

public:
    explicit LinkCardView(QWidget* parent = nullptr);

    void addCard(const FluentIconBase& icon, const QString& title, const QString& content,
                 const QString& url);
    void addCard(FluentIconEnum icon, const QString& title, const QString& content,
                 const QString& url);
    void addCard(const QString& icon, const QString& title, const QString& content,
                 const QString& url);

private:
    QWidget* view_ = nullptr;
    QHBoxLayout* hBoxLayout_ = nullptr;
};

}  // namespace qfw
