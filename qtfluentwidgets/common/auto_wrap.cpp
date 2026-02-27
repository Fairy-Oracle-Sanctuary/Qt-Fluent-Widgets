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

int TextWrap::getCharWidth(const QChar& ch) {
    QString charStr = ch;

    // Check cache first
    if (charWidthCache_.contains(charStr)) {
        return *charWidthCache_.object(charStr);
    }

    // Get Unicode East Asian width
    int width = EAST_ASIAN_WIDTH_TABLE.value(QString::number(ch.unicode()), 1);

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
