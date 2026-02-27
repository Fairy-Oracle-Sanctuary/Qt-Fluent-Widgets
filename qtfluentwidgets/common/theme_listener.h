#pragma once

#include <QThread>
#include <QTimer>

#include "common/config.h"

namespace qfw {

class SystemThemeListener : public QThread {
    Q_OBJECT

public:
    explicit SystemThemeListener(QObject* parent = nullptr);

signals:
    void systemThemeChanged(Theme theme);

protected:
    void run() override;

private slots:
    void onThemeChanged();

private:
    void checkSystemTheme();
    
    Theme currentSystemTheme_ = Theme::Light;
    QTimer* checkTimer_;
};

}  // namespace qfw
