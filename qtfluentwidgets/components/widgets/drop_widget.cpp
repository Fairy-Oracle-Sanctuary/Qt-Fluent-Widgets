#include "components/widgets/drop_widget.h"

#include <QAbstractItemView>
#include <QDialogButtonBox>
#include <QDir>
#include <QDragEnterEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QHideEvent>
#include <QMimeData>
#include <QPainter>
#include <QPushButton>
#include <QSet>
#include <QVBoxLayout>
#include <functional>
#include <stdexcept>

#include "common/config.h"
#include "common/style_sheet.h"
#include "components/widgets/button.h"
#include "components/widgets/label.h"

namespace qfw {
namespace {
// QFileDialog's standard acceptance rejects folders in ExistingFiles mode.
// Keep its navigation and filters, but allow multi-folder / mixed selections.
class PathSelectionDialog : public QFileDialog {
public:
    PathSelectionDialog(QWidget* parent, const QString& title, const QString& directory,
                        std::function<QStringList(const QStringList&)> validate, bool custom)
        : QFileDialog(parent, title, directory), validate_(std::move(validate)), custom_(custom) {
        setOption(QFileDialog::DontUseNativeDialog);
        setAcceptMode(QFileDialog::AcceptOpen);
    }
    void enableMultipleSelection() {
        for (auto* view : findChildren<QAbstractItemView*>()) {
            if (view->objectName() != QStringLiteral("listView") &&
                view->objectName() != QStringLiteral("treeView")) {
                continue;
            }
            view->setSelectionMode(QAbstractItemView::ExtendedSelection);
            connect(view->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this] {
                if (auto* box = findChild<QDialogButtonBox*>()) {
                    if (auto* open = box->button(QDialogButtonBox::Open)) {
                        open->setEnabled(!validate_(candidatePaths()).isEmpty());
                    }
                }
            });
        }
    }
    QStringList acceptedPaths() const { return acceptedPaths_; }
    void accept() override {
        acceptedPaths_ = validate_(custom_ ? candidatePaths() : selectedFiles());
        if (acceptedPaths_.isEmpty()) {
            return;
        }
        if (custom_) {
            QDialog::accept();
        } else {
            QFileDialog::accept();
        }
    }

private:
    QStringList candidatePaths() const {
        for (auto* view : findChildren<QAbstractItemView*>()) {
            if ((view->objectName() != QStringLiteral("listView") &&
                 view->objectName() != QStringLiteral("treeView")) ||
                !view->isVisible()) {
                continue;
            }
            QStringList paths;
            for (const auto& index : view->selectionModel()->selectedRows(0)) {
                const auto path = index.data(QFileSystemModel::FilePathRole).toString();
                if (!path.isEmpty()) {
                    paths.append(path);
                }
            }
            if (!paths.isEmpty()) {
                return paths;
            }
        }
        return selectedFiles();
    }
    std::function<QStringList(const QStringList&)> validate_;
    bool custom_;
    QStringList acceptedPaths_;
};
}  // namespace

DropMultiFilesWidget::DropMultiFilesWidget(QWidget* parent)
    : DropMultiFilesWidget(QStringLiteral("."), true, parent) {}
DropMultiFilesWidget::DropMultiFilesWidget(const QString& directory, bool dash, QWidget* parent)
    : DropMultiFilesWidget(SelectionMode::Files, directory, dash, parent) {}
