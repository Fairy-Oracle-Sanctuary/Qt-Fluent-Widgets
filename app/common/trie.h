#pragma once

#include <QQueue>
#include <QString>
#include <QVariant>
#include <vector>

namespace qfw {

// Trie (prefix tree) for string key-value storage
// Only supports lowercase a-z characters
class Trie {
public:
    Trie();

    // Insert a key-value pair (key is converted to lowercase)
    void insert(const QString& key, const QVariant& value);

    // Get value for exact key match, returns default if not found
    QVariant get(const QString& key, const QVariant& defaultValue = QVariant()) const;

    // Search for all items matching the given prefix
    QList<QPair<QString, QVariant>> items(const QString& prefix) const;

private:
    // Search for node matching prefix
    Trie* searchPrefix(const QString& prefix);

    QString key_;
    QVariant value_;
    std::vector<Trie*> children_;
    bool isEnd_;
};

}  // namespace qfw
