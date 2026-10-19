"""Export pinned Pygments rules/golden families; no Python runtime is shipped.

Run with py -3.9 -X utf8 scripts/export_code_lexer_reference.py rules|fixtures.
The stdout artifact is installed with apply_patch. Regenerate only after
reviewing the Python CodeEdit's tokenFormat and the pinned lexer version.
"""
import json
import sys
import base64
import hashlib
import struct
import zlib
import pygments
from pygments.lexers import get_lexer_by_name
from pygments.token import _TokenType

LANGUAGES = ('python', 'javascript', 'typescript', 'css', 'bash',
             'c', 'cpp', 'csharp', 'java', 'html', 'xml', 'toml', 'ini',
             'powershell', 'sql', 'go', 'rust', 'json', 'yaml', 'markdown')


def family(token):
    token = str(token)
    for prefix, value in (
        ('Token.Text', 'Text'), ('Token.Comment', 'Comment'),
        ('Token.Name.Tag', 'Tag'), ('Token.Name.Attribute', 'Attribute'),
        ('Token.Literal.String', 'String'), ('Token.Keyword.Constant', 'Constant'),
        ('Token.Keyword', 'Keyword'), ('Token.Literal.Number', 'Number'),
        ('Token.Name.Function', 'Function'), ('Token.Name.Class', 'Class'),
        ('Token.Generic.Heading', 'Heading'), ('Token.Generic.Subheading', 'Heading'),
    ):
        if token.startswith(prefix):
            return value
    return 'Name'


def action_data(action, language):
    if action is None:
        return None
    if isinstance(action, _TokenType):
        if language in ('c', 'cpp') and str(action) == 'Token.Name':
            return 'CIdentifier'
        return family(action)
    if action.__name__ == '_handle_codeblock':
        return {'markdownFence': True}
    cells = dict(zip(action.__code__.co_freevars,
                     (cell.cell_contents for cell in (action.__closure__ or ()))))
    if language == 'yaml' and 'token_class' in cells:
        return {'yaml': action.__qualname__.split('.')[1],
                'token': family(cells['token_class']),
                'start': cells.get('start', False), 'implicit': cells.get('implicit', False)}
    if language == 'yaml' and 'indent_token_class' in cells:
        return {'yaml': 'parse_block_scalar_empty_line',
                'token': family(cells['indent_token_class']),
                'content': family(cells['content_token_class'])}
    if 'args' in cells:
        return {'groups': [action_data(item, language) for item in cells['args']]}
    # Python soft-keyword patterns use bygroups(..., using(this), ...).
    if set(cells) == {'gt_kwargs', 'kwargs'} and not cells['kwargs'] and set(cells['gt_kwargs']) <= {'stack'}:
        return {'using': 'self', 'stack': list(cells['gt_kwargs'].get('stack', ('root',)))}
    if set(cells) == {'_other', 'gt_kwargs', 'kwargs'} and not cells['gt_kwargs'] and not cells['kwargs']:
        nested = cells['_other']().aliases[0]
        if nested not in LANGUAGES:
            raise ValueError('Missing embedded lexer: ' + nested)
        return {'using': 'language', 'language': nested}
    raise ValueError('Unsupported lexer callback: ' + repr(cells))


