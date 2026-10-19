#pragma once

#include <QHBoxLayout>
#include <QIcon>
#include <QList>
#include <QPushButton>
#include <QStringList>
#include <memory>

#include "common/icon.h"
#include "components/widgets/progress_ring.h"

namespace qfw {
class BodyLabel;

class FilledProgressBar : public ProgressBar {
    Q_OBJECT
public:
    explicit FilledProgressBar(QWidget* parent = nullptr, bool useAni = true);
    explicit FilledProgressBar(const QIcon& icon, QWidget* parent = nullptr, bool useAni = true);
    explicit FilledProgressBar(const FluentIconBase& icon, QWidget* parent = nullptr,
                               bool useAni = true);
    void setIcon(const QIcon& icon);
    void setIcon(const FluentIconBase& icon);
    QIcon icon() const;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QIcon icon_;
    std::unique_ptr<FluentIconBase> fluentIcon_;
};

class RadialGauge : public ProgressRing {
    Q_OBJECT
public:
    explicit RadialGauge(QWidget* parent = nullptr, bool useAni = true);

protected:
    void paintEvent(QPaintEvent* event) override;
};

class MultiSegmentProgressRing : public ProgressRing {
    Q_OBJECT
    Q_PROPERTY(qreal gapDegree READ gapDegree WRITE setGapDegree)
    Q_PROPERTY(QString textProperty READ text WRITE setText)
public:
    using Segment = QPair<qreal, QColor>;
    explicit MultiSegmentProgressRing(QWidget* parent = nullptr, bool useAni = true);
    QList<Segment> segments() const;
    void setSegments(const QList<Segment>& segments);
    qreal gapDegree() const;
    void setGapDegree(qreal degrees);
    QString text() const;
    void setText(const QString& text);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QList<Segment> segments_;
    qreal gapDegree_ = 3;
    QString text_;
};

class StepProgressBarButton : public QPushButton {
    Q_OBJECT
    Q_PROPERTY(qreal hoverProgress READ hoverProgress WRITE setHoverProgress)
public:
    explicit StepProgressBarButton(const QIcon& icon, const QString& text,
                                   QWidget* parent = nullptr);
    explicit StepProgressBarButton(const FluentIconBase& icon, const QString& text,
                                   QWidget* parent = nullptr);
    void setCheckedLockEnabled(bool enabled);
    bool isCheckedLockEnabled() const;
    void setNonInteractive(bool enabled);
    bool isNonInteractive() const;
    qreal hoverProgress() const;
    void setHoverProgress(qreal progress);
signals:
    void checkedClicked();

protected:
    bool event(QEvent* event) override;
    void nextCheckState() override;
    void paintEvent(QPaintEvent* event) override;

private:
    bool locked_ = false;
    bool nonInteractive_ = false;
    qreal hoverProgress_ = 0;
    QPropertyAnimation* hoverAnimation_;
};

class StepProgressBar : public QWidget {
    Q_OBJECT
    Q_PROPERTY(int current READ current WRITE setCurrent NOTIFY currentChanged)
    Q_PROPERTY(int count READ count WRITE setCount)
public:
    explicit StepProgressBar(QWidget* parent = nullptr);
    explicit StepProgressBar(int count, QWidget* parent = nullptr);
    explicit StepProgressBar(const FluentIconBase& icon, int count = 4, QWidget* parent = nullptr);
    int current() const;
    void setCurrent(int index);
    int count() const;
    void setCount(int count);
    void setIcon(int index, const QIcon& icon);
    void setIcon(int index, const FluentIconBase& icon);
    void setIcons(const QList<QIcon>& icons);
    QIcon icon(int index) const;
    void setStepName(int index, const QString& name);
    void setStepNames(const QStringList& names);
    QString stepName(int index) const;
    StepProgressBarButton* button(int index) const;
    void setNonInteractive(bool enabled);
    bool isNonInteractive() const;
    void nextStep();
    void previousStep();
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    QHBoxLayout* hBoxLayout;
signals:
    void currentChanged(int index);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void rebuild();
    void updateStates();
    int count_ = 4;
    int current_ = 0;
    bool nonInteractive_ = false;
    QList<QIcon> icons_;
    QStringList names_;
    QList<StepProgressBarButton*> buttons_;
    QList<BodyLabel*> labels_;
};
}  // namespace qfw
