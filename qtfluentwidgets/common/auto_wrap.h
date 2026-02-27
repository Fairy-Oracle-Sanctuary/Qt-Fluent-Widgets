#pragma once

#include <QString>
#include <QStringList>
#include <QCache>
#include <QRegularExpression>

namespace qfw {

enum class CharType {
    Space = 0,
    Asian = 1,
    Latin = 2
};

class TextWrap {
public:
    // Text wrap utility class
    static int getCharWidth(const QChar& ch);
    static int getTextWidth(const QString& text);
    static CharType getCharType(const QChar& ch);
    static QString processTextWhitespace(const QString& text);
    static QStringList splitLongToken(const QString& token, int width);
    static QStringList tokenizer(const QString& text);
    
    // Main wrap function
    static QPair<QString, bool> wrap(const QString& text, int width, bool once = true);

private:
    static QPair<QString, bool> wrapLine(const QString& text, int width, bool once);
    
    // East Asian width table
    static const QHash<QString, int> EAST_ASIAN_WIDTH_TABLE;
    
    // Caches for performance
    static QCache<QString, int> charWidthCache_;
    static QCache<QString, int> textWidthCache_;
    static QCache<QString, CharType> charTypeCache_;
    static QCache<QPair<QString, int>, QStringList> splitTokenCache_;
};

}  // namespace qfw