def export_rules():
    result = {}
    for language in LANGUAGES:
        lexer = get_lexer_by_name(language)
        result[language] = {'states': {}, 'identifierTypes': [], 'aliases': lexer.aliases}
        if language == 'json':
            continue  # JsonLexer is a dedicated streaming state machine.
        if language in ('c', 'cpp'):
            types = set()
            for enabled, names in [('stdlibhighlighting', 'stdlib_types'),
                                   ('c99highlighting', 'c99_types'),
                                   ('c11highlighting', 'c11_atomic_types'),
                                   ('platformhighlighting', 'linux_types')]:
                if getattr(lexer, enabled):
                    types.update(getattr(lexer, names))
            result[language]['identifierTypes'] = sorted(types)
        for state, rules in lexer._tokens.items():
            result[language]['states'][state] = [
                {'pattern': match.__self__.pattern, 'flags': match.__self__.flags,
                 'action': action_data(action, language), 'next': transition}
                for match, action, transition in rules
            ]
    print('// Generated from Pygments 2.19.2; see third_party/pygments/LICENSE.')
    print('// Regenerate with scripts/export_code_lexer_reference.py rules.')
    print('#pragma once\nnamespace qfw { namespace code_lexer_data {')
    payload = json.dumps(result, ensure_ascii=True, separators=(',', ':')).encode('ascii')
    print('// Uncompressed rule JSON SHA256: ' + hashlib.sha256(payload).hexdigest())
    print('inline constexpr const char* rules[] = {')
    # qCompress framing: big endian raw length followed by a zlib stream.
    encoded = base64.b64encode(struct.pack('>I', len(payload)) + zlib.compress(payload, 9)).decode('ascii')
    for start in range(0, len(encoded), 16000):
        print('R"QFWLEX(' + encoded[start:start + 16000] + ')QFWLEX",')
    print('};\n} }')


