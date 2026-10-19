# Pygments lexer rules

CodeEdit includes native lexer implementations for all 20 exposed languages from Pygments
2.19.2: Python, JavaScript, TypeScript, CSS, Bash, C, C++, C#, Java, HTML, XML,
TOML, INI, PowerShell, SQL, Go, Rust, JSON, YAML and Markdown. C/C++ also use the reference's enabled
standard/platform type sets; HTML delegates embedded script/style to JS/CSS.
The C++ engine executes these tables using Qt
Core; deployed applications do not require Python or Pygments. JSON uses the
reference's streaming state machine; YAML preserves its indentation context.
Markdown preserves the reference rules and delegates fences to the 20 exposed
lexers (including their exported aliases). Other fence languages use the plain
string fallback. The upstream Markdown callback's relative code offset bug is
corrected, and golden fixtures explicitly normalize only that callback offset.

`scripts/export_code_lexer_reference.py rules` exports the embedded rule header;
`fixtures` exports the differential regression data. The generator requires
Pygments 2.19.2 and rejects unsupported callbacks. The generated files record
their reference version, and the rule header records the uncompressed SHA256.

Keep LICENSE and AUTHORS with redistributed source and binaries. The release
packaging script copies both notices into `licenses/pygments`.
