#pragma once

#include <QButtonGroup>
#include <QLabel>
#include <QList>
#include <QString>
#include <QVariant>

#include "common/config.h"
#include "components/settings/expand_setting_card.h"

class QAbstractButton;

namespace qfw {

class OptionsSettingCard : public ExpandSettingCard {
    Q_OBJECT

public:
    explicit OptionsSettingCard(OptionsConfigItem* configItem, const QVariant& icon,
                                const QString& title, const QString& content = QString(),
                                const QStringList& texts = QStringList(),
                                QWidget* parent = nullptr);

    explicit OptionsSettingCard(OptionsConfigItem* configItem, FluentIconEnum icon,
                                const QString& title, const QString& content = QString(),
                                const QStringList& texts = QStringList(), QWidget* parent = nullptr)
        : OptionsSettingCard(configItem, QVariant::fromValue(FluentIcon(icon).qicon()), title,
                             content, texts, parent) {}

    explicit OptionsSettingCard(OptionsConfigItem* configItem, const FluentIconBase& icon,
                                const QString& title, const QString& content = QString(),
                                const QStringList& texts = QStringList(), QWidget* parent = nullptr)
        : OptionsSettingCard(configItem, QVariant::fromValue(icon.qicon()), title, content, texts,
                             parent) {}

    void setValue(const QVariant& value);

signals:
    void optionChanged(OptionsConfigItem* item);

private slots:
    void onButtonClicked(QAbstractButton* button);

private:
    void initWidget();

    OptionsConfigItem* configItem_;
    QString configName_;
    QLabel* choiceLabel_;
    QButtonGroup* buttonGroup_;
    QStringList texts_;
};

}  // namespace qfw