CASES = [
    ('json-key-comments', 'json', '{"世界 🌍" /* comment */ : true, "value": [null, false, -1.5e+2]}'),
    ('json-queued-strings', 'json', '"first" "second" // comment\n : "value"'),
    ('json-escapes', 'json', '{"a\\u0041": "\\\\\\\" 🌍", "invalid\\u12xx": "ok"}'),
    ('json-partial', 'json', '{"key": "unterminated\\u12'),
    ('json-invalid', 'json', '/broken +1e--2 truefalse nullt undefined /* unfinished'),
    ('json-comment-eof', 'json', '"value" // comment at EOF'),
    ('yaml-block', 'yaml', 'message: |\n  Hello 世界 🌍\n  second\nnext: true\n'),
    ('yaml-indent', 'yaml', 'root:\n  - name: one\n    values:\n      - a\n      - b\n  - name: two\nend: 2\n'),
    ('yaml-directives', 'yaml', '%YAML 1.2\n%TAG !e! tag:example.com,2026:\n---\n!e!foo &anchor {a: 1, b: [true, *anchor]}\n...\n'),
    ('yaml-scalars', 'yaml', 'a: >2-\n  one\n    two\n   \n\nb: |+\n    世界 🌍\nnext: plain\n  continuation\nlast: end\n'),
    ('yaml-quotes', 'yaml', 'single: \'it\'\'s fine\'\ndouble: "a\\u0041\\n 世界 🌍"\nflow: ["a b", {key: value}, !tag item]\n'),
    ('yaml-invalid-indent', 'yaml', 'a:\n    b: c\n  invalid: d\n\tbroken\nkey: "unterminated\nnext: text'),
    ('markdown-inline', 'markdown', '# Hello\n## 世界 🌍\n**bold** _italic_ ~~gone~~ `code`\n![image](asset.png) [link][id]\n[id]: https://example.com\n'),
    ('markdown-lists', 'markdown', 'Title\n=====\nSubtitle\n--------\n- [x] done `code`\n1. **item**\n> quoted\n@user #topic\n'),
    ('markdown-fence-offset', 'markdown', '# Hello 🌍\nText before\n```python extra metadata\nreturn True\n```\nText after\n'),
    ('markdown-fence-json', 'markdown', '```json\n{"🌍": true}\n```\n```yaml\nmessage: |\n  hello\n```\n'),
    ('markdown-fence-unknown', 'markdown', '```unknown_language\nraw text\n```\n```\nplain code\n```\n'),
    ('markdown-partial', 'markdown', '# Heading without newline\n```python\nreturn True'),
    ('c-types', 'c', 'size_t count; uint32_t value; atomic_int ready; __u64 bits; bool valid;'),
    ('c-preprocessor', 'c', '#include <stdio.h>\n#define SAY(x) printf("%s\\n", x)\n#if 0\ninvalid ! text\n#endif\nint main(void) { return 0; }'),
    ('c-functions', 'c', '/* 中文 🌍 */\nint sum(int a, int b) { return a + b; }\nconst char *name = "hello\\n";'),
    ('cpp-raw-types', 'cpp', 'std::size_t count; auto text = u8R"tag(Hello 🌍\n世界)tag";\nconstexpr bool ready = true;'),
    ('cpp-template', 'cpp', 'template<typename T> class Box { public: T value; };\nnamespace app { auto run() -> int { return 42; } }'),
    ('cpp-preprocessor', 'cpp', '#define VALUE 0xffULL\n#if 0\nignored text\n#endif\n// comment\nint main() { return VALUE; }'),
    ('cpp-raw-boundaries', 'cpp', 'auto a = R"abcdefghijklmnop(ok)abcdefghijklmnop";\nauto b = R"abcdefghijklmnopq(invalid)abcdefghijklmnopq";\nauto c = R"(empty delimiter)";'),
    ('csharp-interpolation-reference', 'csharp', 'class Demo { string text = $"hello {true}"; string path = @"C:\\temp\\file"; }'),
    ('csharp-attributes', 'csharp', '[Obsolete("demo")]\nnamespace App { public class Demo<T> { async Task Run() { await task(); } } }'),
    ('csharp-verbatim', 'csharp', 'string text = @"hello ""世界 🌍""\nworld";\nvar value = 42.5m; // note'),
    ('java-members', 'java', 'import java.util.List;\nclass Demo { void run() { System.out.println("Hello 🌍"); } }'),
    ('java-annotations', 'java', '@Override\npublic <T> T value(T input) { return input; }\nint n = 0xff; boolean ready = true;'),
    ('java-text-block', 'java', 'String value = """\nHello 世界\n"""; /* comment\nend */'),
    ('html-embedded', 'html', '<style>.card:hover { color: var(--accent, red); }</style>\n<script>const x = `hello ${true}`;</script>'),
    ('html-attributes', 'html', '<!DOCTYPE html>\n<!-- comment -->\n<div class="card" data-value=42 disabled>世界 🌍 &amp;</div>'),
    ('html-multiline', 'html', '<script>\nconst pattern = /hello/i;\nconst text = `a\n${value}`;\n</script>\n<style>\n@media screen { body { color: #fff; } }\n</style>'),
    ('xml-namespaces', 'xml', '<?xml version="1.0"?>\n<app:config xmlns:app="urn:app" enabled="true">世界 🌍 &amp;</app:config>'),
    ('xml-cdata', 'xml', '<root><![CDATA[<hello>\nworld]]><!-- note\nend --></root>'),
    ('xml-doctype', 'xml', '<!DOCTYPE root SYSTEM "config.dtd">\n<root value=\'quoted\'/>'),
    ('toml-dates', 'toml', '[window]\ntime = 1979-05-27T07:32:00Z\nsize = [900, 700]\nready = true'),
    ('toml-multiline', 'toml', 'title = """Hello\n世界 🌍"""\nraw = \'\'\'C:\\temp\nfile\'\'\''),
    ('toml-tables', 'toml', '[[products]]\n"quoted.key" = { color = "red", width = 2.5 } # note\ncount = 0xFF'),
    ('ini-sections', 'ini', '; comment\n[window]\ntitle = 世界 🌍\nwidth: 900\nenabled=yes'),
    ('ini-continuation', 'ini', '[app]\nvalue = first\n    second\n# note\nother = hello'),
    ('ini-invalid', 'ini', '[broken\nplain text\nkey=\"quoted\"'),
    ('powershell-here', 'powershell', '$value = @"\nHello $name 世界 🌍\n"@\nWrite-Output $value'),
    ('powershell-functions', 'powershell', 'function Get-Value { param([int]$count) if ($count -gt 0) { return $true } }\nGet-ChildItem | ForEach-Object { $_.Name }'),
    ('powershell-comments', 'powershell', '<# multi\ncomment #>\n$name = \'literal $value\'\nWrite-Host "Hello $(Get-Date) $env:USER"'),
    ('sql-keywords', 'sql', 'SELECT COUNT(*) AS total FROM users WHERE active = TRUE AND name IS NOT NULL;'),
    ('sql-strings', 'sql', '-- note\nSELECT \'世界 🌍\', \'it\'\'s\', "column", 42.5e2; /* multi\ncomment */'),
    ('sql-schema', 'sql', 'CREATE TABLE users (id INTEGER PRIMARY KEY, name VARCHAR(255));\nINSERT INTO users VALUES (1, \'Hello\');'),
    ('go-declarations', 'go', 'package main\nimport "fmt"\nfunc main() { fmt.Println("Hello 世界 🌍") }'),
    ('go-raw', 'go', 'var text = `hello\n世界 🌍`; var ch = \'界\'; var n = 0xFF; // note'),
    ('go-types', 'go', 'type User struct { Name string; Active bool }\nfunc value[T any](input T) T { return input }\nvar ready = true'),
    ('rust-nested', 'rust', '/* outer /* inner */ still */\nlet value = r##"Hello 🌍\n世界"##;'),
    ('rust-lifetimes', 'rust', "pub fn borrow<'a>(value: &'a str) -> &'a str { value }\nlet ch = '界'; let escaped = '\\n';"),
    ('rust-macros', 'rust', '#[derive(Debug)]\nstruct User { name: String }\nfn main() { println!("hello {}", 42u32); let value = Some(true); }'),
    ('python-format', 'python', 'f"{value!r:>{width}.{precision}f} / {{literal}}"'),
    ('python-lambda-format', 'python', 'f"{(lambda x: x + 1)(2):04d}"'),
    ('python-debug', 'python', 'f"{value = }"'),
    ('python-dict-slice', 'python', 'f"{data[1:3]} { {\'key\': 2} }"'),
    ('python-raw-format', 'python', 'rf"{value!s:0{width}d} \\n {{literal}}"'),
    ('python-multiline', 'python', 'f"""Hello 🌍\n{value!r:>{width}}\n世界"""'),
    ('python-soft-keyword', 'python', 'match value:\n    case {"name": name} if name:\n        print(name)\n'),
    ('python-imports', 'python', 'from pathlib import Path as P\nclass Demo:\n    async def hello(self):\n        return await task()\n'),
    ('js-jsx-reference', 'javascript', 'const view = <div className="card">{items.map(x => <span>{x.name}</span>)}</div>;'),
    ('ts-jsx-reference', 'typescript', 'const view = <Panel title="Hello">{user.name}</Panel>;'),
    ('js-regex-division', 'javascript', 'const pattern = /[a-z]+\\/test/g;\nconst ratio = value / 2;\nreturn /hello/i;'),
    ('js-nested-template', 'javascript', 'const value = `hello ${ready ? `nested ${name}` : "world"}`;'),
    ('ts-types', 'typescript', 'interface User { name: string; active: boolean; }\nconst user: User = {name: "世界 🌍", active: true};'),
    ('css-media-selector', 'css', '@media (min-width: 600px) { #app > .card:hover::before, input[type="text"] { --accent: #0078d4; color: var(--accent); width: calc(100% - 2rem); } }'),
    ('css-functions', 'css', '.card { background: linear-gradient(90deg, red, rgb(0, 120, 212)); padding: clamp(1rem, 2vw, 3rem); color: var(--accent, blue); }'),
    ('css-urls', 'css', '@import url("theme.css");\n.card { background: url(images/card.png); color: blue !important; }'),
    ('css-custom-unknown', 'css', '.card { --accent: #fff; unknown-property: custom-value; transition-property: color; }'),
    ('css-comments-keyframes', 'css', '/* theme\ncomment */\n@keyframes fade { from { opacity: 0; } to { opacity: 1; } }'),
    ('bash-expansions', 'bash', 'echo "${name:-world} ${array[@]} $(date +%Y) $((count + 1)) $HOME"\necho ${path##*/} ${name/foo/bar}'),
    ('bash-nested', 'bash', 'echo "${name:-${fallback}} $(echo "${user}")"\nvalue=$((16#ff + count * 2))'),
    ('bash-quotes', 'bash', 'echo \'${literal}\' "$HOME" \\${escaped} $\'hello\\n\' `echo done`'),
    ('bash-heredoc', 'bash', 'cat <<EOF\nhello $USER\nEOF\necho done\n'),
    ('bash-condition', 'bash', '#!/bin/bash\nname="Hello"\nfor file in *.txt; do\n    echo "$name: $file"\ndone\n'),
]