DropMultiFilesWidget::DropMultiFilesWidget(SelectionMode mode, const QString& directory, bool dash,
                                           QWidget* parent)
    : QWidget(parent), mode_(mode), defaultDir_(directory), dashLine_(dash) {
    fileFilter_ = tr("All files (*.*)");
    setAcceptDrops(true);
    setMinimumSize(256, 200);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    viewLayout = new QVBoxLayout(this);
    viewLayout->setContentsMargins(20, 20, 20, 20);
    viewLayout->setSpacing(8);
    viewLayout->setAlignment(Qt::AlignCenter);
    QString labelText;
    QString browseText;
    switch (mode_) {
        case SelectionMode::Files:
            labelText = tr("Drag & drop any files here");
            browseText = tr("Browse files");
            break;
        case SelectionMode::File:
            labelText = tr("Drag & drop any file here");
            browseText = tr("Browse file");
            break;
        case SelectionMode::Folder:
            labelText = tr("Drag & drop a folder here");
            browseText = tr("Browse folder");
            break;
        case SelectionMode::Folders:
            labelText = tr("Drag & drop folders here");
            browseText = tr("Browse folders");
            break;
        case SelectionMode::Any:
            labelText = tr("Drag & drop files or folders here");
            browseText = tr("Browse");
            break;
    }
    label = new BodyLabel(labelText, this);
    label->setTextFormat(Qt::PlainText);
    label->setAlignment(Qt::AlignCenter);
    label->setWordWrap(true);
    label->setAttribute(Qt::WA_TransparentForMouseEvents);
    orLabel = new BodyLabel(tr("or"), this);
    orLabel->setAlignment(Qt::AlignCenter);
    orLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    button = new HyperlinkButton(QString(), browseText, this);
    label->setPixelFontSize(15);
    orLabel->setPixelFontSize(15);
    viewLayout->addWidget(label);
    viewLayout->addWidget(orLabel);
    viewLayout->addWidget(button, 0, Qt::AlignHCenter);
    setAccessibleName(labelText);
    connect(button, &QAbstractButton::clicked, this, &DropMultiFilesWidget::browse);
    connect(&QConfig::instance(), &QConfig::themeChanged, this, [this] { update(); });
    connect(&QConfig::instance(), &QConfig::themeColorChanged, this, [this] { update(); });
}
void DropMultiFilesWidget::setLabelText(const QString& text) {
    label->setText(text);
    setAccessibleName(text);
}
void DropMultiFilesWidget::setDefaultDir(const QString& directory) { defaultDir_ = directory; }
QString DropMultiFilesWidget::defaultDir() const { return defaultDir_; }
void DropMultiFilesWidget::setBorderColor(const QColor& color) {
    borderColor_ = color;
    update();
}
QColor DropMultiFilesWidget::borderColor() const { return borderColor_; }
void DropMultiFilesWidget::enableDashLine(bool enabled) {
    dashLine_ = enabled;
    update();
}
bool DropMultiFilesWidget::isDashLine() const { return dashLine_; }
void DropMultiFilesWidget::setBorderWidth(int width) {
    if (width < 0 || width > 64) {
        throw std::invalid_argument("Border width must be between 0 and 64");
    }
    borderWidth_ = width;
    update();
}
int DropMultiFilesWidget::borderWidth() const { return borderWidth_; }
void DropMultiFilesWidget::setBorderRadius(int radius) {
    if (radius < 0) {
        throw std::invalid_argument("Border radius must be nonnegative");
    }
    borderRadius_ = radius;
    update();
}
int DropMultiFilesWidget::borderRadius() const { return borderRadius_; }
void DropMultiFilesWidget::setFileFilter(const QString& filter) {
    fileFilter_ = filter.isEmpty() ? tr("All files (*.*)") : filter;
}
QString DropMultiFilesWidget::fileFilter() const { return fileFilter_; }
void DropMultiFilesWidget::setFileExtensions(const QString& extensions, const QString& name) {
    QStringList patterns;
    for (const auto& pattern : extensions.split(';')) {
        if (!pattern.trimmed().isEmpty()) {
            patterns.append(pattern.trimmed());
        }
    }
    setFileFilter(patterns.isEmpty()
                      ? QString()
                      : (name.isEmpty() ? tr("Files") : name) + QStringLiteral(" (") +
                            patterns.join(' ') + QStringLiteral(")"));
}
QString DropMultiFilesWidget::fileExtensions() const { return fileFilter_; }
QSize DropMultiFilesWidget::sizeHint() const { return QSize(360, 200); }
QStringList DropMultiFilesWidget::validPaths(const QStringList& paths) const {
    QStringList result;
    QSet<QString> seen;
    for (const auto& path : paths) {
        if (path.isEmpty()) {
            continue;
        }
        const QFileInfo info(path);
        const bool isFile = info.isFile();
        const bool isDir = info.isDir();
        const bool files = mode_ == SelectionMode::Files || mode_ == SelectionMode::File;
        const bool folders = mode_ == SelectionMode::Folder || mode_ == SelectionMode::Folders;
        if ((!isFile && !isDir) || (files && !isFile) || (folders && !isDir)) {
            continue;
        }
        const auto normalized = QDir::cleanPath(info.absoluteFilePath());
#ifdef Q_OS_WIN
        const auto key = normalized.toCaseFolded();
#else
        const auto key = normalized;
#endif
        if (seen.contains(key)) {
            continue;
        }
        seen.insert(key);
        result.append(normalized);
        if (mode_ == SelectionMode::File || mode_ == SelectionMode::Folder) {
            break;
        }
    }
    return result;
}
QStringList DropMultiFilesWidget::pathsFromUrls(const QList<QUrl>& urls) const {
    QStringList paths;
    for (const auto& url : urls) {
        if (url.isLocalFile()) {
            paths.append(url.toLocalFile());
        }
    }
    return validPaths(paths);
}
QFileDialog* DropMultiFilesWidget::browseDialog() const { return dialog_.data(); }
void DropMultiFilesWidget::browse() {
    if (!isEnabled() || !isVisible()) {
        return;
    }
    if (dialog_) {
        dialog_->raise();
        dialog_->activateWindow();
        return;
    }
    const bool custom = mode_ == SelectionMode::Folders || mode_ == SelectionMode::Any;
    auto* dialog = new PathSelectionDialog(
        this, button->text(), defaultDir_,
        [this](const QStringList& paths) { return validPaths(paths); }, custom);
    dialog_ = dialog;
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowModality(Qt::WindowModal);
    if (mode_ == SelectionMode::Folder || mode_ == SelectionMode::Folders) {
        dialog->setFileMode(QFileDialog::Directory);
        dialog->setOption(QFileDialog::ShowDirsOnly);
    } else {
        dialog->setFileMode(mode_ == SelectionMode::File ? QFileDialog::ExistingFile
                                                         : QFileDialog::ExistingFiles);
        dialog->setNameFilter(fileFilter_);
    }
    if (custom) {
        dialog->enableMultipleSelection();
    }
    connect(dialog, &QDialog::finished, this, [this, dialog](int result) {
        const auto paths = dialog->acceptedPaths();
        dialog_ = nullptr;
        if (result == QDialog::Accepted && isEnabled() && isVisible() && !paths.isEmpty()) {
            emit selectionChange(paths);
        }
    });
    dialog->open();
}
void DropMultiFilesWidget::cancelBrowsing() {
    if (dialog_) {
        dialog_->reject();
    }
}
void DropMultiFilesWidget::setDragActive(bool active) {
    if (dragActive_ != active) {
        dragActive_ = active;
        update();
    }
}
void DropMultiFilesWidget::dragEnterEvent(QDragEnterEvent* event) {
    const bool valid = isEnabled() && (event->possibleActions() & Qt::CopyAction) &&
                       !pathsFromUrls(event->mimeData()->urls()).isEmpty();
    setDragActive(valid);
    if (valid) {
        event->setDropAction(Qt::CopyAction);
        event->accept();
    } else {
        event->ignore();
    }
}
void DropMultiFilesWidget::dragMoveEvent(QDragMoveEvent* event) {
    const bool valid = isEnabled() && (event->possibleActions() & Qt::CopyAction) &&
                       !pathsFromUrls(event->mimeData()->urls()).isEmpty();
    setDragActive(valid);
    if (valid) {
        event->setDropAction(Qt::CopyAction);
        event->accept();
    } else {
        event->ignore();
    }
}
void DropMultiFilesWidget::dragLeaveEvent(QDragLeaveEvent* event) {
    setDragActive(false);
    event->accept();
}
void DropMultiFilesWidget::dropEvent(QDropEvent* event) {
    setDragActive(false);
    const auto paths = pathsFromUrls(event->mimeData()->urls());
    if (!isEnabled() || !(event->possibleActions() & Qt::CopyAction) || paths.isEmpty()) {
        event->ignore();
        return;
    }
    event->setDropAction(Qt::CopyAction);
    event->accept();
    emit draggedChange(paths);
}
void DropMultiFilesWidget::hideEvent(QHideEvent* event) {
    setDragActive(false);
    cancelBrowsing();
    QWidget::hideEvent(event);
}
void DropMultiFilesWidget::changeEvent(QEvent* event) {
    if (event->type() == QEvent::EnabledChange && !isEnabled()) {
        setDragActive(false);
        cancelBrowsing();
    }
    QWidget::changeEvent(event);
}
void DropMultiFilesWidget::paintEvent(QPaintEvent*) {
    if (borderWidth_ == 0) {
        return;
    }
    const int channel = isDarkTheme() ? 255 : 0;
    const QColor color = !isEnabled()  ? QColor(channel, channel, channel, 32)
                         : dragActive_ ? themeColor()
                         : borderColor_.isValid()
                             ? borderColor_
                             : QColor(channel, channel, channel, isDarkTheme() ? 120 : 100);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QPen pen(color, borderWidth_);
    if (dashLine_) {
        pen.setStyle(Qt::DashLine);
        pen.setDashPattern({4, 4});
    }
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    const qreal inset = qMax(2.0, borderWidth_ / 2.0 + 0.5);
    const QRectF bounds = QRectF(rect()).adjusted(inset, inset, -inset, -inset);
    const qreal radius = qMin(qreal(borderRadius_), qMin(bounds.width(), bounds.height()) / 2);
    painter.drawRoundedRect(bounds, radius, radius);
}

