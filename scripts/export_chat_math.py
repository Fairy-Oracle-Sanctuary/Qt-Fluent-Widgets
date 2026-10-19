"""Reproducible MathText font metrics/rules, not pre-rendered formulas.

Requires the reference Matplotlib 3.9.4 only at development time. Runtime is C++.
Use --style NAME for one JSON resource; --rules for the parser tables;
--unicode for shared Unicode metrics and exact reference kerning (fontTools).
"""
import argparse
import json
import logging
import struct
from pathlib import Path
import matplotlib
from matplotlib import _mathtext as mt, ft2font
from matplotlib._mathtext_data import tex2uni
from matplotlib.font_manager import FontProperties

logging.getLogger('matplotlib').setLevel(logging.ERROR)
parser = argparse.ArgumentParser()
parser.add_argument('--style')
parser.add_argument('--rules', action='store_true')
parser.add_argument('--kerning', action='store_true')
parser.add_argument('--unicode', action='store_true')
parser.add_argument('--named-kerning', action='store_true')
parser.add_argument('--font-classes', action='store_true')
parser.add_argument('--units', action='store_true')
parser.add_argument('--raster-hints', type=Path)
args = parser.parse_args()
assert matplotlib.__version__ == '3.9.4', 'Regenerate intentionally when changing reference version'
fonts = mt.DejaVuSansFonts(FontProperties(size=12), ft2font.LOAD_NO_HINTING)
symbols = sorted(set(chr(n) for n in range(32, 127)) |
                 set(chr(n) for n in tex2uni.values()) |
                 set('\\' + name for name in tex2uni))
if args.raster_hints:
    # Reference Agg uses FORCE_AUTOHINT; its vertical metrics differ from the
    # vector NO_HINTING pass. A reusable per-glyph table, never formula images.
    import matplotlib.backends.backend_agg as agg
    units = json.loads((Path(__file__).resolve().parents[1] /
                        'third_party/matplotlib_math/sized_units.json').read_text())
    result = {};offset = 0
    with args.raster_hints.open('wb') as output:
        for name, glyphs in sorted(units.items()):
            font = ft2font.FT2Font(str(Path(matplotlib.get_data_path()) / 'fonts/ttf' / name))
            result[name] = {}
            for code in sorted(glyphs, key=int):
                result[name][code] = offset
                for size in range(1, 4097):
                    font.set_size(size, 72)
                    glyph = font.load_char(int(code), flags=agg.get_hinting_flag())
                    output.write(struct.pack('<ff', glyph.horiBearingY/64,
                                             (glyph.height-glyph.horiBearingY)/64))
                offset += 4096*8
elif args.units:
    from fontTools.ttLib import TTFont
    result = {}
    for sym in sorted(mt.Parser._delims | mt.Parser._wide_accents | {r'\__sqrt__'}):
        for font, char in fonts.get_sized_alternatives_for_symbol('it', sym):
            info = fonts._get_info(font, 'it', char, 12, 72)
            ttf = TTFont(info.font.fname);name = ttf.getBestCmap()[info.num]
            glyph = ttf['glyf'][name]
            result.setdefault(Path(info.font.fname).name, {})[str(info.num)] = [
                ttf['head'].unitsPerEm, ttf['hmtx'][name][0],
                glyph.xMin, glyph.xMax, glyph.yMin, glyph.yMax]
elif args.font_classes:
    # ParserState preserves rm/it/bf/bfit independently from virtual font names.
    # Store only differences from the existing class-it atlas, sharing records.
    from fontTools.ttLib import TTFont
    font_dir = Path(matplotlib.get_data_path()) / 'fonts/ttf'
    candidates = set(symbols)
    for path in font_dir.glob('*.ttf'):
        candidates.update(chr(n) for n in ft2font.FT2Font(str(path)).get_charmap()
                          if 128 <= n <= 0x1ffff)
    candidates = sorted(candidates)
    def record(style, cls, sym):
        try:
            info = fonts._get_info(style, cls, sym, 12, 72)
            if info.num == 0xa4 and sym not in ('¤',):
                return None
            m = info.metrics
            return [Path(info.font.fname).name, info.num, m.advance, m.width,
                    m.height, m.iceberg, m.xmin, m.xmax, m.ymin, m.ymax,
                    m.slanted, info.offset]
        except (ValueError, KeyError):
            return None
    font_pairs = {}
    for path in font_dir.glob('*.ttf'):
        font = TTFont(str(path));pairs = []
        if 'kern' in font:
            for table in font['kern'].kernTables:
                pairs.extend((font.getGlyphID(a), font.getGlyphID(b))
                             for (a, b), value in table.kernTable.items() if value)
        font_pairs[path.name] = pairs
    def kerning(style, cls, mapping):
        by_font, by_code = {}, {}
        for sym, rec in mapping.items():
            if rec is None:
                continue
            by_font.setdefault(rec[0], {}).setdefault(rec[1], []).append(sym)
            by_code.setdefault(rec[1], []).append(sym)
        pairs = {}
        for name, firsts in by_font.items():
            for left, right in font_pairs[name]:
                for a in firsts.get(left, ()):
                    for b in by_code.get(right, ()):
                        value = fonts.get_kern(style, cls, a, 12, style, cls, b, 12, 72)
                        if value:
                            pairs[a+b] = value
        return pairs
    records, indices, glyphs, kerns = [], {}, {}, {}
    for style in ('it', 'rm', 'bf', 'bfit', 'tt', 'cal', 'bb', 'frak', 'scr', 'regular', 'sf', 'default'):
        baseline_style = 'rm' if style in ('sf', 'default') else style
        baseline = {sym: record(baseline_style, 'it', sym) for sym in candidates}
        for cls in ('it', 'rm', 'bf', 'bfit'):
            actual = {sym: record(style, cls, sym) for sym in candidates}
            changed = {sym: rec for sym, rec in actual.items() if rec != baseline[sym]}
            if not changed:
                continue
            mapping = {}
            for sym, rec in changed.items():
                if rec is None:
                    mapping[sym] = -1
                    continue
                key = tuple(rec)
                if key not in indices:
                    indices[key] = len(records);records.append(rec)
                mapping[sym] = indices[key]
            key = style+'@'+cls;glyphs[key] = mapping
            kerns[key] = kerning(style, cls, actual)
    result = {'records': records, 'glyphs': glyphs, 'kerning': kerns}
