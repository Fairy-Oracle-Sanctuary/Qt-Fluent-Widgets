#include "trie.h"

namespace qfw {

Trie::Trie() : isEnd_(false) {
    children_.resize(26, nullptr);
}

void Trie::insert(const QString& key, const QVariant& value) {
    QString lowerKey = key.toLower();

    Trie* node = this;
    for (const QChar& c : lowerKey) {
        int i = c.toLatin1() - 'a';
        if (i < 0 || i >= 26) {
            return;  // Skip non-alphabetic characters
        }

        if (!node->children_[i]) {
            node->children_[i] = new Trie();
        }

        node = node->children_[i];
    }

    node->isEnd_ = true;
    node->key_ = lowerKey;
    node->value_ = value;
}

QVariant Trie::get(const QString& key, const QVariant& defaultValue) const {
    Trie* node = const_cast<Trie*>(this)->searchPrefix(key);
    if (!node || !node->isEnd_) {
        return defaultValue;
    }

    return node->value_;
}

Trie* Trie::searchPrefix(const QString& prefix) {
    QString lowerPrefix = prefix.toLower();
    Trie* node = this;

    for (const QChar& c : lowerPrefix) {
        int i = c.toLatin1() - 'a';
        if (i < 0 || i >= 26 || !node->children_[i]) {
            return nullptr;
        }

        node = node->children_[i];
    }

    return node;
}

QList<QPair<QString, QVariant>> Trie::items(const QString& prefix) const {
    Trie* node = const_cast<Trie*>(this)->searchPrefix(prefix);
    if (!node) {
        return {};
    }

    QList<QPair<QString, QVariant>> result;
    QQueue<Trie*> queue;
    queue.enqueue(node);

    while (!queue.isEmpty()) {
        Trie* current = queue.dequeue();

        if (current->isEnd_) {
            result.append({current->key_, current->value_});
        }

        for (Trie* child : current->children_) {
            if (child) {
                queue.enqueue(child);
            }
        }
    }

    return result;
}

}  // namespace qfw
