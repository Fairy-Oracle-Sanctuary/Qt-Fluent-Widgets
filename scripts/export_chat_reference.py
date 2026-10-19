"""Export actual Python splitMessage results without importing GUI dependencies."""
import ast
import json
from pathlib import Path
import re

reference = Path(__file__).resolve().parents[2] / 'PySide6-Fluent-Widgets-Extend/qfluentwidgets_pro/components/widgets/chat_content.py'
node = next(n for n in ast.parse(reference.read_text(encoding='utf-8')).body
            if isinstance(n, ast.FunctionDef) and n.name == 'splitMessage')
namespace = {'re': re}
exec(compile(ast.Module(body=[node], type_ignores=[]), str(reference), 'exec'), namespace)
sources = ['', 'plain', 'a\n```py\nprint(1)\n```\nb', '~~~js\nlet n = 1;',
           '````\na\n```\nb\n````\n', '```python\r\nx=1\r\n```\r\nafter',
           '    ```python\nx=1\n```\n', '   ~~~js metadata\nlet x=1\n   ~~~~\n',
           '```py`invalid\na\n```\n', '~~~py\n~~~ extra\n```\n~~~\n',
           'a\rb\rc', 'a\u2028b\u0085c\x0bd\x0ce', '```\n\n```',
           '```python\n世界 🌍\n```\n~~~unknown\nraw\n~~~\n', '```py\n\n\n```\n']
stream = 'Before 世界 🌍\n```python\nprint(1)\n```\nAfter'
sources.extend(stream[:end] for end in range(len(stream)+1))
print(json.dumps([{'name': f'fence-{i}', 'source': text, 'blocks': namespace['splitMessage'](text)}
                  for i, text in enumerate(sources)], ensure_ascii=True, indent=2))