DropSingleFileWidget::DropSingleFileWidget(QWidget* parent)
    : DropSingleFileWidget(QStringLiteral("."), QString(), true, parent) {}
DropSingleFileWidget::DropSingleFileWidget(const QString& directory, const QString& filter,
                                           bool dash, QWidget* parent)
    : DropMultiFilesWidget(SelectionMode::File, directory, dash, parent) {
    setFileFilter(filter);
}
DropSingleFolderWidget::DropSingleFolderWidget(QWidget* parent)
    : DropSingleFolderWidget(QStringLiteral("."), true, parent) {}
DropSingleFolderWidget::DropSingleFolderWidget(const QString& directory, bool dash, QWidget* parent)
    : DropMultiFilesWidget(SelectionMode::Folder, directory, dash, parent) {}
DropMultiFoldersWidget::DropMultiFoldersWidget(QWidget* parent)
    : DropMultiFoldersWidget(QStringLiteral("."), true, parent) {}
DropMultiFoldersWidget::DropMultiFoldersWidget(const QString& directory, bool dash, QWidget* parent)
    : DropMultiFilesWidget(SelectionMode::Folders, directory, dash, parent) {}
DropAnyWidget::DropAnyWidget(QWidget* parent)
    : DropAnyWidget(QStringLiteral("."), QString(), true, parent) {}
DropAnyWidget::DropAnyWidget(const QString& directory, const QString& filter, bool dash,
                             QWidget* parent)
    : DropMultiFilesWidget(SelectionMode::Any, directory, dash, parent) {
    setFileFilter(filter);
}
}  // namespace qfw
