#pragma once

#include <QHBoxLayout>
#include <QLabel>
#include <QList>
#include <QString>
#include <QWidget>

#include "components/settings/expand_setting_card.h"
#include "components/widgets/button.h"
#include "common/config.h"

namespace qfw {

class FolderItem : public QWidget {
    Q_OBJECT

public:
    explicit FolderItem(const QString& folder, QWidget* parent = nullptr);

    const QString& folder() const { return folder_; }

signals:
    void removed(FolderItem* item);

private:
    void initWidget();

    QString folder_;
    QHBoxLayout* hBoxLayout_;
    QLabel* folderLabel_;
    ToolButton* removeButton_;
};

class FolderListSettingCard : public ExpandSettingCard {
    Q_OBJECT

public:
    explicit FolderListSettingCard(ConfigItem* configItem, const QString& title,
                                   const QString& content = QString(),
                                   const QString& directory = QStringLiteral("./"),
                                   QWidget* parent = nullptr);

    QStringList folders() const { return folders_; }

signals:
    void folderChanged(const QStringList& folders);

private slots:
    void onAddFolderButtonClicked();
    void onFolderItemRemoved(FolderItem* item);

private:
    void initWidget();
    void addFolderItem(const QString& folder);
    void showConfirmDialog(FolderItem* item);
    void removeFolder(FolderItem* item);

    ConfigItem* configItem_;
    QString dialogDirectory_;
    PushButton* addFolderButton_;
    QStringList folders_;
};

} // namespace qfw
