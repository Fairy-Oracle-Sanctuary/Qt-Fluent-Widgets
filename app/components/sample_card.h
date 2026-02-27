#pragma once

#include <QLabel>
#include <QVBoxLayout>

#include "components/layout/flow_layout.h"
#include "components/widgets/card_widget.h"

namespace qfw {

// SampleCard - A clickable card showing a sample with icon, title, and content
class SampleCard : public CardWidget {
    Q_OBJECT

public:
    explicit SampleCard(const FluentIconBase& icon, const QString& title, const QString& content,
                        const QString& routeKey, int index, QWidget* parent = nullptr);
    explicit SampleCard(FluentIconEnum icon, const QString& title, const QString& content,
                        const QString& routeKey, int index, QWidget* parent = nullptr);
    explicit SampleCard(const QString& icon, const QString& title, const QString& content,
                        const QString& routeKey, int index, QWidget* parent = nullptr);

    QString routeKey() const { return routeKey_; }
    int index() const { return index_; }

protected:
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    void initWidget();

    QString routeKey_;
    int index_ = 0;
    IconWidget* iconWidget_ = nullptr;
    QLabel* titleLabel_ = nullptr;
    QLabel* contentLabel_ = nullptr;
    QHBoxLayout* hBoxLayout_ = nullptr;
    QVBoxLayout* vBoxLayout_ = nullptr;
};

// SampleCardView - A container for sample cards with flow layout
class SampleCardView : public QWidget {
    Q_OBJECT

public:
    explicit SampleCardView(const QString& title, QWidget* parent = nullptr);

    void addSampleCard(const FluentIconBase& icon, const QString& title, const QString& content,
                       const QString& routeKey, int index);
    void addSampleCard(FluentIconEnum icon, const QString& title, const QString& content,
                       const QString& routeKey, int index);
    void addSampleCard(const QString& icon, const QString& title, const QString& content,
                       const QString& routeKey, int index);

private:
    QLabel* titleLabel_ = nullptr;
    QVBoxLayout* vBoxLayout_ = nullptr;
    FlowLayout* flowLayout_ = nullptr;
};

}  // namespace qfw
