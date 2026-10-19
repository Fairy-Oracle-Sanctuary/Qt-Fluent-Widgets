#pragma once

#include <QEnterEvent>
#include <QPointer>

#include "common/qtcompat.h"
#include "components/widgets/image_cropper.h"
#include "components/widgets/label.h"

class QFileDialog;
namespace qfw {
class AvatarPicker : public AvatarWidget {
    Q_OBJECT
public:
    explicit AvatarPicker(QWidget* parent = nullptr);
    explicit AvatarPicker(const QImage& image, QWidget* parent = nullptr);
    explicit AvatarPicker(const QPixmap& image, QWidget* parent = nullptr);
    explicit AvatarPicker(const QString& path, QWidget* parent = nullptr);
    ~AvatarPicker() override;
    ImageCropper* cropImage(const QImage& image);
    ImageCropper* cropImage(const QPixmap& image);
    ImageCropper* cropImage(const QString& path);
    ImageCropper* cropEditor() const;
    QFileDialog* browseDialog() const;
public slots:
    void chooseImage();
    void cancelSelection();
signals:
    void imageChanged(const QImage& image);

protected:
    void enterEvent(enterEvent_QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void changeEvent(QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    bool hovered_ = false, pressed_ = false;
    QPointer<QFileDialog> dialog_;
    QPointer<ImageCropper> editor_;
};
}  // namespace qfw
