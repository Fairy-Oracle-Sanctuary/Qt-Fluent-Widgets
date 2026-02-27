#pragma once

#include <QTranslator>
#include <QLocale>

namespace qfw {

class FluentTranslator : public QTranslator {
public:
    explicit FluentTranslator(const QLocale& locale = QLocale(), QObject* parent = nullptr);
    
    void load(const QLocale& locale = QLocale());
};

}  // namespace qfw