# Editing leaves documents incomplete. Cover cuts around quotes, escapes,
# indentation, headers and closing fences rather than only valid whole files.
for language, source in [
    ('json', '{"🌍\\u0041" /*comment*/ : [true, -1.5e+2, "value"]}'),
    ('yaml', 'a: |2-\n  世界 🌍\n   \nnext:\n  - name: "hello\\u0041"\nend: true\n'),
    ('markdown', '# 世界 🌍\n```python metadata\nf"{True}"\n```\n[link](target)\n'),
]:
    for number, end in enumerate(sorted({0, 1, 2, 3, 5, 8, 12, 18, 25, len(source)//2, len(source)-2, len(source)-1})):
        CASES.append((language + '-editing-cut-' + str(number), language, source[:end]))
for language in LANGUAGES:
    sample = next(source for _, name, source in CASES if name == language)
    CASES.append(('markdown-embedded-' + language, 'markdown', 'Before 世界 🌍\n```' + language + '\n' + sample + '\n```\nAfter\n'))


def export_fixtures():
    result = []
    for name, language, source in CASES:
        lexer = get_lexer_by_name(language)
        if language == 'markdown':
            # Fix only the upstream callback's documented relative-offset bug.
            # Keep all original patterns and all other token families unchanged.
            from pygments.lexers.markup import MarkdownLexer
            from pygments.token import String, Whitespace, Text
            from pygments.util import ClassNotFound
            def fence(self, match):
                for group, token in [('initial', String.Backtick), ('lang', String.Backtick),
                                     ('whitespace', Whitespace), ('extra', Text), ('newline', Whitespace)]:
                    if match.group(group) is not None:
                        yield match.start(group), token, match.group(group)
                try:
                    nested = get_lexer_by_name(match.group('lang').strip())
                except ClassNotFound:
                    yield match.start('code'), String, match.group('code')
                else:
                    for index, token, value in nested.get_tokens_unprocessed(match.group('code')):
                        yield match.start('code') + index, token, value
                yield match.start('terminator'), String.Backtick, match.group('terminator')
            class OffsetMarkdownLexer(MarkdownLexer):
                tokens = {'root': [((rule[0], fence, *rule[2:])
                                   if isinstance(rule, tuple) and rule[1] is MarkdownLexer._handle_codeblock else rule)
                                   for rule in MarkdownLexer.tokens['root']]}
            lexer = OffsetMarkdownLexer()
        codepoints = ['Text'] * len(source)
        for index, token, value in lexer.get_tokens_unprocessed(source):
            for pos in range(index, min(index + len(value), len(source))):
                codepoints[pos] = family(token)
        utf16 = [kind for char, kind in zip(source, codepoints)
                 for _ in range(2 if ord(char) > 0xffff else 1)]
        result.append({'name': name, 'language': language, 'source': source, 'families': utf16})
    print(json.dumps({'pygments': pygments.__version__, 'cases': result}, ensure_ascii=True, indent=2))


if pygments.__version__ != '2.19.2':
    raise SystemExit('Expected Pygments 2.19.2; review the rule upgrade before regenerating')
if len(sys.argv) != 2 or sys.argv[1] not in ('rules', 'fixtures'):
    raise SystemExit('Usage: export_code_lexer_reference.py rules|fixtures')
export_rules() if sys.argv[1] == 'rules' else export_fixtures()