elif args.unicode or args.named_kerning:
    # Export actual reference fallback/virtual-font decisions, including
    # supplementary-plane characters. Never turn a missing glyph into U+00A4.
    candidates = set()
    font_dir = Path(matplotlib.get_data_path()) / 'fonts/ttf'
    for path in font_dir.glob('*.ttf'):
        candidates.update(ft2font.FT2Font(str(path)).get_charmap())
    candidates = sorted(n for n in candidates if 128 <= n <= 0x1ffff)
    records, indices, styles = [], {}, {}
    for style in ('it', 'rm', 'bf', 'bfit', 'tt', 'cal', 'bb', 'frak', 'scr', 'regular'):
        mapping = {}
        for code in candidates:
            if chr(code) in symbols:
                continue
            try:
                info = fonts._get_info(style, 'it', chr(code), 12, 72)
            except (ValueError, KeyError):
                continue
            if info.num == 0xa4 and code != 0xa4:
                continue
            m = info.metrics
            record = [Path(info.font.fname).name, info.num, m.advance, m.width,
                      m.height, m.iceberg, m.xmin, m.xmax, m.ymin, m.ymax,
                      m.slanted, info.offset]
            key = tuple(record)
            if key not in indices:
                indices[key] = len(records)
                records.append(record)
            mapping[str(code)] = indices[key]
        styles[style] = mapping
    from fontTools.ttLib import TTFont
    kerns = {}
    for style, mapping in styles.items():
        by_font, by_code = {}, {}
        inputs = [(chr(int(code)), records[index]) for code, index in mapping.items()]
        for sym in symbols:
            info = fonts._get_info(style, 'it', sym, 12, 72)
            inputs.append((sym, [Path(info.font.fname).name, info.num]))
        for sym, record in inputs:
            by_font.setdefault(record[0], {}).setdefault(record[1], []).append(sym)
            by_code.setdefault(record[1], []).append(sym)
        pairs = {}
        for name, firsts in by_font.items():
            font = TTFont(str(font_dir / name))
            if 'kern' not in font:
                continue
            # Match the reference's FT get_kerning arguments exactly (info.num),
            # rather than substituting Qt's normal text shaping/kerning rules.
            for table in font['kern'].kernTables:
                for (left, right), adjustment in table.kernTable.items():
                    if not adjustment:
                        continue
                    for a in firsts.get(font.getGlyphID(left), ()):
                        for b in by_code.get(font.getGlyphID(right), ()):
                            if len(a) == len(b) == 1 and a.isascii() and b.isascii():
                                continue
                            value = fonts.get_kern(style, 'it', a, 12, style, 'it', b, 12, 72)
                            if value:
                                pairs[a+b] = value
        kerns[style] = pairs
    result = {'records': records, 'styles': styles, 'kerning': kerns,
              'version': matplotlib.__version__}
    if args.named_kerning:
        result = {style: {key: value for key, value in pairs.items() if key.isascii() and '\\' in key}
                  for style, pairs in kerns.items()}
elif args.kerning:
    result = {}
    for style in ('it', 'rm', 'bf', 'bfit', 'tt', 'cal', 'bb', 'frak', 'scr', 'regular'):
        pairs = {}
        for a in map(chr, range(32, 127)):
            for b in map(chr, range(32, 127)):
                value = fonts.get_kern(style, 'it', a, 12, style, 'it', b, 12, 72)
                if value:
                    pairs[a+b] = value
        result[style] = pairs
elif args.rules:
    names = ('binary_operators', 'relation_symbols', 'arrow_symbols', 'punctuation_symbols',
             'overunder_symbols', 'overunder_functions', 'dropsub_symbols', 'function_names',
             'left_delims', 'right_delims', 'ambi_delims', 'wide_accents', 'small_greek')
    result = {name: sorted(getattr(mt.Parser, '_' + name)) for name in names}
    result.update(spaces=mt.Parser._space_widths, accents=mt.Parser._accent_map,
                  version=matplotlib.__version__)
    result['alternatives'] = {sym: [(str(font), char) for font, char in
                                   fonts.get_sized_alternatives_for_symbol('it', sym)]
                              for sym in sorted(mt.Parser._delims | mt.Parser._wide_accents |
                                                {'\\__sqrt__'})}
else:
    style = int(args.style) if args.style.isdigit() else args.style
    glyphs = {}
    for sym in symbols:
        try:
            info = fonts._get_info(style, 'it', sym, 12, 72)
        except (ValueError, KeyError):
            continue
        m = info.metrics
        glyphs[sym] = [Path(info.font.fname).name, info.num, m.advance, m.width,
                       m.height, m.iceberg, m.xmin, m.xmax, m.ymin, m.ymax,
                       m.slanted, info.offset]
    result = {'glyphs': glyphs, 'xheight': fonts.get_metrics(style, 'it', 'x', 12, 72).iceberg,
              'em': fonts.get_metrics('it', 'it', 'm', 12, 72).advance}
print(json.dumps(result, ensure_ascii=True, sort_keys=True, separators=(',', ':')))
