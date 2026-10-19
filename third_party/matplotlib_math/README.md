# MathText reference assets

Generated from Matplotlib 3.9.4 installed with the Python Pro reference.
`scripts/export_chat_math.py` exports original glyph metrics at 12pt / 72dpi,
symbol classification, spacing rules and sized delimiter alternatives.
`--kerning` exports the actual ASCII character-pair adjustments for ten styles.
`--unicode` exports supported raw Unicode glyphs and exact reference pair
adjustments for ten styles, with shared metric records. The exporter uses
fontTools only during development to enumerate possible FreeType kern pairs.
It preserves Matplotlib's actual get_kerning arguments, rather than replacing
them with a text shaping engine. Missing glyph substitutions are excluded.
The bundled original DejaVu, STIX and Computer Modern fonts are used through
Qt's QRawFont, with no Python, Matplotlib or external TeX process at runtime.
These are reusable glyph data, not pre-rendered formula images.

The C++ parser/layout in `qtfluentwidgets/common/chat_math.cpp` follows
Matplotlib `_mathtext.py` (fraction, radical, script and accent box rules).
See LICENSE, LICENSE_DEJAVU, LICENSE_STIX and LICENSE_BAKOMA for the original
notices, including the Computer Modern font permission.

Qt vector antialiasing can differ from Matplotlib's Agg renderer. A finite
reference corpus does not establish arbitrary-input parser equivalence.
Unsupported commands or glyphs raise an error and retain the formula source.
The atlas also covers the bundled reference fonts' Unicode character maps
through U+1FFFF, including supplementary-plane mathematical alphabets.
Fonts without a glyph (for example CJK) retain source instead of a dummy symbol.
Layout retains horizontal/vertical/centered nodes and each node's shrink level,
including the six-level minimum-size rule. Figure extents use the 72dpi vector
pass; output positions use a separate 192dpi pass with reference Agg vertical
hint metrics. `--units` exports sized glyph design units; `--raster-hints PATH`
exports 103 reusable glyphs at integer pixel sizes 1..4096 as little-endian
float32 height/depth pairs, plus their JSON offsets. These are glyph metrics,
not pre-rendered formula images. Qt still draws the original font outlines.
`--font-classes` exports 406 shared metric records for 8956 differences in
16 font/class combinations, with 8800 exact pair adjustments. `--named-kerning`
exports 2270 additional ASCII named-symbol pairs. The independent font class
is preserved through nested virtual font groups.

The finite corpus covers 170 actual Pro calls: 156 successful reference images
and 14 errors, including fractions/scripts through 16 levels, deep radicals,
accents and substack, nested font groups and sizes 6, 9, 12, 16, 24 and 48pt.
Two Python RecursionError cases render successfully under the native 64-level
bound; the remaining invalid cases retain source. None of these finite checks
establish arbitrary-input MathText or full TeX equivalence. Raster extents are
bounded to 8192 physical pixels per side and 16 megapixels before allocation.
