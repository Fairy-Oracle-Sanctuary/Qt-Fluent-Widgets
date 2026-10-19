#include "view/pro_picker_examples.h"

#include "qtfluentwidgets.h"
#include "view/gallery_interface.h"

namespace qfw {
namespace {
QString sourcePath() {
    return QStringLiteral(
        "https://github.com/Fairy-Oracle-Sanctuary/Qt-Fluent-Widgets/blob/main/app/view/"
        "pro_picker_examples.cpp");
}
QWidget* column() {
    auto* host = new QWidget;
    host->setMaximumWidth(560);
    auto* layout = new QVBoxLayout(host);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);
    return host;
}
}  // namespace
void ProPickerExamples::addBasicInput(GalleryInterface* gallery) {
    auto* host = column();
    auto* layout = static_cast<QVBoxLayout*>(host->layout());
    auto* circles = new CircleColorPicker(host);
    circles->setCurrentIndex(3);
    auto* status = new BodyLabel(tr("Selected color: %1").arg(circles->color().name()), host);
    auto* clear = new PushButton(tr("Clear selection"), host);
    clear->setFixedWidth(140);
    connect(circles, &CircleColorPicker::colorChanged, status, [status](const QColor& color) {
        status->setText(color.isValid() ? tr("Selected color: %1").arg(color.name(QColor::HexArgb))
                                        : tr("No color selected"));
    });
    connect(clear, &QAbstractButton::clicked, circles, [circles] { circles->setCurrentIndex(-1); });
    layout->addWidget(circles, 0, Qt::AlignLeft);
    layout->addWidget(status);
    layout->addWidget(clear, 0, Qt::AlignLeft);
    gallery->addExampleCard(tr("Circular color palette"), host, sourcePath(), 1);

    auto* paletteHost = column();
    auto* paletteLayout = static_cast<QVBoxLayout*>(paletteHost->layout());
    auto* palette = new DropDownColorPalette(QColor("#0078D4"), paletteHost);
    auto* selected =
        new BodyLabel(tr("Selected color: %1").arg(palette->color().name()), paletteHost);
    auto* alpha = new CheckBox(tr("Enable alpha in the color dialog"), paletteHost);
    connect(alpha, &QCheckBox::toggled, palette, &DropDownColorPalette::setAlphaEnabled);
    connect(palette, &DropDownColorPalette::colorSelected, selected,
            [selected](const QColor& color) {
                selected->setText(tr("Selected color: %1").arg(color.name(QColor::HexArgb)));
            });
    auto* hint = new BodyLabel(tr("Choose a swatch, an automatic color, or open the color dialog. "
                                  "This does not change the theme accent."),
                               paletteHost);
    hint->setWordWrap(true);
    paletteLayout->addWidget(palette, 0, Qt::AlignLeft);
    paletteLayout->addWidget(selected);
    paletteLayout->addWidget(alpha);
    paletteLayout->addWidget(hint);
    gallery->addExampleCard(tr("Drop-down color palette"), paletteHost, sourcePath(), 1);

    auto* pickerHost = column();
    auto* pickerLayout = static_cast<QVBoxLayout*>(pickerHost->layout());
    auto* picker = new DropDownColorPicker(QColor("#990078D4"), pickerHost);
    auto* pickerValue = new BodyLabel(tr("Selected color: %1").arg(picker->color().name(QColor::HexArgb)), pickerHost);
    auto* pickerAlpha = new CheckBox(tr("Enable transparency"), pickerHost);
    pickerAlpha->setChecked(true);
    connect(pickerAlpha, &QCheckBox::toggled, picker, &DropDownColorPicker::setAlphaEnabled);
    connect(picker, &DropDownColorPicker::colorChanged, pickerValue, [pickerValue](const QColor& color) {
        pickerValue->setText(tr("Selected color: %1").arg(color.name(QColor::HexArgb)));
    });
    auto* pickerHint = new BodyLabel(tr("Edit RGB, HSV or hexadecimal values. Confirm saves the draft; Cancel or Esc preserves the selected color."), pickerHost);
    pickerHint->setWordWrap(true);
    pickerLayout->addWidget(picker, 0, Qt::AlignLeft);
    pickerLayout->addWidget(pickerValue);
    pickerLayout->addWidget(pickerAlpha);
    pickerLayout->addWidget(pickerHint);
    gallery->addExampleCard(tr("Drop-down color picker"), pickerHost, sourcePath(), 1);

    auto* screenHost = column();
    auto* screenLayout = static_cast<QVBoxLayout*>(screenHost->layout());
    auto* screenPicker = new ScreenColorPicker(screenHost);
    auto* screenValue = new BodyLabel(tr("Selected color: %1").arg(screenPicker->color().name()), screenHost);
    auto* freeze = new CheckBox(tr("Freeze the screen while picking"), screenHost);
    freeze->setChecked(true);
    connect(freeze, &QCheckBox::toggled, screenPicker, &ScreenColorPicker::setFreezeScreenEnabled);
    connect(screenPicker, &ScreenColorPicker::colorPicked, screenValue, [screenValue](const QColor& color) {
        screenValue->setText(tr("Selected color: %1").arg(color.name()));
    });
    connect(screenPicker, &ScreenColorPicker::errorOccurred, screenValue, [screenValue](const QString& error) {
        screenValue->setText(tr("Screen capture failed: %1").arg(error));
    });
    auto* screenHint = new BodyLabel(tr("Click to pick a color from any monitor. Left-click or Enter confirms; right-click or Esc cancels. Live mode requires screen capture exclusion support."), screenHost);
    screenHint->setWordWrap(true);
    screenLayout->addWidget(screenPicker, 0, Qt::AlignLeft);
    screenLayout->addWidget(screenValue);
    screenLayout->addWidget(freeze);
    screenLayout->addWidget(screenHint);
    gallery->addExampleCard(tr("Screen color picker"), screenHost, sourcePath(), 1);

    auto* shortcutHost = column();
    auto* shortcutLayout = static_cast<QVBoxLayout*>(shortcutHost->layout());
    auto* shortcut = new ShortcutPicker(QStringLiteral("Ctrl+Shift+A"), shortcutHost);
    auto* sequence = new BodyLabel(
        tr("Shortcut: %1").arg(shortcut->keySequence().toString(QKeySequence::NativeText)),
        shortcutHost);
    auto* explanation =
        new BodyLabel(tr("Click the card and press a key combination. Save commits it, Reset "
                         "restores the default draft, and Cancel discards changes."),
                      shortcutHost);
    explanation->setWordWrap(true);
    connect(
        shortcut, &ShortcutPicker::keySequenceChanged, sequence,
        [sequence](const QKeySequence& keys) {
            sequence->setText(
                tr("Shortcut: %1")
                    .arg(keys.isEmpty() ? tr("Not set") : keys.toString(QKeySequence::NativeText)));
        });
    shortcutLayout->addWidget(shortcut, 0, Qt::AlignLeft);
    shortcutLayout->addWidget(sequence);
    shortcutLayout->addWidget(explanation);
    gallery->addExampleCard(tr("Shortcut picker"), shortcutHost, sourcePath(), 1);

    auto* flatHost = column();
    auto* flatLayout = static_cast<QVBoxLayout*>(flatHost->layout());
    auto* flat = new MultiSelectionComboBox(flatHost);
    flat->setFixedWidth(320);
    flat->setPlaceholderText(tr("Choose languages"));
    flat->addItems({"Python", "C++", "Rust", "JavaScript", "TypeScript", "Go", "C#", "Kotlin"});
    flat->setSelectedIndices({0, 2});
    flat->setMaxVisibleItems(6);
    auto* flatSelection =
        new BodyLabel(tr("Selected: %1").arg(flat->selectedTexts().join(", ")), flatHost);
    flatSelection->setWordWrap(true);
    auto* flatChips = new CheckBox(tr("Display removable chips"), flatHost);
    auto* flatClear = new PushButton(tr("Clear selection"), flatHost);
    flatClear->setFixedWidth(140);
    connect(flatChips, &QCheckBox::toggled, flat, &MultiSelectionComboBox::setChipsMode);
    connect(flatClear, &QAbstractButton::clicked, flat, &MultiSelectionComboBox::clearSelection);
    connect(flat, &MultiSelectionComboBox::selectedTextChanged, flatSelection,
            [flatSelection](const QStringList& texts) {
                flatSelection->setText(tr("Selected: %1").arg(texts.join(", ")));
            });
    flatLayout->addWidget(flat, 0, Qt::AlignLeft);
    flatLayout->addWidget(flatSelection);
    flatLayout->addWidget(flatChips);
    flatLayout->addWidget(flatClear, 0, Qt::AlignLeft);
    gallery->addExampleCard(tr("Multiple selection combo box"), flatHost, sourcePath(), 1);

    auto* fontHost = column();
    auto* fontLayout = static_cast<QVBoxLayout*>(fontHost->layout());
    auto* font = new FontComboBox(fontHost);
    font->setFixedWidth(320);
    font->setCurrentFont(qfw::getFont());
    auto* preview = new BodyLabel(tr("The quick brown fox — 字体预览 0123456789"), fontHost);
    preview->setWordWrap(true);
    auto updatePreview = [preview](const QFont& selectedFont) {
        auto displayFont = selectedFont;
        displayFont.setPixelSize(20);
        preview->setFont(displayFont);
    };
    updatePreview(font->currentFont());
    auto* filterFonts = new CheckBox(tr("Show monospaced fonts only"), fontHost);
    connect(filterFonts, &QCheckBox::toggled, font, [font](bool enabled) {
        font->setFontFilters(enabled ? FontComboBox::MonospacedFonts : FontComboBox::AllFonts);
    });
    connect(font, &FontComboBox::currentFontChanged, preview, updatePreview);
    fontLayout->addWidget(font, 0, Qt::AlignLeft);
    fontLayout->addWidget(preview);
    fontLayout->addWidget(filterFonts);
    gallery->addExampleCard(tr("Font combo box"), fontHost, sourcePath(), 1);

    auto* treeHost = column();
    auto* treeLayout = static_cast<QVBoxLayout*>(treeHost->layout());
    auto* tree = new TreeComboBox(treeHost);
    tree->setFixedWidth(360);
    tree->setPlaceholderText(tr("Choose a component"));
    const auto input = tree->addItem(tr("Input"));
    const auto buttons = tree->addItem(tr("Buttons"), input);
    const auto save = tree->addItem(tr("Save"), buttons, QStringLiteral("save"));
    tree->addItem(tr("Cancel"), buttons, QStringLiteral("cancel"));
    const auto display = tree->addItem(tr("Display"));
    tree->addItem(tr("Cards"), display, QStringLiteral("cards"));
    tree->addItem(tr("Progress"), display, QStringLiteral("progress"));
    tree->setCurrentModelIndex(save);
    tree->setMaxVisibleItems(8);
    auto* path = new BodyLabel(tr("Selected path: %1").arg(tree->currentText()), treeHost);
    path->setWordWrap(true);
    connect(tree, &TreeComboBox::currentTextChanged, path,
            [path](const QString& text) { path->setText(tr("Selected path: %1").arg(text)); });
    treeLayout->addWidget(tree, 0, Qt::AlignLeft);
    treeLayout->addWidget(path);
    gallery->addExampleCard(tr("Hierarchical single selection"), treeHost, sourcePath(), 1);

    auto* multiHost = column();
    auto* multiLayout = static_cast<QVBoxLayout*>(multiHost->layout());
    auto* multi = new MultiSelectionTreeComboBox(multiHost);
    multi->setFixedWidth(420);
    multi->setPlaceholderText(tr("Choose components"));
    const auto basic = multi->addItem(tr("Input"));
    const auto color = multi->addItem(tr("Color picker"), basic, QStringLiteral("color"));
    multi->addItem(tr("Shortcut picker"), basic, QStringLiteral("shortcut"));
    const auto statusGroup = multi->addItem(tr("Display"));
    const auto progress = multi->addItem(tr("Progress"), statusGroup, QStringLiteral("progress"));
    multi->addItem(tr("Cards"), statusGroup, QStringLiteral("cards"));
    multi->setSelectedIndexes({color, progress});
    auto* selection =
        new BodyLabel(tr("Selected: %1").arg(multi->selectedTexts().join(", ")), multiHost);
    selection->setWordWrap(true);
    auto* chips = new CheckBox(tr("Display removable chips"), multiHost);
    chips->setChecked(true);
    connect(chips, &QCheckBox::toggled, multi, &MultiSelectionTreeComboBox::setChipsMode);
    connect(multi, &MultiSelectionTreeComboBox::selectedTextChanged, selection,
            [selection](const QStringList& texts) {
                selection->setText(tr("Selected: %1").arg(texts.join(", ")));
            });
    auto* treeHint =
        new BodyLabel(tr("Parent checkboxes select all enabled descendant leaves. "
                         "Use the arrow to expand a group; remove a chip to deselect it."),
                      multiHost);
    treeHint->setWordWrap(true);
    multiLayout->addWidget(multi, 0, Qt::AlignLeft);
    multiLayout->addWidget(selection);
    multiLayout->addWidget(chips);
    multiLayout->addWidget(treeHint);
    gallery->addExampleCard(tr("Hierarchical multiple selection"), multiHost, sourcePath(), 1);
}
}  // namespace qfw
