#pragma once

#include "components/widgets/info_bar.h"
#include "components/widgets/progress_ring.h"

namespace qfw {

// Persistent neutral task notification. Dismissing it does not cancel a worker.
class ProgressInfoBar : public InfoBar {
    Q_OBJECT
    Q_PROPERTY(int value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(bool indeterminate READ isIndeterminate WRITE setIndeterminate)
    Q_PROPERTY(QString remainingTime READ remainingTime WRITE setRemainingTime)

public:
    explicit ProgressInfoBar(const QString& title, const QString& content,
                             Qt::Orientation orient = Qt::Horizontal, bool isClosable = true,
                             int duration = -1,
                             InfoBarPosition position = InfoBarPosition::TopRight,
                             QWidget* parent = nullptr);

    void setTitle(const QString& title);
    void setContent(const QString& content);
    QString remainingTime() const;
    void setRemainingTime(const QString& text);
    int value() const;
    void setValue(int value);
    bool isIndeterminate() const;
    void setIndeterminate(bool enabled);
    void setCustomBarColor(const QColor& light, const QColor& dark);
    void addWidget(QWidget* widget, int stretch = 0);

    static ProgressInfoBar* create(const QString& title, const QString& content,
                                   Qt::Orientation orient = Qt::Horizontal, bool isClosable = true,
                                   int duration = -1,
                                   InfoBarPosition position = InfoBarPosition::TopRight,
                                   QWidget* parent = nullptr);
    static ProgressInfoBar* info(const QString& title, const QString& content,
                                 Qt::Orientation orient = Qt::Horizontal, bool isClosable = true,
                                 int duration = -1,
                                 InfoBarPosition position = InfoBarPosition::TopRight,
                                 QWidget* parent = nullptr);

    IndeterminateProgressRing* progressRing = nullptr;
    ProgressRing* determinateRing = nullptr;

signals:
    void valueChanged(int value);

protected:
    void adjustText() override;
    void closeEvent(QCloseEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void changeEvent(QEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    void refreshText();
    QString description_;
    QString remainingTime_;
    bool indeterminate_ = true;
    bool closing_ = false;
    bool settingValue_ = false;
};

}  // namespace qfw
