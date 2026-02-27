#include "folder_list_dialog.h"

#include <QApplication>
#include <QPainterPath>
#include <QScrollBar>

#include "../../common/config.h"
#include "../../common/icon.h"
#include "../../common/style_sheet.h"
#include "dialog.h"

namespace qfw {

ClickableWindow::ClickableWindow(QWidget* parent) : QWidget(parent) {
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(292, 72);
}

void ClickableWindow::enterEvent(QEnterEvent* e) {
    isEnter = true;
    update();
}

void ClickableWindow::leaveEvent(QEvent* e) {
    isEnter = false;
    update();
}

void ClickableWindow::mousePressEvent(QMouseEvent* e) {
    isPressed = true;
    update();
}

void ClickableWindow::mouseReleaseEvent(QMouseEvent* e) {
    isPressed = false;
    update();
    if (rect().contains(e->pos()) && e->button() == Qt::LeftButton) {
        emit clicked();
    }
}

void ClickableWindow::paintEvent(QPaintEvent* e) {
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);

    bool isDark = isDarkTheme();
    int bg = isDark ? 51 : 204;
    QBrush brush(QColor(bg, bg, bg));
    painter.setPen(Qt::NoPen);

    if (!isEnter) {
        painter.setBrush(brush);
        painter.drawRoundedRect(rect(), 4, 4);
    } else {
        painter.setPen(QPen(QColor(bg, bg, bg), 2));
        painter.drawRect(1, 1, width() - 2, height() - 2);
        painter.setPen(Qt::NoPen);
        if (!isPressed) {
            bg = isDark ? 24 : 230;
            brush.setColor(QColor(bg, bg, bg));
            painter.setBrush(brush);
            painter.drawRect(2, 2, width() - 4, height() - 4);
        } else {
            bg = isDark ? 102 : 230;
            brush.setColor(QColor(153, 153, 153));
            painter.setBrush(brush);
            painter.drawRoundedRect(5, 1, width() - 10, height() - 2, 2, 2);
        }
    }
}

