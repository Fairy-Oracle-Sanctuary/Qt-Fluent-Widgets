#pragma once

#include <QObject>
#include <QString>

namespace qfw {

// Gallery translator - holds translated UI strings
class Translator : public QObject {
    Q_OBJECT

public:
    explicit Translator(QObject* parent = nullptr);

    QString text() const { return tr("Text"); }
    QString view() const { return tr("View"); }
    QString menus() const { return tr("Menus & toolbars"); }
    QString icons() const { return tr("Icons"); }
    QString layout() const { return tr("Layout"); }
    QString dialogs() const { return tr("Dialogs & flyouts"); }
    QString scroll() const { return tr("Scrolling"); }
    QString material() const { return tr("Material"); }
    QString dateTime() const { return tr("Date & time"); }
    QString navigation() const { return tr("Navigation"); }
    QString basicInput() const { return tr("Basic input"); }
    QString statusInfo() const { return tr("Status & info"); }
    QString price() const { return tr("Price"); }
};

}  // namespace qfw
