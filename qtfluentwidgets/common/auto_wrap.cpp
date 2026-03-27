#include "common/auto_wrap.h"

#include <QCache>
#include <QChar>
#include <QHash>
#include <QPair>
#include <QStringList>

namespace qfw {

// Static member definitions
QCache<QString, int> TextWrap::charWidthCache_(128);
QCache<QString, int> TextWrap::textWidthCache_(32);
QCache<QString, CharType> TextWrap::charTypeCache_(128);
QCache<QPair<QString, int>, QStringList> TextWrap::splitTokenCache_(128);

const QHash<QString, int> TextWrap::EAST_ASIAN_WIDTH_TABLE = {{"F", 2}, {"H", 1}, {"W", 2},
                                                              {"A", 1}, {"N", 1}, {"Na", 1}};

// Helper function to determine if a character is East Asian wide (CJK, etc.)
static bool isEastAsianWide(ushort unicode) {
    // CJK Unified Ideographs: U+4E00-U+9FFF
    // CJK Unified Ideographs Extension A: U+3400-U+4DBF
    // CJK Unified Ideographs Extension B-H: U+20000-U+323AF (surrogate pairs)
    // CJK Compatibility Ideographs: U+F900-U+FAFF
    // CJK Symbols and Punctuation: U+3000-U+303F
    // Hiragana: U+3040-U+309F
    // Katakana: U+30A0-U+30FF
    // Bopomofo: U+3100-U+312F, U+31A0-U+31BF
    // Hangul Syllables: U+AC00-U+D7AF
    // Hangul Jamo: U+1100-U+11FF
    // Fullwidth ASCII variants: U+FF00-U+FFEF

    return (unicode >= 0x4E00 && unicode <= 0x9FFF) ||  // CJK Unified Ideographs
           (unicode >= 0x3400 && unicode <= 0x4DBF) ||  // CJK Extension A
           (unicode >= 0xF900 && unicode <= 0xFAFF) ||  // CJK Compatibility
           (unicode >= 0x3000 && unicode <= 0x303F) ||  // CJK Symbols/Punctuation
           (unicode >= 0x3040 && unicode <= 0x309F) ||  // Hiragana
           (unicode >= 0x30A0 && unicode <= 0x30FF) ||  // Katakana
           (unicode >= 0x3100 && unicode <= 0x312F) ||  // Bopomofo
           (unicode >= 0x31A0 && unicode <= 0x31BF) ||  // Bopomofo Extended
           (unicode >= 0xAC00 && unicode <= 0xD7AF) ||  // Hangul Syllables
           (unicode >= 0x1100 && unicode <= 0x11FF) ||  // Hangul Jamo
           (unicode >= 0xFF00 && unicode <= 0xFFEF);    // Fullwidth variants
}

int TextWrap::getCharWidth(const QChar& ch) {
    QString charStr = ch;

    // Check cache first
    if (charWidthCache_.contains(charStr)) {
        return *charWidthCache_.object(charStr);
    }

    // Determine character width based on Unicode category
    // East Asian wide characters (CJK, etc.) have width 2, others have width 1
    int width = isEastAsianWide(ch.unicode()) ? 2 : 1;

    // Cache result
    charWidthCache_.insert(charStr, new int(width));
    return width;
}

int TextWrap::getTextWidth(const QString& text) {
    // Check cache first
    if (textWidthCache_.contains(text)) {
        return *textWidthCache_.object(text);
    }

    int totalWidth = 0;
    for (const QChar& ch : text) {
        totalWidth += getCharWidth(ch);
    }

    // Cache result
    textWidthCache_.insert(text, new int(totalWidth));
    return totalWidth;
}

CharType TextWrap::getCharType(const QChar& ch) {
    QString charStr = ch;

    // Check cache first
    if (charTypeCache_.contains(charStr)) {
        return *charTypeCache_.object(charStr);
    }

    CharType type;

    if (ch.isSpace()) {
        type = CharType::Space;
    } else if (getCharWidth(ch) == 1) {
        type = CharType::Latin;
    } else {
        type = CharType::Asian;
    }

    // Cache result
    charTypeCache_.insert(charStr, new CharType(type));
    return type;
}

QString TextWrap::processTextWhitespace(const QString& text) {
    // Replace multiple whitespace with single space and trim
    QString result = text.simplified();
    return result.trimmed();
}

QStringList TextWrap::splitLongToken(const QString& token, int width) {
    QStringList chunks;
    for (int i = 0; i < token.length(); i += width) {
        chunks.append(token.mid(i, width));
    }
    return chunks;
}

QStringList TextWrap::tokenizer(const QString& text) {
    QStringList tokens;
    QString buffer;
    CharType lastCharType = CharType::Space;

    for (const QChar& ch : text) {
        CharType charType = getCharType(ch);

        if (!buffer.isEmpty() && (charType != lastCharType || charType != CharType::Latin)) {
            tokens.append(buffer);
            buffer.clear();
        }

        buffer += ch;
        lastCharType = charType;
    }

    if (!buffer.isEmpty()) {
        tokens.append(buffer);
    }

    return tokens;
}

QPair<QString, bool> TextWrap::wrap(const QString& text, int width, bool once) {
    width = qMax(1, width);
    QStringList lines = text.split('\n');
    bool isWrapped = false;
    QStringList wrappedLines;

    for (const QString& line : lines) {
        QString processedLine = processTextWhitespace(line);

        if (getTextWidth(processedLine) > width) {
            auto [wrappedLine, lineWrapped] = wrapLine(processedLine, width, once);
            wrappedLines.append(wrappedLine);

            if (once) {
                wrappedLines.append(text.mid(wrappedLine.length()).trimmed());
                return qMakePair(wrappedLines.join("\n"), true);
            }
        } else {
            wrappedLines.append(processedLine);
        }
    }

    return qMakePair(wrappedLines.join("\n"), isWrapped);
}

QPair<QString, bool> TextWrap::wrapLine(const QString& text, int width, bool once) {
    QString lineBuffer;
    QStringList wrappedLines;
    int currentWidth = 0;

    for (const QString& token : tokenizer(text)) {
        int tokenWidth = getTextWidth(token);

        if (token == " " && currentWidth == 0) {
            continue;
        }

        if (currentWidth + tokenWidth <= width) {
            lineBuffer += token;
            currentWidth += tokenWidth;

            if (currentWidth == width) {
                wrappedLines.append(lineBuffer.trimmed());
                lineBuffer.clear();
                currentWidth = 0;
            }
        } else {
            if (currentWidth != 0) {
                wrappedLines.append(lineBuffer.trimmed());
            }

            QStringList chunks = splitLongToken(token, width);

            for (int i = 0; i < chunks.size() - 1; ++i) {
                wrappedLines.append(chunks[i].trimmed());
            }

            lineBuffer = chunks.last();
            currentWidth = getTextWidth(chunks.last());
        }
    }

    if (currentWidth != 0) {
        wrappedLines.append(lineBuffer.trimmed());
    }

    if (once && !wrappedLines.isEmpty()) {
        QString firstLine = wrappedLines.first();
        QStringList restLines = wrappedLines.mid(1);
        return qMakePair(firstLine + "\n" + restLines.join(" "), true);
    }

    return qMakePair(wrappedLines.join("\n"), true);
}

}  // namespace qfw
