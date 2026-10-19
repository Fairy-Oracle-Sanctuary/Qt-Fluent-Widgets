#pragma once

#include <QKeySequence>
#include <QPointer>

#include "components/widgets/card_widget.h"

namespace qfw {
class KeyCaps;
class ShortcutEditor;
class TransparentToolButton;

class ShortcutPicker : public CardWidget {
    Q_OBJECT
    Q_PROPERTY(
        QKeySequence keySequence READ keySequence WRITE setKeySequence NOTIFY keySequenceChanged)
public:
    explicit ShortcutPicker(QWidget* parent = nullptr);
    explicit ShortcutPicker(const QKeySequence& sequence, QWidget* parent = nullptr);
    explicit ShortcutPicker(const QString& sequence, QWidget* parent = nullptr);
    ~ShortcutPicker() override;
    QKeySequence keySequence() const;
    void setKeySequence(const QKeySequence& sequence);
    void setKeySequence(const QString& sequence);
    QKeySequence defaultKeySequence() const;
    void setDefaultKeySequence(const QKeySequence& sequence);
    void setDefaultKeySequence(const QString& sequence);
    void reset();
    void clear();
    QString dialogTitle() const;
    void setDialogTitle(const QString& title);
    QString dialogDescription() const;
    void setDialogDescription(const QString& description);
    void setEscapeCancelsCapture(bool enabled);
    bool isEscapeCancelsCapture() const;
    bool isEditing() const;
    void showEditor();
    void cancelEditing();
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    QWidget* keyCaps;
    TransparentToolButton* editButton;
signals:
    void keySequenceChanged(const QKeySequence& sequence);
    void keySequenceSelected(const QKeySequence& sequence);
    void editingFinished();
    void editingCanceled();

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    void updateAccessibleName();
    QKeySequence sequence_;
    QKeySequence default_;
    QString title_;
    QString description_;
    bool customTitle_ = false;
    bool customDescription_ = false;
    bool escapeCancels_ = true;
    bool pressPending_ = false;
    QPointer<ShortcutEditor> editor_;
};
}  // namespace qfw
