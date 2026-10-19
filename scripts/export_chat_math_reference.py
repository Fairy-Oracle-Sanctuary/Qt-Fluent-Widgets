"""Actual Pro renderFormula images + metadata for native geometry/ink regression."""
import argparse
import importlib.util
import json
import os
from pathlib import Path

os.environ.setdefault('QT_QPA_PLATFORM', 'offscreen')
os.environ['PYTHONDONTWRITEBYTECODE'] = '1'
from PySide6.QtGui import QGuiApplication
app = QGuiApplication.instance() or QGuiApplication([])
args = argparse.ArgumentParser()
args.add_argument('--images', type=Path, required=True)
args = args.parse_args()
source = Path(__file__).resolve().parents[2] / 'PySide6-Fluent-Widgets-Extend/qfluentwidgets_pro/common/chat_math.py'
spec = importlib.util.spec_from_file_location('reference_chat_math', source)
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)
expressions = [
    'x', 'x^2', 'x_i', 'x_i^2', 'a+b=c', '-2.5', 'x^{a+b}', 'x_{i_j}',
    r'\alpha+\beta=\gamma', r'\Gamma+\Omega', r'\sin x+\cos(x)',
    r'\frac{1}{2}', r'\frac{x+1}{y-1}', r'\frac{1}{\frac{2}{3}}',
    r'\dfrac{a}{b}', r'\binom{n}{k}', r'\sqrt{x}', r'\sqrt{x^2+y^2}',
    r'\sqrt[3]{x}', r'\sum_{i=0}^{n}i', r'\prod_{k=1}^{n}k',
    r'\int_0^1 x^2\,dx', r'\lim_{x\to 0}\frac{\sin x}{x}',
    r'\left(\frac{x}{y}\right)', r'\left[\sqrt{x}\right]',
    r'\left. x\right|_0^1', r'\left( a\middle| b\right)',
    r'\hat{x}', r'\bar{x}', r'\vec{x}', r'\dot{x}', r'\ddot{x}',
    r'\widehat{abc}', r'\widetilde{xyz}', r'\overline{x+y}',
    r'\overset{a}{b}', r'\underset{a}{b}', r"f'(x)",
    r'\mathrm{ABC}+\mathit{abc}', r'\mathbf{ABC}', r'\mathbb{R}',
    r'\mathcal{L}', r'\mathfrak{g}', r'\mathscr{A}', r'\mathtt{code}',
    r'\text{hello world}', r'\operatorname{rank}(A)', r'\boldsymbol{x}',
    r'\substack{a\\b}', r'\genfrac{[}{]}{0.5}{1}{a}{b}',
    r'a\quad b\!c', '^2', r'\frac{a}', r'\unknown{x}', 'x^^2', r'\sqrt{}',
    '', ' ', 'x_{', r'\left(x', r'\input{secret}',
    'AVATAR', r'\mathrm{AVATAR}', r'\text{To WA office}', r'\mathbf{AVATAR}',
    '{x}^2', r'{a+x}_i^n', r'\mathit{x}_i^2',
    'é+ñ', 'Ж+я', 'α+ϑ', 'א+ב', '𝕏+𝒜', 'ℝ+ℂ',
    r'\mathrm{éñ}', r'\text{café}', r'\mathbf{Жя}',
    r'\mathit{é}', r'\mathtt{é}', r'\mathbb{a}',
    r'\frac{é}{Ж}', r'\sqrt{𝕏}', 'é_i^2',
    r'\text{Té AVé}', r'\mathrm{ЖЯ}', r'\mathbf{éñ}',
]
for levels in range(1, 17):
    fraction, script = 'x', 'x'
    for _ in range(levels):
        fraction = r'\frac{1}{' + fraction + '}'
        script = 'x^{' + script + '}'
    expressions.extend((fraction, script))
expressions.extend((
    r'\boldsymbol{123+\Gamma+\alpha+x}', r'\boldsymbol{x^2+\frac{a}{b}}',
    r'\boldsymbol{\sum_{i=0}^{n}x_i}', r'\frac{1}{\substack{a\\b}}',
    r'\sum_{\substack{i=0\\j=1}}^n x', r'\mathring{x}',
    r'\frac{1}{\overline{\frac{1}{\sqrt{x}}}}',
))
for levels in (6, 10, 16):
    root, accent, stack = 'x', 'x', 'x'
    for _ in range(levels):
        root = r'\sqrt{' + root + '}'
        accent = r'\hat{' + accent + '}'
        stack = r'\frac{1}{\substack{a\\' + stack + '}}'
    expressions.extend((root, accent, stack))
expressions.extend((
    r'\mathrm{\mathbb{ABCabc}}', r'\mathbf{\mathbb{ABCabc}}',
    r'\mathit{\mathbb{ABCabc}}', r'\mathbf{\mathfrak{ABCabc}}',
    r'\mathrm{\mathsf{αβΓ}}', r'\mathbf{\mathsf{αβΓ}}',
    r'\mathbfit{\mathbb{ABCabc}}', r'\mathrm{\mathscr{ABCabc}}',
    r'\mathbf{\mathcal{ABC}}', r'\mathbb{A}+\mathrm{\mathbb{A}}+\mathbb{A}',
    r'\alpha\Gamma', r'\mathrm{\alpha\Gamma}',
))
samples = [(expression, 12) for expression in expressions]
for size in (6, 9, 16, 24, 48):
    samples.extend((expression, size) for expression in (
        r'\frac{x_i^2+é}{\sqrt{𝕏}}', r'\sum_{i=0}^n x_i',
        r'\sqrt{\sqrt{\sqrt{\sqrt{\sqrt{x}}}}}', r'\boldsymbol{123+\alpha+x}',
    ))
samples.extend((expression, 12) for expression in (
    r'\boldsymbol{\sum}', r'\boldsymbol{\sum_i}',
    r'\boldsymbol{\frac{\sum_i}{n}}', r'\boldsymbol{\overline{x}}',
))
args.images.mkdir(parents=True, exist_ok=True)
result = []
for i, (expression, size) in enumerate(samples):
    item = {'name': f'math-{i}', 'expression': expression, 'pointSize': size}
    try:
        image = module.renderFormula(expression, pointSize=size)
        item.update(width=image.width(), height=image.height(), dpr=image.devicePixelRatio())
        assert image.save(str(args.images / (item['name'] + '.png')))
    except Exception as error:
        item['error'] = type(error).__name__
    result.append(item)
print(json.dumps(result, ensure_ascii=True, indent=2))
