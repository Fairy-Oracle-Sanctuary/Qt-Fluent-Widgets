#pragma once

#include <QColor>
#include <QPointer>
#include <QStringList>
#include <QUrl>
#include <QWidget>

class QFileDialog;
class QVBoxLayout;

namespace qfw {
class BodyLabel;
class HyperlinkButton;

// These widgets select paths only. They never copy, move, upload or delete files.
class DropMultiFilesWidget : public QWidget {
    Q_OBJECT
    Q_PROPERTY(int borderWidth READ borderWidth WRITE setBorderWidth)
    Q_PROPERTY(int borderRadius READ borderRadius WRITE setBorderRadius)
    Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor)
    Q_PROPERTY(bool dashLine READ isDashLine WRITE enableDashLine)
public:
    explicit DropMultiFilesWidget(QWidget* parent = nullptr);
    explicit DropMultiFilesWidget(const QString& defaultDir, bool isDashLine = true,
                                  QWidget* parent = nullptr);
    void setLabelText(const QString& text);
    void setDefaultDir(const QString& directory);
    QString defaultDir() const;
    void setBorderColor(const QColor& color);
    QColor borderColor() const;
    void enableDashLine(bool enabled);
    bool isDashLine() const;
    void setBorderWidth(int width);
    int borderWidth() const;
    void setBorderRadius(int radius);
    int borderRadius() const;
    void setFileFilter(const QString& filter);
    QString fileFilter() const;
    // Like Python, this filter affects browsing, not drag-and-drop.
    void setFileExtensions(const QString& extensions, const QString& name = QString());
    QString fileExtensions() const;
    QStringList pathsFromUrls(const QList<QUrl>& urls) const;
    QFileDialog* browseDialog() const;
    QSize sizeHint() const override;

    QVBoxLayout* viewLayout;
    BodyLabel* label;
    BodyLabel* orLabel;
    HyperlinkButton* button;
public slots:
    void browse();
    void cancelBrowsing();
signals:
    void draggedChange(const QStringList& paths);
    void selectionChange(const QStringList& paths);

protected:
    enum class SelectionMode { Files, File, Folder, Folders, Any };
    DropMultiFilesWidget(SelectionMode mode, const QString& defaultDir, bool isDashLine,
                         QWidget* parent);
    void paintEvent(QPaintEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    QStringList validPaths(const QStringList& paths) const;
    void setDragActive(bool active);
    SelectionMode mode_;
    QString defaultDir_;
    QString fileFilter_;
    QColor borderColor_;
    int borderWidth_ = 2;
    int borderRadius_ = 16;
    bool dashLine_;
    bool dragActive_ = false;
    QPointer<QFileDialog> dialog_;
};

class DropSingleFileWidget : public DropMultiFilesWidget {
    Q_OBJECT
public:
    explicit DropSingleFileWidget(QWidget* parent = nullptr);
    explicit DropSingleFileWidget(const QString& defaultDir, const QString& fileFilter = QString(),
                                  bool isDashLine = true, QWidget* parent = nullptr);
};
class DropSingleFolderWidget : public DropMultiFilesWidget {
    Q_OBJECT
public:
    explicit DropSingleFolderWidget(QWidget* parent = nullptr);
    explicit DropSingleFolderWidget(const QString& defaultDir, bool isDashLine = true,
                                    QWidget* parent = nullptr);
};
class DropMultiFoldersWidget : public DropMultiFilesWidget {
    Q_OBJECT
public:
    explicit DropMultiFoldersWidget(QWidget* parent = nullptr);
    explicit DropMultiFoldersWidget(const QString& defaultDir, bool isDashLine = true,
                                    QWidget* parent = nullptr);
};
class DropAnyWidget : public DropMultiFilesWidget {
    Q_OBJECT
public:
    explicit DropAnyWidget(QWidget* parent = nullptr);
    explicit DropAnyWidget(const QString& defaultDir, const QString& fileFilter = QString(),
                           bool isDashLine = true, QWidget* parent = nullptr);
};
}  // namespace qfw
