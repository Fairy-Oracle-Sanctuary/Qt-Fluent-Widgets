#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>

namespace qfw {

class ExpandLayout;

class SettingCardGroup : public QWidget {
    Q_OBJECT

public:
    explicit SettingCardGroup(const QString& title, QWidget* parent = nullptr);

    void addSettingCard(QWidget* card);
    void addSettingCards(const QList<QWidget*>& cards);

    void adjustSize();

private:
    QLabel* titleLabel_;
    QVBoxLayout* vBoxLayout_;
    ExpandLayout* cardLayout_;
};

} // namespace qfw
