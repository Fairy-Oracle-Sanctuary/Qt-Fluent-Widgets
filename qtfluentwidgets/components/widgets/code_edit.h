#pragma once

#include "line_edit.h"
#include <QFutureWatcher>
#include <QSyntaxHighlighter>
#include <QTimer>

namespace qfw {

enum class CodeLanguage {
    PYTHON, C, CPP, CSHARP, JAVA, JAVASCRIPT, TYPESCRIPT, JSON, HTML, CSS,
    XML, YAML, TOML, INI, BASH, POWERSHELL, SQL, GO, RUST, MARKDOWN
};

struct CodeTokenSpan {
    int start = 0;
    int length = 0;
    QString token;
    bool operator==(const CodeTokenSpan& other) const {
        return start == other.start && length == other.length && token == other.token;
    }
};

struct CodeLexResult {
    QString error;
    QStringList lines;
    QVector<QVector<CodeTokenSpan>> spans;
};

// A document snapshot is lexed off the GUI thread. All positions use Qt's
// UTF-16 indexing; the worker never accesses a widget or a QTextDocument.
CodeLexResult lexCode(const QString& text, const QString& language);

class CodeHighlighter : public QSyntaxHighlighter {
public:
    explicit CodeHighlighter(QTextDocument* document);
    QTextCharFormat tokenFormat(const QString& token) const;
    CodeLexResult snapshot;
protected:
    void highlightBlock(const QString& text) override;
};

class CodeEdit : public PlainTextEdit {
    Q_OBJECT
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged)
    Q_PROPERTY(int indentSize READ indentSize WRITE setIndentSize)
    Q_PROPERTY(bool lineNumbersVisible READ lineNumbersVisible WRITE setLineNumbersVisible)
public:
    explicit CodeEdit(QWidget* parent = nullptr, CodeLanguage language = CodeLanguage::PYTHON);
    CodeEdit(QWidget* parent, const QString& language);
    static QStringList supportedLanguages();
    QString language() const { return language_; }
    void setLanguage(CodeLanguage language);
    void setLanguage(const QString& language);
    void setIndentSize(int size);
    int indentSize() const { return indentSize_; }
    void setLineNumbersVisible(bool visible);
    bool lineNumbersVisible() const { return numbersVisible_; }
    void setReadOnly(bool readonly);
    QWidget* lineNumberArea() const { return gutter_; }
    CodeHighlighter* highlighter() const { return highlighter_; }
signals:
    void languageChanged(const QString& language);
    void highlightingFinished();
    void highlightingFailed(const QString& error);
protected:
    void resizeEvent(QResizeEvent* event) override;
    void changeEvent(QEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
private:
    friend class CodeLineNumberArea;
    int gutterWidth() const;
    void updateGutter();
    void paintLineNumbers(QPaintEvent* event);
    void highlightCurrentLine();
    void startHighlighting();
    void applyHighlightBatch();
    void indentSelection(bool backwards);
    QWidget* gutter_ = nullptr;
    CodeHighlighter* highlighter_ = nullptr;
    QTimer lexTimer_, applyTimer_;
    QFutureWatcher<CodeLexResult> watcher_;
    QString language_;
    int indentSize_ = 4;
    bool numbersVisible_ = true;
    quint64 version_ = 0, jobVersion_ = 0, applyVersion_ = 0;
    bool jobActive_ = false;
    QVector<int> pendingRows_;
};

} // namespace qfw
