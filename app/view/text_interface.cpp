#include "view/text_interface.h"

#include <QCompleter>
#include <QStringListModel>
#include <QVBoxLayout>
#include "components/widgets/check_box.h"
#include "components/widgets/code_edit.h"
#include "components/widgets/combo_box.h"
#include "components/widgets/button.h"
#include "components/widgets/label.h"

#include "common/translator.h"
#include "components/widgets/line_edit.h"
#include "components/widgets/spin_box.h"
#include "view/pro_container_examples.h"
#include "view/pro_input_examples.h"

namespace qfw {

TextInterface::TextInterface(QWidget* parent)
    : GalleryInterface(
          []() {
              Translator t;
              return t.text();
          }(),
          "qtfluentwidgets.components.widgets", parent) {
    setObjectName("textInterface");
    ProContainerExamples::addText(this);
    ProInputExamples::addText(this);

    auto* codeHost = new QWidget(this);
    auto* codeLayout = new QVBoxLayout(codeHost);
    codeLayout->setContentsMargins(0, 0, 0, 0);
    codeLayout->setSpacing(12);
    auto* codeNote = new BodyLabel(tr("20 languages; changing language keeps your code. Load sample replaces it."), codeHost);
    codeNote->setWordWrap(true);
    codeLayout->addWidget(codeNote);
    auto* codeControls = new QHBoxLayout;
    auto* codeLanguage = new ComboBox(codeHost);
    codeLanguage->addItems(CodeEdit::supportedLanguages());
    codeLanguage->setCurrentText("json");
    auto* loadCode = new PushButton(tr("Load sample"), codeHost);
    auto* advancedCode = new PushButton(tr("Load advanced sample"), codeHost);
    advancedCode->setEnabled(true);
    auto* codeReadOnly = new CheckBox(tr("Read only"), codeHost);
    auto* codeNumbers = new CheckBox(tr("Line numbers"), codeHost);
    codeNumbers->setChecked(true);
    for (QWidget* control : QList<QWidget*>{codeLanguage, loadCode, advancedCode, codeReadOnly, codeNumbers}) codeControls->addWidget(control);
    codeControls->addStretch();
    codeLayout->addLayout(codeControls);
    auto* codeEditor = new CodeEdit(codeHost, "json");
    codeEditor->setMinimumWidth(500);
    codeEditor->setFixedHeight(320);
    const QHash<QString, QString> codeSamples = {
        {"python", "# Unicode and multiline strings\nclass Greeter:\n    def hello(self, name):\n        message = \"\"\"Hello\n世界 🌍\n\"\"\"\n        return f\"{message}, {name}\"\n"},
        {"c", "/* multiline\n   comment */\n#include <stdio.h>\nint main(void) {\n    printf(\"Hello\\n\");\n    return 0;\n}\n"},
        {"cpp", "#include <iostream>\n/* multiline\n   comment */\nint main() {\n    std::cout << \"Hello\" << std::endl;\n    return 0;\n}\n"},
        {"csharp", "using System;\nclass Demo {\n    static void Main() { Console.WriteLine(\"Hello\"); }\n}\n"},
        {"java", "class Demo {\n    public static void main(String[] args) {\n        System.out.println(\"Hello\");\n    }\n}\n"},
        {"javascript", "// Template strings\nconst name = \"World\";\nconst message = `Hello\n${name}`;\nconsole.log(message);\n"},
        {"typescript", "interface User { name: string; active: boolean; }\nconst user: User = {name: \"Hello\", active: true};\n"},
        {"json", "{\n    \"configurations\": [\n        {\n            \"name\": \"Win32\",\n            \"includePath\": [\"${workspaceFolder}/**\"],\n            \"defines\": [\"_DEBUG\", \"UNICODE\"]\n        }\n    ],\n    \"version\": 4,\n    \"start\": true\n}\n"},
        {"html", "<!-- embedded languages -->\n<html>\n<style>body { color: #0078d4; }</style>\n<script>const value = \"Hello\";</script>\n<body>Hello</body>\n</html>\n"},
        {"css", "/* Theme */\n.card {\n    color: #0078d4;\n    border-radius: 8px;\n}\n"},
        {"xml", "<?xml version=\"1.0\"?>\n<settings enabled=\"true\">\n    <name>Hello</name>\n</settings>\n"},
        {"yaml", "# Configuration\nname: Hello\nmessage: |\n  multiline\n  text\nitems:\n  - one\n  - two\n"},
        {"toml", "# Configuration\n[window]\ntitle = \"Hello\"\nsize = [900, 700]\nenabled = true\n"},
        {"ini", "; Configuration\n[window]\ntitle = Hello\nwidth = 900\n"},
        {"bash", "#!/bin/bash\nname=\"Hello\"\nfor file in *.txt; do\n    echo \"$name: $file\"\ndone\n"},
        {"powershell", "# Configuration\n$name = \"Hello\"\nGet-ChildItem | ForEach-Object {\n    Write-Output \"$name: $_\"\n}\n"},
        {"sql", "-- Query\nSELECT name, COUNT(*) AS total\nFROM users\nWHERE active = TRUE\nGROUP BY name;\n"},
        {"go", "package main\nimport \"fmt\"\nfunc main() {\n    fmt.Println(\"Hello\");\n}\n"},
        {"rust", "// Hello\nfn main() {\n    let name = \"World\";\n    println!(\"Hello {}\", name);\n}\n"},
        {"markdown", "# Hello\n\n**Bold** and *italic*.\n\n```python\nprint(\"Hello\")\n```\n"}
    };
    codeEditor->setPlainText(codeSamples.value("json"));
    const QHash<QString, QString> advancedSamples = {
        {"json", "{\n    \"世界 🌍\" /* key comment */ : true,\n    \"values\": [null, false, -1.5e+2],\n    \"escaped\": \"Hello \\u0041\"\n}\n"},
        {"yaml", "%YAML 1.2\n---\nroot:\n  - name: 世界 🌍\n    message: |2-\n      first line\n      second line\n  - name: other\nflow: {ready: true, values: [one, two]}\n...\n"},
        {"markdown", "# Hello 世界 🌍\n\n- [x] **Task** with `code`\n[Link](https://example.com)\n\n```python extra metadata\nreturn True\n```\n\n```json\n{\"世界 🌍\": true}\n```\n"},
        {"c", "#include <stdint.h>\n#include <stddef.h>\n/* Standard and platform types */\nsize_t count = 4;\nuint32_t value = 0xff;\nint sum(int a, int b) { return a + b; }\n"},
        {"cpp", "#include <string>\ntemplate<typename T> class Box { public: T value; };\nstd::size_t count = 4;\nauto text = u8R\"tag(Hello 世界 🌍\nraw text)tag\";\nconstexpr bool ready = true;\n"},
        {"csharp", "[Obsolete(\"demo\")]\nnamespace App {\n    public class Demo<T> {\n        string text = $\"Hello {true}\";\n        string path = @\"C:\\temp\\file\";\n    }\n}\n"},
        {"java", "import java.util.List;\nclass Demo {\n    @Override\n    public String toString() {\n        System.out.println(\"Hello 世界 🌍\");\n        return \"demo\";\n    }\n}\n"},
        {"html", "<!DOCTYPE html>\n<style>\n.card:hover { color: var(--accent, red); }\n</style>\n<script>\nconst text = `Hello ${true}`;\n</script>\n<div class=\"card\">世界 🌍 &amp;</div>\n"},
        {"xml", "<?xml version=\"1.0\"?>\n<app:config xmlns:app=\"urn:app\" enabled=\"true\">\n    <![CDATA[<Hello>\n世界 🌍]]>\n</app:config>\n"},
        {"toml", "[[products]]\n\"quoted.key\" = { color = \"red\", width = 2.5 }\ntime = 1979-05-27T07:32:00Z\ntitle = \"\"\"Hello\n世界 🌍\"\"\"\ncount = 0xFF\n"},
        {"ini", "; Configuration\n[window]\ntitle = 世界 🌍\nvalue = first\n    second\nwidth: 900\nenabled=yes\n"},
        {"powershell", "function Get-Value {\n    param([int]$count)\n    if ($count -gt 0) { return $true }\n}\n$value = @\"\nHello $name 世界 🌍\n\"@\nGet-ChildItem | ForEach-Object { $_.Name }\n"},
        {"sql", "CREATE TABLE users (id INTEGER PRIMARY KEY, name VARCHAR(255));\nINSERT INTO users VALUES (1, 'Hello 世界 🌍');\nSELECT COUNT(*) AS total FROM users\nWHERE name IS NOT NULL;\n"},
        {"go", "package main\nimport \"fmt\"\ntype User struct { Name string; Active bool }\nfunc main() {\n    text := `Hello\n世界 🌍`\n    fmt.Println(text)\n}\n"},
        {"rust", "#[derive(Debug)]\nstruct User { name: String }\npub fn borrow<'a>(value: &'a str) -> &'a str { value }\n/* outer /* inner */ still */\nlet text = r##\"Hello 世界 🌍\nraw text\"##;\n"},
        {"python", "# Nested format fields and debug expressions\nwidth, precision = 12, 3\nvalue = 3.1415926\nprint(f\"{value!r:>{width}.{precision}}\")\nprint(f\"{value = :.2f} / {{literal}}\")\nmatch value:\n    case float() as number:\n        print(f\"{number:.{precision}f}\")\n"},
        {"javascript", "// Regular expressions, division and nested templates\nconst pattern = /[a-z]+\\/test/gi;\nconst ratio = 12 / 3;\nconst message = `outer ${`inner ${ratio}`} done`;\nconsole.log(pattern.test(message));\n"},
        {"typescript", "interface User<T> { name: string; value: T; }\ntype Result = User<number> | null;\nconst user: Result = { name: \"世界 🌍\", value: 42 };\nconst message = `Hello ${user?.name ?? \"Guest\"}`;\n"},
        {"css", "/* Selectors, at-rules and nested functions */\n@media screen and (min-width: 600px) {\n  .card:hover > #title::before {\n    --accent: #0078d4;\n    color: var(--accent, red);\n    width: calc(100% - 2rem);\n    background: linear-gradient(90deg, red, blue);\n  }\n}\n"},
        {"bash", "#!/bin/bash\nname=\"${USER:-guest}\"\necho \"${name^^}: ${#name}\"\necho \"$(printf '%s' \"${name:0:3}\")\"\ncount=$((2 + 3 * 4))\ncat <<EOF\nHello $name\nEOF\n"}
    };
    connect(codeLanguage, &ComboBox::currentTextChanged, advancedCode, [advancedCode, advancedSamples](const QString& language) { advancedCode->setEnabled(advancedSamples.contains(language)); });
    connect(advancedCode, &QPushButton::clicked, codeEditor, [codeEditor, codeLanguage, advancedSamples] { codeEditor->setPlainText(advancedSamples.value(codeLanguage->currentText())); });
    auto* advancedNote = new CaptionLabel(tr("Advanced samples for 20 languages: types, embedded syntax, strings, and expressions."), codeHost);
    advancedNote->setWordWrap(true);
    codeLayout->addWidget(advancedNote);
    connect(codeLanguage, &ComboBox::currentTextChanged, codeEditor, [codeEditor](const QString& language) { codeEditor->setLanguage(language); });
    connect(loadCode, &QPushButton::clicked, codeEditor, [codeEditor, codeLanguage, codeSamples] { codeEditor->setPlainText(codeSamples.value(codeLanguage->currentText())); });
    connect(codeReadOnly, &QCheckBox::toggled, codeEditor, &CodeEdit::setReadOnly);
    connect(codeNumbers, &QCheckBox::toggled, codeEditor, &CodeEdit::setLineNumbersVisible);
    codeLayout->addWidget(codeEditor);
    addExampleCard(tr("Code editor with asynchronous highlighting"), codeHost, "", 1);

    // Line edit with clear button
    auto* lineEdit = new LineEdit(this);
    lineEdit->setText(tr("ko no dio da!"));
    lineEdit->setClearButtonEnabled(true);
    addExampleCard(tr("A LineEdit with a clear button"), lineEdit,
                   "https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/examples/text/"
                   "line_edit/demo.py");

    // Search line edit with completer
    auto* searchLineEdit = new SearchLineEdit(this);
    searchLineEdit->setPlaceholderText(tr("Type a stand name"));
    searchLineEdit->setClearButtonEnabled(true);
    searchLineEdit->setFixedWidth(230);

    QStringList stands = {
        "Star Platinum",  "Hierophant Green", "Made in Haven",     "King Crimson",
        "Silver Chariot", "Crazy diamond",    "Metallica",         "Another One Bites The Dust",
        "Heaven's Door",  "Killer Queen",     "The Grateful Dead", "Stone Free",
        "The World",      "Sticky Fingers",   "Ozone Baby",        "Love Love Deluxe",
        "Hermit Purple",  "Gold Experience",  "King Nothing",      "Paper Moon King",
        "Scary Monster",  "Mandom",           "20th Century Boy",  "Tusk Act 4",
        "Ball Breaker",   "Sex Pistols",      "D4C • Love Train",  "Born This Way",
        "SOFT & WET",     "Paisley Park",     "Wonder of U",       "Walking Heart",
        "Cream Starter",  "November Rain",    "Smooth Operators",  "The Matte Kudasai"};
    auto* completer = new QCompleter(stands, searchLineEdit);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setMaxVisibleItems(10);
    searchLineEdit->setCompleter(completer);

    addExampleCard(tr("A autosuggest line edit"), searchLineEdit,
                   "https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/examples/text/"
                   "line_edit/demo.py");

    // Password line edit
    auto* passwordHost = new QWidget(this);
    auto* passwordLayout = new QVBoxLayout(passwordHost);
    passwordLayout->setContentsMargins(0,0,0,0);
    passwordLayout->setSpacing(12);
    auto* passwordLineEdit = new PasswordLineEdit(passwordHost);
    passwordLineEdit->setFixedWidth(230);
    passwordLineEdit->setPlaceholderText(tr("Enter your password"));
    passwordLineEdit->setText("FluentWidgets");
    auto* hidePasswordButton = new CheckBox(tr("Hide password view button"),passwordHost);
    connect(hidePasswordButton,&QCheckBox::toggled,passwordLineEdit,
        [passwordLineEdit](bool checked) { passwordLineEdit->setViewPasswordButtonVisible(!checked); });
    passwordLayout->addWidget(passwordLineEdit,0,Qt::AlignLeft);
    passwordLayout->addWidget(new CaptionLabel(tr("Hold the eye button to view; release to hide."),passwordHost));
    passwordLayout->addWidget(hidePasswordButton);
    addExampleCard(tr("A password line edit"), passwordHost,
                   "https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/master/examples/text/"
                   "line_edit/demo.py");

    // Spin box
    addExampleCard(tr("A SpinBox with a spin button"), new SpinBox(this),
                   "https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/examples/text/"
                   "spin_box/demo.py");

    // Double spin box
    addExampleCard(tr("A DoubleSpinBox with a spin button"), new DoubleSpinBox(this),
                   "https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/examples/text/"
                   "spin_box/demo.py");

    // Text edit
    auto* textEdit = new TextEdit(this);
    textEdit->setMarkdown(
        "## Steel Ball Run \n"
        " * Johnny Joestar 🦄 \n"
        " * Gyro Zeppeli 🐴 ");
    textEdit->setFixedHeight(150);
    auto* richHost = new QWidget(this);
    auto* richLayout = new QVBoxLayout(richHost);
    richLayout->setContentsMargins(0, 0, 0, 0);
    richLayout->setSpacing(12);
    auto* richNote = new BodyLabel(tr("Edit rich text, paste formatting, or open the Fluent context menu."), richHost);
    richNote->setWordWrap(true);
    auto* readOnly = new CheckBox(tr("Read only"), richHost);
    connect(readOnly, &QCheckBox::toggled, textEdit, &QTextEdit::setReadOnly);
    richLayout->addWidget(richNote);
    richLayout->addWidget(textEdit);
    richLayout->addWidget(readOnly);
    addExampleCard(tr("A simple TextEdit"), richHost,
                   "https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/examples/text/"
                   "line_edit/demo.py",
                   1);
}

}  // namespace qfw
