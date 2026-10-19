"""Export original Pro ECharts examples without importing the Qt Gallery."""
import argparse
import json
import runpy
from pathlib import Path

parser = argparse.ArgumentParser()
parser.add_argument('reference', type=Path)
args = parser.parse_args()
source = runpy.run_path(str(args.reference / 'gallery/view/chart/charts_js.py'))
runpy.run_path(str(args.reference / 'gallery/common/resource.py'))
names = ['line', 'bar', 'pie', 'scatter', 'kline', 'radar', 'boxplot', 'heat',
         'relation', 'funnel', 'gauge', 'calendar']
result = {}
for name in names:
    result[name] = source['get_relation_chart_option']() if name == 'relation' else source[name + '_chart']
print(json.dumps(result, ensure_ascii=False, indent=2))
