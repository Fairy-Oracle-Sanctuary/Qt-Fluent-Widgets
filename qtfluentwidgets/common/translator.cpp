#include "common/translator.h"

namespace qfw {

FluentTranslator::FluentTranslator(const QLocale& locale, QObject* parent) : QTranslator(parent) {
    load(locale);
}

void FluentTranslator::load(const QLocale& locale) {
    // Load translation file for the specified locale
    QString translationFile =
        QStringLiteral(":/qfluentwidgets/i18n/qfluentwidgets.%1.qm").arg(locale.name());

    if (!QTranslator::load(translationFile)) {
        // Fallback to English if translation not found
        QString fallbackFile = QStringLiteral(":/qfluentwidgets/i18n/qfluentwidgets.en.qm");
        QTranslator::load(fallbackFile);
    }
}

}  // namespace qfw
