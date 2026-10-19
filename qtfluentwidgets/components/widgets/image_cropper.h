#pragma once

#include <QDialog>
#include <QImage>
#include <QPainterPath>
#include <QPointer>
#include <QPushButton>
#include <QVBoxLayout>
#include <functional>

namespace qfw {
enum class CropShape { RECTANGLE, CIRCLE, CUSTOM };
class ImageCropCanvas;

// A parent-scoped overlay, or a frameless modal dialog when no parent is supplied.
// Crops are returned as source-pixel QImages; this component never saves files.
class ImageCropper : public QDialog {
    Q_OBJECT
public:
    using CropPathFactory = std::function<QPainterPath(const QRectF&)>;
    explicit ImageCropper(QWidget* parent = nullptr);
    explicit ImageCropper(const QImage& image, QWidget* parent = nullptr);
    explicit ImageCropper(const QPixmap& image, QWidget* parent = nullptr);
    explicit ImageCropper(const QString& path, QWidget* parent = nullptr);
    ~ImageCropper() override;
    void setImage(const QImage& image);
    void setImage(const QPixmap& image);
    void setImage(const QString& path);
    QImage sourceImage() const;
    CropShape cropShape() const;
    void setCropShape(CropShape shape);
    void setCropPathFactory(CropPathFactory factory);
    QRectF selectionRect() const;
    void setSelectionRect(const QRectF& rect);
    QRectF imageRect() const;
    QRectF displaySelection() const;
    QImage croppedImage() const;
    QWidget* canvasWidget() const;
    QWidget* windowMask = nullptr;
    QWidget* toolBar = nullptr;
    QPushButton *cancelButton, *rotateButton, *flipButton, *shapeButton, *yesButton;
    QVBoxLayout* editorLayout;
public slots:
    void rotate();
    void flip();
    void accept() override;
    void done(int code) override;
signals:
    void imageCropped(const QImage& image);
    void selectionChanged(const QRectF& rect);

protected:
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;
    bool focusNextPrevChild(bool next) override;

private:
    friend class ImageCropCanvas;
    QPainterPath cropPath(const QRectF& rect) const;
    void syncOwnerGeometry();
    ImageCropCanvas* canvas_;
    QImage image_;
    QRectF selection_;
    CropShape shape_ = CropShape::RECTANGLE;
    CropPathFactory pathFactory_;
    QPointer<QWidget> owner_, previousFocus_;
    QSpacerItem* toolbarGap_;
    bool filterInstalled_ = false;
    bool finishing_ = false;
};
}  // namespace qfw