FolderCard::FolderCard(const QString& folderPath, QWidget* parent)
    : ClickableWindow(parent), folderPath(folderPath) {
    folderName = QFileInfo(folderPath).fileName();
    if (folderName.isEmpty()) {
        folderName = folderPath;
    }
    QString c = getIconColor();
    closeIcon = QPixmap(QString(":/qfluentwidgets/images/folder_list_dialog/Close_%1.png").arg(c))
                    .scaled(12, 12, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

void FolderCard::paintEvent(QPaintEvent* e) {
    ClickableWindow::paintEvent(e);
    QPainter painter(this);
    painter.setRenderHints(QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform |
                           QPainter::Antialiasing);

    QColor color = isDarkTheme() ? Qt::white : Qt::black;
    painter.setPen(color);
    if (isPressed) {
        drawText(&painter, 12, 12, 12, 10);
        painter.drawPixmap(width() - 26, 18, closeIcon);
    } else {
        drawText(&painter, 10, 13, 10, 11);
        painter.drawPixmap(width() - 24, 20, closeIcon);
    }
}

void FolderCard::drawText(QPainter* painter, int x1, int fontSize1, int x2, int fontSize2) {
    QFont font("Microsoft YaHei");
    font.setBold(true);
    font.setPixelSize(fontSize1);
    painter->setFont(font);
    QString name = QFontMetrics(font).elidedText(folderName, Qt::ElideRight, width() - 48);
    painter->drawText(x1, 30, name);

    font.setBold(false);
    font.setPixelSize(fontSize2);
    painter->setFont(font);
    QString path = QFontMetrics(font).elidedText(folderPath, Qt::ElideRight, width() - 24);
    painter->drawText(QRect(x2, 37, width() - 16, 18), Qt::AlignLeft, path);
}

AddFolderCard::AddFolderCard(QWidget* parent) : ClickableWindow(parent) {
    QString c = getIconColor();
    iconPix = QPixmap(QString(":/qfluentwidgets/images/folder_list_dialog/Add_%1.png").arg(c))
                  .scaled(22, 22, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

void AddFolderCard::paintEvent(QPaintEvent* e) {
    ClickableWindow::paintEvent(e);
    QPainter painter(this);
    int w = width();
    int h = height();
    int pw = iconPix.width();
    int ph = iconPix.height();
    if (!isPressed) {
        painter.drawPixmap(w / 2 - pw / 2, h / 2 - ph / 2, iconPix);
    } else {
        painter.drawPixmap(w / 2 - (pw - 4) / 2, h / 2 - (ph - 4) / 2, pw - 4, ph - 4, iconPix);
    }
}

FolderListDialog::FolderListDialog(const QStringList& folderPaths, const QString& title,
                                   const QString& content, QWidget* parent)
    : MaskDialogBase(parent),
      title(title),
      content(content),
      originalPaths(folderPaths),
      folderPaths(folderPaths) {
    vBoxLayout = new QVBoxLayout(widget);
    titleLabel = new QLabel(title, widget);
    contentLabel = new QLabel(content, widget);
    scrollArea = new SingleDirectionScrollArea(widget);
    scrollWidget = new QWidget(scrollArea);
    completeButton = new QPushButton(tr("Done"), widget);
    addFolderCard = new AddFolderCard(scrollWidget);

    for (const auto& path : folderPaths) {
        folderCards.append(new FolderCard(path, scrollWidget));
    }

    initWidget();
}

void FolderListDialog::initWidget() {
    setQss();

    int w = qMax(qMax(titleLabel->width() + 48, contentLabel->width() + 48), 352);
    widget->setFixedWidth(w);

    scrollArea->resize(294, 72);
    scrollWidget->resize(292, 72);
    scrollArea->setFixedWidth(294);
    scrollWidget->setFixedWidth(292);
    scrollArea->setMaximumHeight(400);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(scrollWidget);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    if (auto* hBar = scrollArea->horizontalScrollBar()) {
        hBar->hide();
    }

    initLayout();

    connect(addFolderCard, &AddFolderCard::clicked, this, &FolderListDialog::showFileDialog);
    connect(completeButton, &QPushButton::clicked, this, &FolderListDialog::onButtonClicked);
    for (auto* card : folderCards) {
        connect(card, &FolderCard::clicked, this, &FolderListDialog::showDeleteFolderCardDialog);
    }
}

void FolderListDialog::initLayout() {
    vBoxLayout->setContentsMargins(24, 24, 24, 24);
    vBoxLayout->setSizeConstraint(QVBoxLayout::SetFixedSize);
    vBoxLayout->setAlignment(Qt::AlignTop);
    vBoxLayout->setSpacing(0);

    auto* layout_1 = new QVBoxLayout();
    layout_1->setContentsMargins(0, 0, 0, 0);
    layout_1->setSpacing(6);
    layout_1->addWidget(titleLabel, 0, Qt::AlignTop);
    layout_1->addWidget(contentLabel, 0, Qt::AlignTop);
    vBoxLayout->addLayout(layout_1, 0);
    vBoxLayout->addSpacing(12);

    auto* layout_2 = new QHBoxLayout();
    layout_2->setAlignment(Qt::AlignCenter);
    layout_2->setContentsMargins(4, 0, 4, 0);
    layout_2->addWidget(scrollArea, 0, Qt::AlignCenter);
    vBoxLayout->addLayout(layout_2, 1);
    vBoxLayout->addSpacing(24);

    scrollLayout = new QVBoxLayout(scrollWidget);
    scrollLayout->setAlignment(Qt::AlignTop);
    scrollLayout->setContentsMargins(0, 0, 0, 0);
    scrollLayout->setSpacing(8);
    scrollLayout->addWidget(addFolderCard, 0, Qt::AlignTop);
    for (auto* card : folderCards) {
        scrollLayout->addWidget(card, 0, Qt::AlignTop);
    }

    auto* layout_3 = new QHBoxLayout();
    layout_3->setContentsMargins(0, 0, 0, 0);
    layout_3->addStretch(1);
    layout_3->addWidget(completeButton);
    vBoxLayout->addLayout(layout_3, 0);

    adjustWidgetSize();
}

void FolderListDialog::showFileDialog() {
    QString path = QFileDialog::getExistingDirectory(this, tr("Choose folder"), "./");
    if (path.isEmpty() || folderPaths.contains(path)) {
        return;
    }

    auto* card = new FolderCard(path, scrollWidget);
    scrollLayout->addWidget(card, 0, Qt::AlignTop);
    connect(card, &FolderCard::clicked, this, &FolderListDialog::showDeleteFolderCardDialog);
    card->show();

    folderPaths.append(path);
    folderCards.append(card);

    adjustWidgetSize();
}

void FolderListDialog::showDeleteFolderCardDialog() {
    auto* senderCard = qobject_cast<FolderCard*>(sender());
    if (!senderCard) return;

    QString titleStr = tr("Are you sure you want to delete the folder?");
    QString contentStr = tr("If you delete the ") + QString("\"%1\"").arg(senderCard->folderName) +
                         tr(" folder and remove it from the list, the folder will no "
                            "longer appear in the list, but will not be deleted.");

    auto* dialog = new Dialog(titleStr, contentStr, window());
    connect(dialog, &Dialog::yesSignal, this,
            [this, senderCard]() { deleteFolderCard(senderCard); });
    dialog->exec();
}

void FolderListDialog::deleteFolderCard(FolderCard* folderCard) {
    scrollLayout->removeWidget(folderCard);
    int index = folderCards.indexOf(folderCard);
    if (index != -1) {
        folderCards.removeAt(index);
        folderPaths.removeAt(index);
    }
    folderCard->deleteLater();
    adjustWidgetSize();
}

void FolderListDialog::setQss() {
    titleLabel->setObjectName(QStringLiteral("titleLabel"));
    contentLabel->setObjectName(QStringLiteral("contentLabel"));
    completeButton->setObjectName(QStringLiteral("completeButton"));
    scrollWidget->setObjectName(QStringLiteral("scrollWidget"));

    qfw::setStyleSheet(this, FluentStyleSheet::FolderListDialog);

    titleLabel->adjustSize();
    contentLabel->adjustSize();
    completeButton->adjustSize();
}

void FolderListDialog::onButtonClicked() {
    QStringList sortedOriginal = originalPaths;
    QStringList sortedCurrent = folderPaths;
    sortedOriginal.sort();
    sortedCurrent.sort();

    if (sortedOriginal != sortedCurrent) {
        setEnabled(false);
        QApplication::processEvents();
        emit folderChanged(folderPaths);
    }
    close();
}

void FolderListDialog::adjustWidgetSize() {
    int N = folderCards.size();
    int h = 72 * (N + 1) + 8 * N;
    scrollArea->setFixedHeight(qMin(h, 400));
}

}  // namespace qfw
