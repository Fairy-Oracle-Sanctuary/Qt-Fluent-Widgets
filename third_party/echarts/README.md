# Apache ECharts 5.6.0

`echarts.min.js` is copied unchanged from the local Python Pro reference:
`qfluentwidgets_pro/_rc/js/echarts.min.js`.
SHA256: `10A74668550EB53749CF02EACAA4F5A7EDAE81ECDAE056DECD9E95EF87D92FA4`.
The bundle includes ZRender 5.6.1. No CDN or runtime download is needed.

Original ECharts notices:
- https://github.com/apache/echarts/blob/5.6.0/LICENSE
- https://github.com/apache/echarts/blob/5.6.0/NOTICE
- https://github.com/apache/echarts/blob/5.6.0/licenses/LICENSE-d3
- https://github.com/ecomfe/zrender/blob/5.6.1/LICENSE

`qtfluentwidgets/resources/js/chart.html` retains the reference ECharts bridge.
The C++ host uses Qt 5/6 WebEngine and a native presentation frame gate.
Application-supplied JavaScript options are trusted code, just as in Python.
The standalone component is explicitly linked with `qtfluentwidgets_chart`;
the base library and umbrella header do not require WebEngine.
Set `Qt::AA_ShareOpenGLContexts` before constructing QApplication, then construct
the charts before showing a frameless host. ChartWidget uses FramelessWebEngineView:
it makes the Windows host translucent before WebEngine constructs its surface,
bootstraps the page, and calls the host's updateFrameless(). After host.show(), call
host.setMicaEffectEnabled(host.isMicaEffectEnabled()) to restore its configured Mica
state. The Gallery and chart tests do this. For QOpenGLWidget, call updateFrameless()
after constructing the surface and likewise restore Mica after showing the host.

Gallery examples are exported unchanged by:
`py -3.9 scripts/export_chart_examples.py <Python-Pro-directory>`.
Its JSON output is saved to `app/resources/chart/examples.json`.
