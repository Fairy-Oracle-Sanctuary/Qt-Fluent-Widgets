#include "view/text_interface.h"

#include <QCompleter>
#include <QStringListModel>

#include "common/translator.h"
#include "components/date_time/date_picker.h"
#include "components/date_time/time_picker.h"
#include "components/widgets/line_edit.h"
#include "components/widgets/spin_box.h"

namespace qfw {

TextInterface::TextInterface(QWidget* parent)
    : GalleryInterface(
          []() {
              Translator t;
              return t.text();
          }(),
          "qtfluentwidgets.components.widgets", parent) {
    setObjectName("textInterface");

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
    auto* passwordLineEdit = new PasswordLineEdit(this);
    passwordLineEdit->setFixedWidth(230);
    passwordLineEdit->setPlaceholderText(tr("Enter your password"));
    addExampleCard(tr("A password line edit"), passwordLineEdit,
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

    // Date picker (instead of DateEdit which doesn't exist in C++)
    addExampleCard(tr("A DatePicker"), new DatePicker(this),
                   "https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/examples/text/"
                   "spin_box/demo.py");

    // Time picker (instead of TimeEdit which doesn't exist in C++)
    addExampleCard(tr("A TimePicker"), new TimePicker(this),
                   "https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/examples/text/"
                   "spin_box/demo.py");

    // Text edit
    auto* textEdit = new TextBrowser(this);
    textEdit->setMarkdown(
        "## Steel Ball Run \n"
        " * Johnny Joestar 🦄 \n"
        " * Gyro Zeppeli 🐴 ");
    textEdit->setFixedHeight(150);
    addExampleCard(tr("A simple TextEdit"), textEdit,
                   "https://github.com/zhiyiYo/PyQt-Fluent-Widgets/blob/PySide6/examples/text/"
                   "line_edit/demo.py",
                   1);
}

}  // namespace qfw
