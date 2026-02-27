#pragma once

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>

#include "../widgets/scroll_area.h"
#include "mask_dialog_base.h"

namespace qfw {

class ClickableWindow : public QWidget {
    Q_OBJECT

public:
    explicit ClickableWindow(QWidget* parent = nullptr);

signals:
    void clicked();

protected:
    void enterEvent(QEnterEvent* e) override;
    void leaveEvent(QEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void paintEvent(QPaintEvent* e) override;

    bool isPressed = false;
    bool isEnter = false;
};

class FolderCard : public ClickableWindow {
    Q_OBJECT

public:
    explicit FolderCard(const QString& folderPath, QWidget* parent = nullptr);
    QString folderPath;
    QString folderName;

protected:
    void paintEvent(QPaintEvent* e) override;

private:
    void drawText(QPainter* painter, int x1, int fontSize1, int x2, int fontSize2);
    QPixmap closeIcon;
};

class AddFolderCard : public ClickableWindow {
    Q_OBJECT

public:
    explicit AddFolderCard(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* e) override;

private:
    QPixmap iconPix;
};

/**
 * @brief Folder list dialog box
 */
class FolderListDialog : public MaskDialogBase {
    Q_OBJECT

public:
    explicit FolderListDialog(const QStringList& folderPaths, const QString& title,
                              const QString& content, QWidget* parent = nullptr);
    ~FolderListDialog() = default;

    QPushButton* completeButton;

signals:
    void folderChanged(const QStringList& folderPaths);

private slots:
    void showFileDialog();
    void onButtonClicked();
    void showDeleteFolderCardDialog();

private:
    void initWidget();
    void initLayout();
    void setQss();
    void adjustWidgetSize();
    void deleteFolderCard(FolderCard* folderCard);

    QString title;
    QString content;
    QStringList originalPaths;
    QStringList folderPaths;

    QVBoxLayout* vBoxLayout;
    QLabel* titleLabel;
    QLabel* contentLabel;
    SingleDirectionScrollArea* scrollArea;
    QWidget* scrollWidget;
    QVBoxLayout* scrollLayout;
    AddFolderCard* addFolderCard;
    QList<FolderCard*> folderCards;
};

}  // namespace qfw
