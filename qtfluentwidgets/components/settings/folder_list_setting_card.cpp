#include "components/settings/folder_list_setting_card.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QSizePolicy>

#include "common/config.h"
#include "common/icon.h"
#include "common/style_sheet.h"
#include "components/dialog_box/dialog.h"

namespace qfw {

// ============================================================================
// FolderItem
// ============================================================================

FolderItem::FolderItem(const QString& folder, QWidget* parent)
    : QWidget(parent),
      folder_(folder),
      hBoxLayout_(new QHBoxLayout(this)),
      folderLabel_(new QLabel(folder, this)),
      removeButton_(new ToolButton(FluentIcon(FluentIconEnum::Close), this)) {
    initWidget();
}

void FolderItem::initWidget() {
    removeButton_->setFixedSize(39, 29);
    removeButton_->setIconSize(QSize(12, 12));

    setFixedHeight(53);
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
    hBoxLayout_->setContentsMargins(48, 0, 60, 0);
    hBoxLayout_->addWidget(folderLabel_, 0, Qt::AlignLeft);
    hBoxLayout_->addSpacing(16);
    hBoxLayout_->addStretch(1);
    hBoxLayout_->addWidget(removeButton_, 0, Qt::AlignRight);
    hBoxLayout_->setAlignment(Qt::AlignVCenter);

    folderLabel_->setProperty("qssClass", "SettingCardTitleLabel");

    connect(removeButton_, &ToolButton::clicked, this, [this]() { emit removed(this); });
}

// ============================================================================
// FolderListSettingCard
// ============================================================================

FolderListSettingCard::FolderListSettingCard(ConfigItem* configItem, const QString& title,
                                             const QString& content, const QString& directory,
                                             QWidget* parent)
    : ExpandSettingCard(QVariant::fromValue(FluentIcon(FluentIconEnum::Folder).qicon()), title,
                        content, parent),
      configItem_(configItem),
      dialogDirectory_(directory),
      addFolderButton_(
          new PushButton(FluentIcon(FluentIconEnum::FolderAdd), tr("Add folder"), this)) {
    // Get initial folders from config
    QVariant value = QConfig::instance().get(*configItem_);
    if (value.canConvert<QStringList>()) {
        folders_ = value.toStringList();
    } else if (value.canConvert<QString>()) {
        // Single folder as string
        folders_ = QStringList{value.toString()};
    }

    initWidget();
}

void FolderListSettingCard::initWidget() {
    addFolderButton_->setMaximumWidth(118);
    addWidget(addFolderButton_);

    // Initialize layout
    viewLayout_->setSpacing(0);
    viewLayout_->setAlignment(Qt::AlignTop);
    viewLayout_->setContentsMargins(0, 0, 0, 0);

    for (const QString& folder : folders_) {
        addFolderItem(folder);
    }

    connect(addFolderButton_, &PushButton::clicked, this,
            &FolderListSettingCard::onAddFolderButtonClicked);

    qfw::setStyleSheet(this, FluentStyleSheet::ExpandSettingCard);
}

void FolderListSettingCard::onAddFolderButtonClicked() {
    QString folder = QFileDialog::getExistingDirectory(this, tr("Choose folder"), dialogDirectory_);

    if (folder.isEmpty() || folders_.contains(folder)) {
        return;
    }

    addFolderItem(folder);
    folders_.append(folder);
    QConfig::instance().set(*configItem_, folders_);
    emit folderChanged(folders_);
}

void FolderListSettingCard::addFolderItem(const QString& folder) {
    auto* item = new FolderItem(folder, view_);
    connect(item, &FolderItem::removed, this, &FolderListSettingCard::onFolderItemRemoved);
    viewLayout_->addWidget(item);
    item->show();
    adjustViewSize();
}

void FolderListSettingCard::onFolderItemRemoved(FolderItem* item) { showConfirmDialog(item); }

void FolderListSettingCard::showConfirmDialog(FolderItem* item) {
    QString name = QFileInfo(item->folder()).fileName();
    QString title = tr("Are you sure you want to delete the folder?");
    QString content = tr("If you delete the ") + QStringLiteral("\"") + name +
                      QStringLiteral("\" ") +
                      tr("folder and remove it from the list, the folder will no "
                         "longer appear in the list, but will not be deleted.");

    auto* dialog = new Dialog(title, content, window());
    connect(dialog, &Dialog::yesSignal, this, [this, item]() { removeFolder(item); });
    dialog->exec();
}

void FolderListSettingCard::removeFolder(FolderItem* item) {
    if (!folders_.contains(item->folder())) {
        return;
    }

    folders_.removeOne(item->folder());
    viewLayout_->removeWidget(item);
    item->deleteLater();
    adjustViewSize();

    QConfig::instance().set(*configItem_, folders_);
    emit folderChanged(folders_);
}

}  // namespace qfw
