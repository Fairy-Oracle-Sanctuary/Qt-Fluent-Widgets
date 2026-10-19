# Python 扩展组件的 C++ 迁移

参考实现：[PySide6-Fluent-Widgets-Pro](https://github.com/Fairy-Oracle-Sanctuary/PySide6-Fluent-Widgets-Pro)。
主体采用原生 Qt Widgets / QPainter，不依赖 Python 运行时；可选图表模块按原版使用 Qt WebEngine / ECharts。
按类别逐批接入；本页只列出实际完成的组件。

## 第一批：扩展按钮

已迁移并接入 Gallery「基本输入」页面顶部：

- `FilledPushButton`、`FilledToolButton`：五种配色，背景/边框动画。
- `TextPushButton`、`TextToolButton`：五种配色，文字/图标和悬停动画。
- `OutlinedPushButton`、`OutlinedToolButton`：可勾选描边按钮，选中时使用主题色。
- `RoundPushButton`、`RoundToolButton`：胶囊/圆形外观；工具按钮支持 `setHasBorder(false)`。
- `HyperlinkToolButton`：点击或键盘激活时打开 `setUrl()` 指定的链接。
- `LuminaPushButton`：光晕动画，支持 `setGlowColor()`；布局需要预留 45px 光晕空间。

所有类可通过 `qtfluentwidgets.h` 导入，也可直接包含 `components/widgets/extended_button.h`。
构造方法遵循现有 C++ 控件风格，支持 `QIcon` / `FluentIconBase`，继承原有按钮的信号和图标所有权管理。
内置配色字符串与 Python 相同：`information`、`success`、`attention`、`warning`、`error`，默认 `attention`。

```cpp
#include "qtfluentwidgets.h"

auto* save = new qfw::FilledPushButton(
    qfw::FluentIcon(qfw::FluentIconEnum::Save), QObject::tr("Save"), parent);
save->setColorScheme(QStringLiteral("success"));

auto* selected = new qfw::OutlinedPushButton(QObject::tr("Select"), parent);
selected->setChecked(true);

auto* more = new qfw::RoundToolButton(
    qfw::FluentIcon(qfw::FluentIconEnum::Add), parent);
more->setHasBorder(false);
```

## 第二批：进度按钮和标签

已迁移并接入 Gallery「基本输入」：

- `ProgressPushButton`：确定进度环和停止图标；值限制在 0–100，支持进度动画。
  默认不会因点击自行开始；`setAutoProgressEnabled(true)` 后空闲点击开始，运行中点击只发出 `stopRequested()`。
  达到 100 也不会擅自结束任务，由业务代码调用 `setProgressing(false)`。
- `IndeterminateProgressPushButton`：默认显示循环进度弧，支持 `start()`、`stop()` 和 `isSpinning()`。
  隐藏或禁用时暂停刷新，重新显示/启用后按原状态恢复。
- `Chip`：可勾选标签，支持图标、鼠标和键盘切换；独立的 `closed()` 信号不会改变选中状态或自动删除控件。
  Gallery 演示关闭后隐藏和恢复；业务中可自行连接移除逻辑。
- `Tag`：纯展示状态标签，不是按钮；`setType()` 支持 `information`、`success`、`warning`、`error`、`progress`。

对应头文件为 `progress_button.h`、`chip.h` 和 `tag.h`，均由 `qtfluentwidgets.h` 导出。

```cpp
auto* download = new qfw::ProgressPushButton(QObject::tr("Download"), parent);
download->setAutoProgressEnabled(true);
QObject::connect(download, &qfw::ProgressPushButton::stopRequested, download,
                 [download] { download->setProgressing(false); });
// 实际进度由业务更新：download->setValue(42);

auto* chip = new qfw::Chip(QObject::tr("Category"), parent);
QObject::connect(chip, &qfw::Chip::closed, chip, &QWidget::hide);
auto* status = new qfw::Tag(QObject::tr("Success"), parent);
status->setType(QStringLiteral("success"));
```

## 第三批：加载占位和容器

已迁移并接入对应 Gallery 页面：

- 「状态和信息」：`SkeletonWidget`、`ArticleSkeleton`、`CirclePersonalInfoSkeleton`、`RectanglePersonalInfoSkeleton`。
  使用原生绘制和 30° 顺时针倾斜扫光；隐藏时暂停动画，支持绝对/相对占位几何、自定义颜色和动画时长。
- 「文本」：`Watermark`。覆盖在目标 QWidget 上，不拦截鼠标；自动跟随目标尺寸和子控件层级，缓存高 DPI 文本图块。
  支持文本、旋转角度、透明度、间距、亮/暗颜色、重新绑定目标，以及普通 `show()` / `hide()`。
- 「对话框和弹出窗口」：`Drawer`。父控件内的四方向动画抽屉，带阴影、渐变遮罩；支持遮罩点击、Esc、关闭按钮、焦点恢复。
  不创建独立窗口；`open()` / `close()` 是动画操作，`hide()` 立即收起。通过 `viewLayout` 或 `addWidget()` 填入内容。
- 「布局」：`DashboardCardWidget`，基于 `SimpleCardWidget`，包含图标、标题、说明、无文字开关和自定义内容布局。
  `setCardBackgroundColor()` 支持亮/暗主题，`resetCardBackgroundColor()` 恢复基础卡片背景。
  切换主题时停止旧背景动画；标题保留半粗体，说明为 12px。Gallery 对齐 Python 的 400px 展示宽度、彩色图标，以及暖色 `#fff7e8` / `#34312a`、冷色 `#edf7fa` / `#282e30` 背景。
- 「布局」：`ToolBox`。整条标题可点击，悬停只改变箭头背景；原生裁切展开动画不挤压内部控件。
  首项默认展开；切换时先收起旧项，再展开新项，动画中也只显示一个内容区。当前项再次点击全部收起，索引为 -1。
  支持插入、移除、禁用、标题/图标修改、动画反向切换和动态内容高度；移除会隐藏并解除页面父子关系，不删除页面。

```cpp
auto* placeholder = new qfw::SkeletonWidget(parent);
placeholder->addEllipse(QRectF(0, 0, 100, 100));
placeholder->addRect(QRectF(120, 10, 280, 30));
placeholder->addRect(QRectF(120, 60, 140, 30));
placeholder->setFixedSize(400, 100);

auto* watermark = new qfw::Watermark(QStringLiteral("Internal use"), parent);
watermark->setOpacity(0.12);

auto* drawer = new qfw::Drawer(QObject::tr("Notifications"), parent);
drawer->addWidget(new qfw::BodyLabel(QObject::tr("No more notifications")));
drawer->open(qfw::DrawerPosition::RIGHT);

auto* tools = new qfw::ToolBox(parent);
tools->addItem(new qfw::LineEdit(), QObject::tr("First"));
tools->addItem(new qfw::LineEdit(), QObject::tr("Second"));
tools->setCurrentIndex(1);
```

公开类均由 `qtfluentwidgets.h` 导出。内置可见文本保持英文并使用 `tr()`；Gallery 和库的中文 TS/QM 同步更新。
尺寸、时长、颜色等无效参数会抛出 `std::invalid_argument`，ToolBox 无效索引抛出 `std::out_of_range`。

## 第四批：扩展输入

已迁移 6 个公开组件，均为原生 Qt Widgets：

- 「基本输入」：`RatingWidget` 显示数字评分和单颗矢量星星，支持任意有限分数、0–6 位显示精度和亮/暗星星颜色。
- 「基本输入」：`InteractiveRatingWidget` 显示 5 颗连续填充星星，支持悬停预览、点击提交、只读、键盘及 RTL；不绘制点击后的焦点边框。
- 「基本输入」：`ToolTipSlider` 继承原有 `Slider`，悬停或调整值时显示提示，直接拖动手柄也可更新数值。
  提示随控件隐藏/禁用而收起，并由控件拥有，避免切换页面后残留。
- 「基本输入」：`RangeSlider` 支持横/纵双手柄、范围内选值、最小间距、拖动提示和键盘操作。
  `rangeChanged(low, high)` 表示选中的两端变化；`setRange()` 修改允许的边界并钳制选择，包含单点范围和完整 int 范围。
  默认间距为 1，边界跨度不足时实际间距自动缩小；违反间距的 `setValues()` 被忽略。
  鼠标选择最近手柄；Ctrl+左右键选择低/高手柄，再用方向键调整。纵向与 Python 一样，从顶部最小值到底部最大值。
- 「文本」：`LabelLineEdit` 带可点击的前后缀按钮，支持修改、清空、清除按钮和附加 QAction。
  编辑区边距自动避开这些按钮，前后缀文本由调用者提供，不自动翻译。
- 「文本」：`PinBox` 是多个原生单字符输入框，支持输入移焦、退格、左右键、Ctrl+V 粘贴、校验器、密码显示和尺寸配置。
  单格使用 `PinBoxLineEdit` 专用外观，聚焦时整圈 2px 主题色边框，宽高各缩小 2px，失焦后恢复配置尺寸，与 Python 一致。
  不使用普通 LineEdit 的底边高亮和 10px 内边距；`textChanged(QStringList)` 提供逐格文本，`text()` 拼接所有字符。
  默认 4 格，`setPinBoxCount()` 支持 1–128 格并清空旧内容；重建后保留尺寸、校验器和显示模式。
  校验器为借用对象，不接管其所有权，可将其 parent 设为 PinBox。程序赋值和粘贴先校验，再原子更新内容。

Gallery 为评分、滑块和 PIN 设置了初始值，横向示例限制宽度，纵向滑块限制高度。
内置英文文本通过 `tr()` 翻译；TS/QM 和编译资源同步更新。

```cpp
auto* rating = new qfw::InteractiveRatingWidget(3.5, parent);
QObject::connect(rating, &qfw::RatingWidget::valueChanged, parent,
                 [](qreal value) { /* 保存评分 */ });

auto* interval = new qfw::RangeSlider(Qt::Horizontal, parent);
interval->setRange(0, 100);
interval->setValues(20, 80);
interval->setMaximumWidth(420);

auto* address = new qfw::LabelLineEdit(QStringLiteral("https://"), QStringLiteral(".org"), parent);
address->setText(QStringLiteral("fairy"));

auto* pin = new qfw::PinBox(parent);
pin->setPinBoxCount(6);
pin->setEchoMode(QLineEdit::Password);
```

对应头文件为 `rating_widget.h`、`extended_slider.h`、`extended_line_edit.h`，由 `qtfluentwidgets.h` 导出。
为前后缀扩展，原有 `LineEdit::adjustTextMargins()` 改为 protected virtual；普通 LineEdit 的边距逻辑不变。

## 第五批：副标题选项和日期时间输入

新增并接入 Gallery 的 8 个公开组件：

- 「基本输入」：`SubtitleCheckBox`、`SubtitleRadioButton`。标题与浅色副标题分开绘制，副标题自动换行并提供 `heightForWidth()`；整块内容可以点击。

`SubtitleCheckBox` 默认标题/副标题间距为 2px，避免标题行留白叠加后显得过远；仍可通过 `setMargin()` 自定义，换行与高度计算同步使用该间距。
  保留原生复选框的三态、键盘和单选按钮互斥分组行为，支持 RTL、禁用和亮暗主题。
  `setSubText()` 更新副标题与无障碍说明；`setMargin()` 控制两行之间的距离（0–64px）。
- 「日期和时间」：`DateEdit`、`TimeEdit`、`DateTimeEdit`，使用内嵌增减按钮。
- 「日期和时间」：`CompactDateEdit`、`CompactTimeEdit`、`CompactDateTimeEdit`，使用紧凑箭头和带阴影、淡入动画的增减浮层。
  所有日期时间类继承对应的 Qt 原生编辑器，保留 `setDisplayFormat()`、字段选择、输入校验、上下界、闰年和本地化功能；按钮调整当前选中的字段。
  紧凑按钮不抢编辑焦点，浮层打开前后保留选中的字段，避免 Qt 5 自动重置到第一个字段。
  支持只读、隐藏符号、长按重复、自定义聚焦底边颜色与错误状态。内嵌/紧凑按钮和文本边距支持 RTL。

已有 `CompactSpinBox` / `CompactDoubleSpinBox` 也加入演示，并修复原来的紧凑按钮信号连接。
紧凑浮层在控件隐藏、禁用、只读、移动、改变尺寸或隐藏符号时收起；关闭浮层后的焦点恢复不会立即重新打开它。
浮层及其动画由编辑器管理，即使平台将浮层重新设为顶层子控件，销毁编辑器时也会一并释放。
数值/日期类的 `setAccelerated()` 同时配置原生编辑器和自定义按钮，解除原有多继承接口歧义。
只读切换不会丢失调用者手动隐藏符号的设置。

```cpp
auto* history = new qfw::SubtitleCheckBox(
    QObject::tr("Keep history"), QObject::tr("Store recent conversations locally."), parent);
history->setMaximumWidth(400);

auto* date = new qfw::CompactDateEdit(parent);
date->setDisplayFormat(QStringLiteral("yyyy-MM-dd"));
date->setDate(QDate::currentDate());
date->setAccelerated(true);
date->setFixedWidth(180);
```

对应头文件为 `subtitle_button.h` 和 `date_time_edit.h`，由 `qtfluentwidgets.h` 导出。
Gallery 示例限制宽度、设置初始值，内置英文文本与中文 TS/QM 同步更新。
Issue #13 的临时 Gallery 页面及侧栏/顶部导航入口已移除，不影响底层 FlowLayout 和卡片组件。

## 第六批：拖放文件和文件夹选择

新增 `DropMultiFilesWidget`、`DropSingleFileWidget`、`DropSingleFolderWidget`、
`DropMultiFoldersWidget` 和 `DropAnyWidget`，接入 Gallery「基本输入」。
保留 Python 的圆角虚线边框、居中说明、浏览按钮与 `viewLayout` / `label` / `orLabel` / `button` 接口。
可配置边框颜色、宽度、圆角、实线/虚线、默认目录与浏览过滤器；隐藏或禁用时取消尚未完成的选择。

`draggedChange(QStringList)` 只报告拖入的路径，`selectionChange(QStringList)` 只报告浏览选定的路径。
仅接受实际存在的本地文件/文件夹，按组件类型筛选、去重；单选取第一个有效路径，取消或无有效路径不发信号。
浏览过滤器与 Python 一样不限制拖入路径。组件仅报告路径，不复制、移动、上传或删除文件；拖放只接受 CopyAction。

Python 原文件中多个文件组件误用文件夹对话框、单文件误用多选对话框，这里按组件名称落实单/多选规则。
所有浏览窗口采用 Qt 自带非原生 QFileDialog，异步打开；多文件夹和混合路径模式可在列表里按 Ctrl/Shift 多选，
也可双击文件夹继续浏览。内置说明为英文 `tr()` 文本，中文 TS/QM 同步更新；示例宽度 360px。

```cpp
auto* drop = new qfw::DropSingleFileWidget(parent);
drop->setFileExtensions(QStringLiteral("*.png;*.jpg"), QObject::tr("Images"));
QObject::connect(drop, &qfw::DropMultiFilesWidget::draggedChange, parent,
                 [](const QStringList& paths) { /* 读取 paths.first() */ });
QObject::connect(drop, &qfw::DropMultiFilesWidget::selectionChange, parent,
                 [](const QStringList& paths) { /* 处理浏览选定的文件 */ });
```

日期选择器的月份对齐保留 Python 行为。移除「文本」页之前用于代替 DateEdit / TimeEdit 的重复选择器示例，
日期选择器及新的日期时间编辑器集中在「日期和时间」页。

第六批测试覆盖五种选择规则、路径去重/远程路径拒绝、拖放只读路径与 CopyAction、
取消/重复打开/隐藏/禁用/销毁，以及列表和详细视图的单选、多选文件、多选文件夹和混合选择；另验证中英文与亮暗主题。

## 第七批：轻量筛选条和数字分页器

新增 `ExclusiveLiteFilter`、`OutlinedExclusiveLiteFilter`、`MultiSelectionLiteFilter`、
`OutlinedMultiSelectionLiteFilter`、`Pager` 和 `PageButton`，接入 Gallery「导航」。
胶囊按钮复用已有 `PillPushButton`，描边样式复用第一批 `OutlinedPushButton`，不重复移植。

筛选条高 36px，内部选项高 32px，横向溢出时可以滚轮/触控板平滑滚动，尾部箭头滚动 100px；
滚动区内容禁用自动背景填充，空隙透出所在页面背景，不受系统亮/暗调色板影响。
到底后用同尺寸占位保持宽度稳定，键盘聚焦自动滚入视野，支持 RTL。
单选默认选中首项，点击当前项不取消；多选默认不选中，Gallery 设置了两个初始选项。
`addItem()` 去重并返回按钮，`items()` 保持插入顺序；多选结果也按插入顺序输出。
支持 `removeItem()`、`clear()` 和多选的 `clearSelection()`，只在实际选项状态改变时发出信号。
清空会报告空选择，移除当前单选项会选择剩余首项；直接删除选项也能修复内部状态。

数字分页器是原生 QWidget / QPainter，保留前后/首尾导航、窗口化页码、省略号和页码输入。
非空时初始页为 1，页数为 0 时当前页为 0、禁用导航和输入；页数减少会钳制当前页。
无效页码被忽略，`currentPageChanged(int)` 仅在当前页实际变化时发出。
`setMaxVisible()` 支持 1–25，不因总页数较少而拒绝更改；首尾页可额外显示，内部复用至多 27 个 PageButton。
大页数使用安全整数运算，页码按钮会适当加宽以免截断；重复点击当前页不取消高亮。
Gallery 示例设置了初始页和尺寸上限；中文 TS/QM 与资源同步更新。

```cpp
auto* filter = new qfw::ExclusiveLiteFilter(parent);
filter->addItems({QObject::tr("All"), QObject::tr("Active"), QObject::tr("Completed")});
filter->setFixedWidth(440);
QObject::connect(filter, &qfw::ExclusiveLiteFilterBase::currentTextChanged, parent,
                 [](const QString& text) { /* 筛选内容 */ });

auto* pager = new qfw::Pager(20, 5, parent);
pager->setCurrentPage(7);
QObject::connect(pager, &qfw::Pager::currentPageChanged, parent,
                 [](int page) { /* 加载对应页 */ });
```

头文件为 `exclusive_filter.h`、`pager.h`，通过 `qtfluentwidgets.h` 导出。
导航测试覆盖互斥/多选/去重、清空与所有权、重入信号、横向滚动/键盘/RTL、
页数边界/极大值、分页控件复用、鼠标和键盘激活、跳转校验以及主题/翻译。

## 第八批：扩展进度展示

新增 `FilledProgressBar`、`RadialGauge`、`MultiSegmentProgressRing`、`StepProgressBar`
和 `StepProgressBarButton`，接入 Gallery「状态和信息」，仍使用原生 Qt Widgets / QPainter。

竖向进度从底部向上填充，百分比在顶部，支持底部图标；仪表盘为底部留缺口的 270° 圆弧。
均复用原有 ProgressBar 的范围、动画、暂停/错误色和格式接口，进度按 `(value - minimum) / (maximum - minimum)` 计算。
同步修正 ProgressBar 的动画数值与原生 valueChanged 不同步、越界赋值更新绘制值以及极大范围的文本溢出问题。

多段环以非负有限权重归一化，支持颜色、段间角度和中央文本；零权重不占空间，单个正权重不留缝。
非法权重或颜色会抛出 `std::invalid_argument`，不会破坏已有段数据；极大权重也不会溢出。

步骤进度条默认 4 步、当前第 1 步，已到达的步骤显示主题色圆形按钮和完成图标，连线随进度着色。
支持点击/键盘、前后步骤、名称/图标、RTL 和非交互模式；重复点击当前步骤不会取消选中。
`setCount()` 支持 1–128，缩减时钳制当前步并发出实际变化信号，重建不残留旧控件，保留配置与非交互状态。
Gallery 使用 4 步并设置第二步为初值，限制演示宽度；翻译 TS/QM 和资源同步更新。

```cpp
auto* gauge = new qfw::RadialGauge(parent);
gauge->setValue(65);
auto* ring = new qfw::MultiSegmentProgressRing(parent);
ring->setSegments({{45, QColor("#0ea5e9")}, {30, QColor("#22c55e")}, {25, QColor("#f59e0b")}});
ring->setText(QObject::tr("Storage"));
auto* steps = new qfw::StepProgressBar(qfw::FluentIcon(qfw::FluentIconEnum::Accept), 4, parent);
steps->setStepNames({QObject::tr("Prepare"), QObject::tr("Configure"), QObject::tr("Install"), QObject::tr("Finish")});
steps->setCurrent(1);
```

通过 `extended_progress.h` 或 `qtfluentwidgets.h` 导入。进度测试覆盖数值/极值/动画、圆弧与填充方向、
多段归一化与校验、步骤互锁与通知、重建所有权、鼠标/键盘/禁用、悬停生命周期和亮暗主题/RTL。

## 第九批：颜色与快捷键选择

新增 `CircleColorPicker`、`DropDownColorPalette` 和 `ShortcutPicker`，接入 Gallery「基本输入」。
全部使用原生 QWidget / QPainter，不使用 WebView、系统全局快捷键监听或主题色修改。

圆形色板保留 32px 点击区域、22px 色块和 30px 选中圈，透明色显示棋盘格底色。
默认六个色块，首项选中；允许重复颜色，索引变化和颜色变化分别通知。
`setColors()` 先验证再原子替换，优先保留原索引/颜色；空列表清空选择，`setCurrentIndex(-1)` 取消选择。
支持鼠标、方向键/Home/End/Enter 和 RTL；不为色块添加鼠标提示。最多支持 4096 个色块。

下拉调色板保留 60 个主题色、10 个标准色、自动颜色和「其他颜色」入口。
`colorChanged` 只报告实际颜色变化，`colorSelected` 报告每次用户确认，包括确认相同颜色。
「自动」是显式配置的颜色（默认黑色），不是应用主题色；`automaticChanged` 单独报告模式变化。
弹层带阴影与淡入，支持键盘、Esc、点击外部关闭；小屏幕约束在可用屏幕内并允许滚动。
Windows 使用无边框透明弹窗，并禁用系统弹窗阴影，只保留圆角面板的自绘阴影；透明外沿不出现黑框或右侧/底部直线。
顶部「自动」和底部「其他颜色」为常态透明的自绘按钮，不绘制常态边框；只在悬停、按下或键盘焦点时添加轻微底色，图标与文本布局对齐 Python，并支持 RTL。
悬停底色覆盖整行，只保留面板外沿圆角。色块选中、悬停和键盘焦点均显示白色外框，填充向内收缩 2px，点击区域和网格位置保持不变；Python/C++ 使用相同几何。
调色板面板的描边在 340×432 内容尺寸内部绘制，不额外叠加 QSS 边框或布局边距；两侧色块留白均为 12px，与 Python 一致，透明阴影空间不计入内容留白。
「其他颜色」复用现有 `ColorDialog`，仅确认后提交；为它增加返回当前草稿的只读 `color()` 接口。
隐藏、禁用或销毁选择器时清理弹层/对话框，选择器不会接管调用者的业务逻辑。

快捷键选择器采用主题色键帽和现有 `CardWidget` / `MessageBoxBase`。
点击卡片或用 Enter/Space 打开遮罩编辑器，只捕获编辑器内的一个 Qt 组合键，不注册快捷键。
支持 Ctrl/Alt/Shift/Meta、数字键盘、修饰键单独输入、Shift+Tab 和可配置的 Escape 录入。
默认 Escape 取消；Save 提交，Reset 恢复默认草稿，Cancel 放弃草稿。多段键序列或非法组合抛出 `std::invalid_argument`。
`keySequenceChanged` 仅在实际值变化时发出，`keySequenceSelected` 和 `editingFinished` 在每次保存时发出，
取消则发出 `editingCanceled`。隐藏/禁用/销毁时结束捕获，不留下应用事件过滤器。
遮罩按父窗口客户区坐标定位并跟随窗口移动/缩放，避免将屏幕坐标当作子控件坐标。
取消按钮保留原有对话框样式；选择器独立使用时创建独立编辑窗口，不放大选择器本身。

```cpp
auto* colors = new qfw::CircleColorPicker({Qt::red, Qt::green, Qt::blue}, parent);
colors->setCurrentIndex(1);
auto* palette = new qfw::DropDownColorPalette(QColor("#0078D4"), parent);
palette->setAutomaticColor(Qt::black);
palette->setAlphaEnabled(true);
auto* shortcut = new qfw::ShortcutPicker(QStringLiteral("Ctrl+Shift+A"), parent);
QObject::connect(shortcut, &qfw::ShortcutPicker::keySequenceSelected, parent,
                 [](const QKeySequence& sequence) { /* 保存为 PortableText 或创建业务 QShortcut */ });
```

通过 `color_picker.h` / `shortcut_picker.h` 或 `qtfluentwidgets.h` 导入。Gallery 设置了初值和宽度限制，
英文内置文本使用 `tr()`，中文 TS/QM 与资源同步更新；测试覆盖选择规则、草稿提交、键盘、RTL、主题/翻译和所有权。

## 第十批：树形单选和多选下拉框

新增 `TreeComboBox` 和 `MultiSelectionTreeComboBox`，接入 Gallery「基本输入」。
默认使用自有 `QStandardItemModel`，也可绑定借用的 `QAbstractItemModel`；只展示第一列。
单选只提交叶子节点，按钮显示完整的 `父/子/叶子` 路径，`currentData()` 返回所选角色数据。
`currentModelIndexChanged` / `currentTextChanged` 仅在实际变化时通知，`activated` 在每次用户确认时通知。

多选以模型索引区分同名节点，保留选择顺序；父节点复选框批量选择/取消所有可用后代叶子，
勾选部分后代时显示半选。父节点本身不加入结果，禁用的分支/叶子不会由批量操作勾选。
`setSelectedIndexes()` 先校验再提交，忽略无效索引并去重，拒绝其他模型/列或非叶子节点。
默认显示可移除标签，空间不足时可滚动，`setChipsMode(false)` 显示逗号分隔文本。
`selectionChanged` 报告索引变化，`selectedTextChanged` 只报告显示名称实际变化。

树形弹层为原生 QWidget，带阴影和 160ms 淡入/滑动，屏幕边缘可向上打开，支持滚动与 RTL。
展开箭头与复选框独立操作，鼠标整行可提交叶子；键盘方向键导航，Space/Enter 提交或切换父节点，Esc 关闭。
数据改名、增删、移动、重置和外部模型销毁后同步选择/标签；隐藏、禁用、移动/调整尺寸时关闭弹层。
外部模型不转移所有权，多选使用代理模型保存勾选状态，不改写源模型的 `CheckStateRole`，避免污染共享视图。
与 Python 相同，仅通过索引/叶子名称处理结果；C++ 多选状态与源模型隔离。

```cpp
auto* single = new qfw::TreeComboBox(parent);
const auto group = single->addItem(QObject::tr("Tools"));
const auto leaf = single->addItem(QObject::tr("Editor"), group, QStringLiteral("editor"));
single->setCurrentModelIndex(leaf);
single->setFixedWidth(360);

auto* multi = new qfw::MultiSelectionTreeComboBox(parent);
multi->setModel(single->model()); // Borrowed; the model must outlive its use.
multi->setSelectedIndexes({leaf});
multi->setFixedWidth(420);
QObject::connect(multi, &qfw::MultiSelectionTreeComboBox::selectionChanged, parent,
                 [](const QList<QModelIndex>& indexes) { /* Process leaf indexes */ });
```

通过 `tree_combo_box.h` 或 `qtfluentwidgets.h` 导入。Gallery 设置初值、宽度限制和对应源码链接；
英文内置文本使用 `tr()`，中文 TS/QM 与资源同步更新。独立测试覆盖选择校验、模型生命周期、
重入信号、父节点半选、同名叶子、共享模型隔离、鼠标/键盘/RTL、标签和主题/翻译。

## 第十一批：普通多选下拉框与字体选择器

新增 `MultiSelectionComboBox` 和 `FontComboBox`，接入 Gallery「基本输入」，
使用原生 Qt Widgets，不依赖 Python。通过 `extended_combo_box.h` 或 `qtfluentwidgets.h` 导入。

多选默认使用逗号分隔文本，`setChipsMode(true)` 显示可移除标签并支持横向滚动。
列表沿用 Python 的 36px 行高、14px 字体和 18px 复选框；点击选项不会关闭弹层。
选项以索引区分，同名文本也可以独立选择；结果按选项插入顺序输出。
插入/移除后重映射索引，清空恢复占位文本；仅实际变化时通知，支持信号重入与回调销毁控件。
程序赋值可保留禁用选项的选择，禁用项不能由鼠标或键盘切换。
标签使用稳定的选项身份，避免排队中的关闭信号在索引移动后删除错误选项。
图标保存 QIcon 值，不借用临时 FluentIcon 指针。数据接口遵循已有 ComboBox 的 `QVariant` 风格。

字体选择器从 QFontDatabase 读取字体，列表采用字体自身的 14px 预览和 33px 行高。
支持可缩放/不可缩放、等宽/比例字体过滤及 writingSystem；组合过滤取交集，冲突组合得到空列表。
过滤变化尽量保留仍可用的当前字体，否则选择首项；未知字体赋值被忽略。
`currentFontChanged(QFont)` 仅在选中的字体族实际变化时通知，不因重复选择触发。
支持单选指示条、键盘确认、Esc、RTL，以及亮/暗主题；隐藏、禁用或改变尺寸/位置时关闭弹层。
同时修复旧 ComboBox 的菜单信号连接生命周期，旧菜单延迟销毁不会清空新打开的菜单。

```cpp
auto* languages = new qfw::MultiSelectionComboBox(parent);
languages->addItems({"Python", "C++", "Rust", "JavaScript"});
languages->setSelectedIndices({0, 2});
languages->setChipsMode(true);
languages->setFixedWidth(320);

auto* font = new qfw::FontComboBox(parent);
font->setFontFilters(qfw::FontComboBox::MonospacedFonts);
font->setCurrentFont(QFont("Consolas"));
```

Gallery 提供纯文本/标签切换、清空选择、等宽筛选与实际字体预览，中文翻译同步更新。
独立测试覆盖同名选项、插入/移除/清空、选择数据、通知重入、标签身份、弹层生命周期，
以及字体筛选/语言系统、鼠标/键盘、RTL 和亮/暗主题绘制。

## 第十二批：分隔器与瀑布流布局

新增 `Splitter` / `SplitterHandle` 与 `WaterfallLayout`，接入 Gallery「布局」。
分隔器使用 Python 的 14px 手柄、亮暗主题拖动标记及悬停背景，保留 Qt 原生拖动、
伸缩比例和状态保存/恢复；水平与垂直方向均可用。

瀑布流使用最短列优先放置，默认三列，可设置固定列宽或自动列数。
支持独立横纵间距、内容边距、RTL、隐藏项紧凑重排和可配置的几何动画。
与 Python 保持布局效果，但 `takeAt()` 遵守原生 QLayoutItem 所有权约定，
高度计算不计末尾行距和坐标原点；`removeAllWidgets()` 保留控件，
`takeAllWidgets()` 通过 `deleteLater()` 删除控件。
动画中移除/隐藏控件会清理或重置对应动画，不改变被移除控件的父对象。

```cpp
auto* splitter = new qfw::Splitter(Qt::Horizontal, parent);
splitter->addWidget(leftPanel);
splitter->addWidget(rightPanel);

auto* layout = new qfw::WaterfallLayout(container, true, true);
layout->setColumnWidth(160);
layout->setHorizontalSpacing(12);
layout->setVerticalSpacing(12);
layout->setAnimation(220, QEasingCurve::OutCubic);
layout->addWidget(card);
```

独立测试覆盖最短列、固定/自动列宽、边距与换行高度、RTL、隐藏重排、
动画中移除、控件所有权、非控件布局项及极值参数，以及分隔条亮暗绘制和拖动。
同时修复字体与普通多选弹层误开启 Qt 原生滚动条的问题，统一使用 Fluent 滚动条，
补充亮暗主题、RTL、滚动范围和原生滚动位置双向同步测试。

## 第十三批：分组时间线与 Toast 通知

新增 `TimeLineWidget`、`TimeLineGroup`、`TimeLineItem` 与 `Toast`，接入 Gallery「状态和信息」。
通过 `time_line.h`、`toast.h` 或 `qtfluentwidgets.h` 导入。

时间线沿用 Python 的默认 370px 宽度、328px 卡片、36px 图标格中的 16px 状态图标、
50px 最小卡片高度、19px 卡片间距和分组连接线。正文支持富文本与换行，
整卡支持鼠标和空格/回车激活；右键、取消点击、隐藏或禁用后释放不触发。
RTL 镜像缩进、连接线和图标位置。移除项/分组使用 `deleteLater()`，外部删除也会更新清单。

Toast 提供 `info()`、`success()`、`warning()`、`error()`、`custom()` 和 `newToast()` 工厂。
必须提供父窗口，不创建独立顶层窗口；默认竖排文字，可选择横排和添加自定义控件。
沿用 Python 的 8px 圆角、4px 顶部状态条、200ms OutQuad 入场动画、24px 堆叠间距和六种停靠位置。
信息通知实时跟随主题色，其余状态使用 Python 固定颜色；亮暗背景分别为 `#F2F2F2` / `#323232`。
文字使用原生 QLabel 换行，并限制在父窗口内，缩放/改变内容时同步重排。
各窗口独立管理通知；关闭会重排剩余通知、发出一次 `closed()` 并延迟销毁。
负时长保持显示，零时长排队关闭；显式隐藏暂停计时，重新显示重新计时，重复显示不会重复入栈。

```cpp
auto* timeline = new qfw::TimeLineWidget(parent);
auto* group = timeline->addGroup(QObject::tr("Today"));
auto* task = group->addItem(QObject::tr("Build gallery"));
QObject::connect(task, &qfw::CardWidget::clicked, parent, [parent] {
    qfw::Toast::success(QObject::tr("Completed"), QObject::tr("Task finished."),
        2000, true, qfw::ToastPosition::TopRight, Qt::Vertical, parent);
});
```

专项测试覆盖亮暗主题/RTL 像素与几何、富文本/换行、键鼠点击和回调销毁、
六方向横竖排堆叠、自动/手动关闭、隐藏重开、外部删除、跨窗口隔离、重新绑定父窗口、
父窗口缩放、动态内容、自定义按钮、状态配色及中文翻译。

## 第十四批：任务进度通知与胶囊加载通知

新增 `ProgressToast` 和 `RoundProgressToast`，接入 Gallery「状态和信息」。
通过 `progress_toast.h` / `round_progress_toast.h` 或 `qtfluentwidgets.h` 导入。
两者复用 Toast 的六方向堆叠、24px 间距、入场动画、父窗口所有权和关闭通知。
与普通 Toast 混合堆叠，内容变化或窗口缩放后同步重排，不另建顶层窗口。

进度通知沿用 Python 的 280×50 最小展示尺寸、16px 状态图标、
`12,10,12,10` 内边距、8px 圆角、描边和底部 3px 进度条，不绘制顶部状态条。
进度钳制在 0–100，默认 150ms 平滑动画；隐藏或禁用时停止刷新并同步到当前值。
进度、文字、状态和生命周期互不影响：到 100 不自动成功或关闭，关闭也不取消业务任务。
信息状态跟随主题色，其他状态保留 Python 的亮/暗配色；
`setStateColor()` 优先于 `setCustomBarColor()`，可分别配置亮暗颜色。
状态图标只重着色外形，保留对比色符号；可添加自定义按钮或其他控件。

胶囊通知沿用 Python 的 180×48 最小展示尺寸、20px 加载环、3px 环宽，
`20,14,30,14` 内边距，以及 16px 模糊、3px 偏移的轻量阴影。
默认不显示标题、关闭按钮或进度条；无参内容翻译为「正在加载，请稍候」，
显式空字符串仍保留为空。隐藏时暂停环动画，再显示从原相位恢复；
禁用时停止，启用后重新开始。默认持续显示，可由 `close()` 或非负时长关闭。
继承的 success/warning/error/custom 工厂仍返回普通 Toast，不将终态伪装成加载通知。

最小尺寸是正常展示偏好：小窗口内以可用宽度为上限，正文自动换行。
两者沿用真实 QLabel / Windows 字体的测量，不丢失此前中文/DPI 修复；
RTL 镜像文本、前置图标和不对称内边距。进度条与 Python 一样从左向右填充。
中英文 TS/QM 同步更新，Gallery 演示进度/状态独立修改与手动结束加载。

```cpp
auto* task = qfw::ProgressToast::info(QObject::tr("Preparing release"), 35, -1,
    true, qfw::ToastPosition::TopRight, parent);
task->setValue(80);
task->setIcon(qfw::InfoBarIconEnum::Success); // Explicit business decision.
task->setStateColor(qfw::InfoBarIconEnum::Success, QColor("#0f7b0f"), QColor("#6ccb5f"));

auto* loading = new qfw::RoundProgressToast(parent);
loading->show();
// Call loading->close() when your task ends.
```

测试覆盖亮暗主题/RTL 几何与像素、状态颜色优先级和图标对比色、进度动画和钳制、
回调重入/销毁、隐藏/禁用/关闭、自动定时关闭、六方向混合堆叠、正文变长/缩回、
空闲布局稳定性、动态 RTL、中文翻译，以及五档 Windows 原生字体/DPI 下的正文测量。

## 第十五批：带任务进度环的信息条

新增 `ProgressInfoBar`，通过 `progress_info_bar.h` 或 `qtfluentwidgets.h` 导入，
接入 Gallery「状态和信息」页。复用免费版 InfoBar 的中性背景、六方向堆叠与关闭按钮，
默认持续显示且处于不确定进度模式。环占 36×36 图标区，实际环为 20×20、3px 环宽；
文字沿用 14px 字体和 Python 的 8px 顶部文字内边距。

`setValue()` 钳制 0–100 并切换到确定进度，环立即更新，无进度插值动画；
`setIndeterminate(true)` 切回加载环，不丢失已记录的百分比。
隐藏时暂停加载动画、再显示恢复相位，禁用或关闭时停止动画。
达到 100% 不自动成功或关闭；关闭 UI 不会取消业务任务。
`progressRing` / `determinateRing` 可访问两种环，`setCustomBarColor(light, dark)` 同步设置。
默认环色跟随当前亮/暗主题的主题色。

`setTitle()` / `setContent()` 可更新文本；`setRemainingTime()` 接受调用方已格式化的字符串，
使用「 · 」附加在正文后，不计算 ETA。空文本隐藏对应标签，内容使用纯文本，
长内容按照真实 QLabel 字体宽度换行；缩回短文字时尺寸同步恢复。
横排/竖排和 RTL 均保留 InfoBar 布局，自定义控件仍可通过 `addWidget()` 加入。
文字变化立即重新定位同一 InfoBar 堆叠，不与 Toast 堆叠混用。
隐藏、关闭、外部删除和父窗口销毁时清理堆叠及位置动画，避免残留项。

`create()` / `info()` 返回任务信息条；继承的 success/warning/error 返回普通 InfoBar，
与 Python 保持相同的终态工厂语义。

```cpp
auto* task = qfw::ProgressInfoBar::info(QObject::tr("Download"),
    QObject::tr("Preparing download..."), Qt::Vertical, true, -1,
    qfw::InfoBarPosition::TopRight, parent);
task->setRemainingTime(QObject::tr("Remaining: 10 s"));
task->setValue(65);
// Connect closedSignal to your cancellation handler only if that is intended.
```

测试覆盖亮暗主题/RTL、横竖布局、中文原生字体和五档 DPI、进度钳制与回调销毁、
模式切换/隐藏恢复/禁用/重复关闭、自动关闭、六方向混合堆叠、ETA 和正文变长/缩回、
父窗口销毁与外部删除，以及 Gallery 中文翻译。

## 第十六批：紧凑等待对话框

新增 `WaitingDialog`，通过 `waiting_dialog.h` 或 `qtfluentwidgets.h` 导入，
接入 Gallery「对话框和弹出窗口」页。构造时必须提供父窗口，嵌套控件作为父参数时，
自动绑定它的顶层窗口；无父窗口会抛出 `std::invalid_argument`。

沿用 Python 常规 300×132 逻辑像素面板、56px 加载环与 6px 环宽，
`39,32,29,32` 内边距、30px 图文间距、20px 标题和 14px 正文、10px 双行间距，
以及 60px 模糊/向下 10px/透明度 50 的阴影。纯文本标签支持动态更新、换行和空文本隐藏。
默认标题「请稍候…」可翻译；显式空字符串仍为空。RTL 镜像前后位置和不对称内边距。

遮罩覆盖父窗口的客户区，使用黑色透明度 76，不额外画不透明背景；
随窗口移动、缩放同步更新。小窗口内面板宽高受可用区域限制，
宽度小于 280px 时使用 40px 环、16px 横向内边距和间距。
长正文按真实 QLabel 的 `heightForWidth()` 测量，变回短文本时恢复紧凑高度。

使用 `open()` 非阻塞显示，`accept()` 表示完成，Esc / `reject()` 表示取消。
默认点击遮罩不取消，可通过继承的 `setClosableOnMaskClicked(true)` 启用。
父窗口隐藏或关闭时取消等待；直接 `hide()` 仅隐藏，不发出拒绝信号。
加载环只在显示期间运行，开始关闭时立即停止。
使用单一受控淡入/淡出动画，立即 Esc、重复关闭时以第一次结果为准，不残留遮罩。
继承普通 QDialog 生命周期，不自动销毁；Gallery 在 `finished()` 后 `deleteLater()`。

这是业务任务的 UI，不自动中止后台工作；需要取消任务时，显式连接 `rejected()`。
`progressRing`、`titleLabel`、`contentLabel` 和 `panel()` 可访问展示控件。

```cpp
auto* dialog = new qfw::WaitingDialog(parent);
dialog->setContent(QObject::tr("Preparing download..."));
QObject::connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater);
dialog->open();
// Call dialog->accept() when the task completes, or connect rejected() to cancellation.
```

回归覆盖亮暗主题/RTL、面板和文字间距、遮罩透明度与阴影、模式/焦点、
立即 Esc/重复关闭/重开、隐藏/父窗口销毁/信号回调删除、移动缩放和极小窗口、
长内容缩回、中文翻译，以及五档 Windows 字体/DPI 的中文测量。

## 第十七批：图片对比与局部放大镜

新增 `ImageComparisonSlider`、`ImageMagnifierWidget`，接入 Gallery「视图」页顶部。
示例使用已打包的本地 `SBR.jpg`，分别展示灰度/彩色前后对比，以及可配置倍数、
半径与开关的放大镜；不下载远程图片、不读取用户桌面或截屏。

`ImageComparisonSlider` 用百分比控制分隔线，默认 50，范围 0–100；
左侧为 before，右侧为 after，RTL 下不交换图片或键盘方向。
两张原图绘制到同一完整显示区域后分别裁切，不因拖动重新缩放各自半边。
支持路径、QImage、QPixmap 和单独替换/清空一侧；空侧透出父背景。
分隔线和 24×52px 双箭头握柄保持 Python 外观。
点击或拖动调整，左右键每次 1%，Home/End 到两端；隐藏/禁用结束拖动。
`valueChanged(qreal)` 只在实际值变化时发出；非有限值抛出 `std::invalid_argument`。
`sizeHint()` 优先使用 before 原图尺寸，其次 after，均为空时为 400×300。
提供 `scaledToWidth()`、`scaledToHeight()` 和 `setScaledSize()`，尺寸必须为正且不超 Qt 控件限制。

`ImageMagnifierWidget` 继承 ImageLabel，保留图片、缩放和圆角接口。
默认 2 倍、半径 50px；`setMagnification()` 要求有限且不小于 1，
`setRadius()` 最小为 8px，`setMagnifierEnabled()` 可独立开关镜片。
悬停时镜片跟随鼠标，键盘聚焦时居中，方向键以 5px 步长移动且不越界。
镜片从原始 QImage 直接采样，不放大已缩小的预览缓存；边框与十字线使用当前主题主色。
移出且失焦、隐藏、禁用或空图时不显示镜片，没有后台刷新计时器。

```cpp
auto* comparison = new qfw::ImageComparisonSlider(beforeImage, afterImage, parent);
comparison->scaledToWidth(420);
comparison->setValue(50);

auto* magnifier = new qfw::ImageMagnifierWidget(sourceImage, parent);
magnifier->scaledToWidth(420);
magnifier->setMagnification(2);
magnifier->setRadius(50);
```

头文件为 `image_comparison.h`、`image_magnifier.h`，通过 `qtfluentwidgets.h` 导出。
回归覆盖范围/信号/无效参数、图片值语义与加载、尺寸与等比缩放、拖动/隐藏/禁用、
物理方向键/RTL、亮暗主题/1×及2×像素对齐、原图细节采样、镜片开关/边缘及空图，
以及 Gallery 中文翻译。

## 第十八批：图片裁剪与头像选择

新增 `ImageCropper` 和 `AvatarPicker`，接入 Gallery「视图」页顶部。
示例使用已打包的本地图片，支持矩形、圆形、圆角矩形和菱形裁剪，
结果只更新预览/头像，不写入文件、不上传内容。

`ImageCropper` 有父参数时绑定顶层窗口，作为同窗口内遮罩层显示，
不另开透明窗口改变 Mica 宿主的激活状态；无父参数时可作为独立模态对话框。
默认选择原图中央 80% 区域；选区和输出均使用原图像素坐标。
支持移动、四角缩放、顺时针 90° 旋转和水平翻转；旋转/翻转后重置选区。
圆形裁剪使用正方形选区，角点调整时固定对角。
四角手柄半径 8px，最小选区边长按显示尺寸 16px 换算，拖动不会越过原图边界。
黑色遮罩透明度 170；透明原图只在图像范围内铺中性灰，避免透出 Gallery 内容。
工具栏沿用 262×48px 深灰圆角背景、46×40px 五个工具按钮、取消/确认两处分隔线。
自定义形状按钮默认禁用，形状由业务代码配置；Gallery 通过打开前的下拉框选择。

`setCropShape(CropShape::RECTANGLE/CIRCLE/CUSTOM)`、
`setCropPathFactory(std::function<QPainterPath(const QRectF&)>)` 可配置裁剪形状。
路径工厂接受给定矩形并返回其坐标范围内的路径，应为不抛异常、无副作用的纯绘制函数。
`selectionRect()`、`setSelectionRect()`、`sourceImage()`、`imageRect()`、
`displaySelection()`、`canvasWidget()` 和工具按钮可访问/配置。
无效形状、空路径工厂或非有限/不与原图重叠的选区抛出 `std::invalid_argument`。
程序设置的选区会裁到原图边界；非矩形输出保留透明区域。
输出 QImage 的 DPR 统一为 1，避免高 DPI 原图按逻辑尺寸再次缩小。

`imageCropped(QImage)` 只在有效确认时发出，随后按 QDialog 生命周期关闭；
取消/空图确认不发结果，重复确认不重复提交，允许结果回调删除对话框。
Esc 可取消，Tab 在工具栏内循环；遮罩拦截宿主快捷键和背景键盘输入。
随宿主移动/缩放同步，宿主隐藏/关闭时取消。建议非阻塞 `show()`，
按需连接 `finished()` 到 `deleteLater()`，普通 ImageCropper 不自动销毁。

`AvatarPicker` 继承 AvatarWidget，保留图片、半径和圆形外观。
左键在控件内释放，或按空格/回车，打开异步本地文件选择；右键、拖出释放、
自动重复按键不激活。悬停覆盖黑色透明度 155 和白色相机图标。
`cropImage()` 可直接传路径、QImage 或 QPixmap，复用圆形 ImageCropper。
只有确认后才更新头像并发出 `imageChanged(QImage)`；取消保留旧头像和半径。
隐藏/禁用/销毁选择器会收起未完成的交互，重复浏览不创建多个文件窗口。
文件对话框默认使用系统原生窗口，与 Python 保持一致，异步打开且不启用保存操作；`browseDialog()`、`cropEditor()`
可访问当前交互，`cancelSelection()` 可取消。

裁剪工具栏的旋转按钮与 Python 一致，使用 `FluentIcon::Rotate` 的白色素材，
居中绘制为 14×14，禁用时透明度为 80/255。

```cpp
auto* editor = new qfw::ImageCropper(image, parent);
editor->setCropShape(qfw::CropShape::CIRCLE);
QObject::connect(editor, &qfw::ImageCropper::imageCropped, receiver,
                 [](const QImage& cropped) { /* 更新业务内存中的图片 */ });
QObject::connect(editor, &QDialog::finished, editor, &QObject::deleteLater);
editor->show();

auto* avatar = new qfw::AvatarPicker(image, parent);
avatar->setRadius(48);
```

头文件为 `image_cropper.h`、`avatar_picker.h`，通过 `qtfluentwidgets.h` 导出。
回归覆盖裁剪像素/透明度/自定义路径/DPR、旋转翻转、全部四角/固定对角、移动边界与拖动取消、
宿主遮罩几何/键盘拦截、确认取消/重开/重复提交/回调删除/父对象销毁，
头像文件选择/鼠标键盘/确认取消/隐藏禁用/回调销毁、亮暗主题/RTL/1×及2×渲染与中文翻译。

## 第十九批：轻量音频波形

新增 `AudioWaveformWidget`，接入 Gallery「视图」页顶部。只显示归一化单声道
采样，不解码文件、不播放音频，也不引入 Qt Multimedia 或网络依赖。
Gallery 提供合成采样、追加两秒、进度滑块、模拟进度、允许定位、重置与清空。
切走示例页会停止模拟计时，不在后台持续推进。

- `setSamples(QVector<float>, sampleRate=24000)` 替换数据并复位进度；
  `appendSamples(samples[, sampleRate])` 追加数据，保留进度。
  非空流不允许变更采样率，空追加无操作；采样率须为正整数，
  非有限采样报错，有限采样钳制到 [-1,1]，校验失败不改变现有数据。
  `samples()` 返回独立值副本，数据保存在紧凑 float 向量中。
- `sampleCount()`、`sampleRate()`、`duration()`、`position()` 查询状态；
  时长向上取整为毫秒，位置钳制到 [0,duration]。C++ 时长和位置使用 qint64。
  已完成的 128 帧分块缓存极值，追加只补齐新增块，重建显示条不会反复扫描整段录音。
- `setBarWidth()`、`setBarSpacing()`、`setAmplitudeScale()` 与 Python 同名接口一致，
  默认 1.5px / 2px / 1 倍。分箱保留正负极值和零基线，静音绘制圆帽点；
  默认尺寸提示 480×160，最小尺寸提示 32×32，背景透明。
- `setWaveformColor(light[,dark])`、`setPlayedColor(light[,dark])` 设置颜色，
  省略 dark 时两种主题共用 light。默认未播放颜色 #D6D6D6 / #686868，
  已播放颜色 #606060 / #F2F2F2，禁用透明度 0.43。
  播放区与未播放区使用不相交裁剪，避免半透明圆帽被重复叠加。
- 左键点击/拖动请求定位，方向键每次移动一秒，Home/End 到起点/终点，
  重复点击当前位置仍发 `seekRequested`；程序调用 `setPosition()` 不发定位请求。
  保留 Python 的物理时间方向（RTL 不反转时间），小数鼠标坐标及半整数取偶舍入。
  `setSeekEnabled(false)` 禁止用户定位但仍可更新进度；隐藏、禁用、替换采样或
  关闭定位会取消当前拖动。所有修改须在 GUI 线程执行，生产者线程应使用队列连接。
- `samplesChanged()`、`durationChanged(qint64)`、`positionChanged(qint64)`、
  `seekRequested(qint64)` 提供状态通知，允许结果回调删除控件。

```cpp
auto* waveform = new qfw::AudioWaveformWidget(parent);
waveform->setSamples(normalizedMonoSamples, 24000);
waveform->appendSamples(nextChunk); // 保留当前进度
waveform->setPosition(1500);
QObject::connect(waveform, &qfw::AudioWaveformWidget::seekRequested,
                 player, [player](qint64 ms) { /* 外部播放器定位 */ });
```

头文件 `audio_waveform.h`，通过 `qtfluentwidgets.h` 导出。
回归覆盖采样复制/校验/追加/时长/信号顺序、缓存块边界、点击/拖动/键盘/取消、
回调销毁、亮暗/RTL/1×及2×、半透明裁剪、缩放和正负极值的独立 Python 公式像素对照，
以及库和 Gallery 的中文翻译。

## 第二十批：紧凑确认浮层

新增 `FlyoutDialog : FlyoutViewBase`，接入 Gallery「对话框」页顶部，提供普通确认和
带输入框的两种示例。示例只回显操作结果，不写入文件。

`titleLabel`、`contentLabel`、`contentWidget`、`viewLayout`、`vBoxLayout` 和按钮底栏均
可访问；`addWidget(widget, stretch, align)` 将自定义内容加入正文。空标题/正文隐藏。
外边距 1px，正文边距 15px、间距 8px，合计正文从边缘 16px 开始；底栏固定 40px，
内边距 4px，确认和取消图标为 16×16，两按钮均分可用宽度。普通状态透明。
最小宽度 256px，正文自然宽度偏好最多 480px（自定义控件/标题可要求更宽）；
根据实际宽度重新计算换行高度。亮色边框沿用基础 Flyout，暗色为白色 alpha 20，
底栏分隔线为黑色 alpha 17 / 白色 alpha 15。

`showAt(QWidget* / QPoint, parent, animation)` 支持已有 Flyout 动画，默认 PullUp。
返回弹出容器并转移视图所有权，关闭后容器和视图一起延迟销毁；每次交互请新建
堆上的 FlyoutDialog，不要将栈对象交给自动销毁的弹层。重复调用复用尚未销毁的容器，
不创建多个弹层。`flyout()` 查询当前容器；也兼容 `Flyout::make()` 的结果关闭连接，
但要获得阴影区点击/隐藏窗口的完整取消行为，优先使用 `showAt()`。

`accept()`、`reject()`、按钮和 Enter/Esc 完成操作；`accepted()` 或 `rejected()` 后发
`closed()`，同一交互只完成一次。外部点击（包含阴影区）、原生弹窗隐藏、宿主隐藏/关闭、
锚点销毁均取消。结果回调可销毁视图或容器，删除视图会收起残留弹层。
自定义多行输入等控件保留其原生按键行为；Esc 也可以从正文子控件取消。

```cpp
auto* dialog = new qfw::FlyoutDialog(tr("Apply changes?"), tr("Confirm to continue."));
auto* name = new qfw::LineEdit;
dialog->addWidget(name, 0, Qt::Alignment());
QObject::connect(dialog, &qfw::FlyoutDialog::accepted, receiver,
                 [name] { /* 读取 name->text() */ });
dialog->showAt(button, window);
```

头文件 `flyout_dialog.h`，通过 `qtfluentwidgets.h` 导出。回归覆盖正文/底栏几何、
长短正文重测量、中文字体/亮暗/RTL/1×及2×、按钮/键盘/输入框回车、外部与阴影点击、
宿主/锚点/隐藏取消、单次结果、回调销毁、空目标和基础 Flyout 集成。
Windows 隐藏窗口另验证 100% / 125% / 150% / 175% / 200% 的中文字体布局。

## 第二十一批：圆角列表

新增三种外观的控件版与模型版，并加入 Gallery「视图」页顶部：

- `RoundListWidget` / `RoundListView`：圆角卡片、悬停背景、主题色选中描边。
- `TransparentRoundListWidget` / `TransparentRoundListView`：常态透明，选中保持透明。
- `CategoryCardListWidget` / `CategoryCardListView`：选中保留常态卡片底色。
- `RoundListItemDelegate`：可配置 `transparentSelection`、`keepNormalBackground`。

按 Python 实例的实际绘制结果对齐：普通/分类款行高 48px，透明款 46px，
行间 spacing 为 1，圆角 5px，图标 16px；背景保留 Python 委托与样式表的两层叠加。
没有普通列表的侧边选中条，选中描边与 FluentIcon 使用经过暗色主题调整的主色。
模型中的普通 QIcon 保留原色，FluentIcon 使用拥有图标副本的 `themedIcon`。
支持 Qt 原有字体/前景/背景/尺寸提示角色、编辑、复选框、键盘、多选及平滑滚动条。
选中绘制读取实时视图状态，不依赖缓存行号，排序/删行/换模型后不会留下错误的描边。
右键默认不改变选择，可通过 `setSelectRightClickedRow(true)` 开启。

```cpp
auto* list = new qfw::RoundListWidget(parent);
list->addItem(tr("Documents"), qfw::FluentIconEnum::Document);
list->addItem(tr("Pictures"), qfw::FluentIconEnum::Photo);
list->setCurrentRow(0);

auto* model = new QStandardItemModel(parent);
model->appendRow(new QStandardItem(
    qfw::RoundListItemDelegate::themedIcon(qfw::FluentIcon(qfw::FluentIconEnum::Home)),
    tr("Home")));
auto* view = new qfw::TransparentRoundListView(parent);
view->setModel(model);
```

通过 `qtfluentwidgets.h` 或 `components/widgets/round_list_view.h` 导入。
回归覆盖六类、亮暗主题、RTL、1×/2×绘制、图标所有权与原色、程序选择与清空、
右键、键盘、多选、禁用项、动态变体、模型排序/替换/清空/移除和显式行高。

## 第二十二批：圆角与线条表格

新增 `RoundTableWidget` / `RoundTableView` 和 `LineTableWidget` / `LineTableView`，
通过 `qtfluentwidgets.h` 或 `components/widgets/extended_table_view.h` 导入。
Gallery「视图」页顶部各有一张示例卡，可在控件表格/模型表格间切换；
支持 Ctrl / Shift 多选，线条款另可启用表头排序。

圆角款隐藏两个表头，不画外围边框；行高 48px，行内上下间距 2px，圆角 8px，
交替行和常态选中行采用 Python 的浅卡片底色与细描边（亮色白色 alpha 127、暗色白色 alpha 8）。
线条款隐藏纵表头，横表头 52px / 14px 粗体，行高 48px，无交替底色，
主题色选中底色 alpha 30，悬停 40，按下 60，行末 1px 分隔线。
两款默认不画普通表格的左侧选中指示条。

`TableItemDelegate` 补齐 Python 配置接口：`setShowIndicator`、`setBorderRadius`、
`setUseStackedWidgetStyle`、`setUseTransparentRows`、`setUseThemeColorSelection`、
`setShowRowDivider`；C++ 另提供 `setRowMargin`。原有普通表格的默认配置保持不变。
新增表格的选中背景读取实际模型选择，不依赖缓存行号；模型排序、删除、替换后不会残留选中背景。
新增表格的 `selectRow()` 按模型选择行，修正 RTL 视图左侧留白时原生 Qt 漏选的问题。
圆角行按可见列的物理边缘绘制，隐藏/移动列、单列、RTL 和横向滚动时保持连贯；
分隔线在单元格裁切内逐段绘制，不向表格外延伸。

```cpp
auto* table = new qfw::LineTableWidget(parent);
table->setColumnCount(3);
table->setRowCount(1);
table->setHorizontalHeaderLabels({tr("Task"), tr("Status"), tr("Progress")});
table->setItem(0, 0, new QTableWidgetItem(tr("Compile Gallery")));
table->setItem(0, 1, new QTableWidgetItem(tr("Completed")));
table->setItem(0, 2, new QTableWidgetItem("100%"));
table->selectRow(0);
```

保留 Qt 模型、编辑、复选框、字体/颜色角色、右键选择开关以及平滑滚动。
回归覆盖四类、亮暗/RTL/1×及2×、实测背景透明度、悬停/按下、跨列分隔线、
单列圆角、移动/隐藏列、横向滚动、程序选择/多选、模型生命周期、复选框与编辑提交/取消。

## 第二十三批：圆角标签栏

- `RoundTabBar`：浏览器式圆角标签栏，沿用路由、添加/关闭请求、拖动排序和横向滚动接口。
- `RoundTabItem`：32px 高、8px 圆角，选中轮廓带 5px 外展底角且不画底部描边。

按 Python 实测采用 38px 标签栏、顶部 6px / 左侧 16px 边距、相邻标签 5px 重叠，
默认最大宽度 200px、关闭阴影；隐藏标签不占布局空间，RTL 布局镜像。
悬停底色 alpha 10，按下 alpha 12；选中背景支持独立亮暗配色。
相邻选中/悬停标签之间隐藏分隔线，滚动时按视口坐标绘制分隔线。
只有按住鼠标左键才进入拖动逻辑，避免开启 movable 后普通悬停触发排序。

Gallery「导航」页新增圆角标签栏，可拖动、滚动、添加和关闭标签。
添加与关闭按钮只发出请求，不自动删除应用页面；本批未提供 `RoundTabWidget` 页面容器。
回归覆盖亮暗/RTL 布局、隐藏标签、宽度变更、添加/关闭请求、销毁与重建、
悬停/拖动及 1×/2× 选中/悬停/按下像素。

```cpp
auto* tabs = new qfw::RoundTabBar(parent);
tabs->setMovable(true);
tabs->setScrollable(true);
tabs->addTab("home", tr("Home"), qfw::FluentIconEnum::Home);
tabs->addTab("settings", tr("Settings"), qfw::FluentIconEnum::Settings);
connect(tabs, &qfw::TabBar::tabCloseRequested, tabs, &qfw::TabBar::removeTab);
```

## 第二十四批：圆角标签页面容器

`RoundTabWidget` 继承 `TabWidget`，结合圆角标签栏和应用提供的 QWidget 页面。
容器统一绘制选中标签与页面的填充和描边：默认页面圆角 8px、顶部标签栏 38px、
页面从 y=38px 开始，中间布局间距为 0；选中标签与页面使用同一轮廓，衔接处无横线。
页面直接读取标签栏的亮/暗背景配置；通过容器或标签栏修改配色都会刷新完整轮廓。
标签滚动、拖动动画、显示/隐藏及主题切换时更新轮廓，标签部分裁切到实际视口。

支持 `addTab(page, label)`、`addTab(page, icon, label)` 和原有
`addTab(page, label, icon, routeKey)`；`insertTab` 也提供对应形式，`addPage` 保留添加页面别名。
无图标的标签不会默认显示箭头。重复页面/空页面返回 -1，重复显式路由抛出
`std::invalid_argument`，不改变页面和标签的对应关系。

- `indexOf`、`stackedWidget`、`tabBarAutoHide` / `setTabBarAutoHide`、
  `tabWhatsThis` / `setTabWhatsThis` 补齐页面及标签配置接口。
- `Ctrl+Tab` / `Ctrl+Shift+Tab` 循环切换可见且启用的页面；禁用/隐藏当前标签时优先
  选择后方可用页面，再选择前方页面。所有标签隐藏时保留当前页面，与 Python 一致。
- `currentChanged` 按实际页面及索引变化通知；插入到当前页前面但页面没变时不额外通知。
  拖动时保留当前页面身份；跨隐藏标签重排也保持路由、标签、页面一致。
- 关闭标签只发出请求。`removeTab` / `clear` 隐藏移除的页面但不删除它；页面可重新加入，
  输入状态保留。应用负责决定是否 `deleteLater()`，保留页面仍归原容器管理。
- Gallery「导航」页新增圆角标签页面：可输入内容、切换/重排、添加/关闭、自动隐藏标签栏。
  关闭示例页面时由 Gallery 显式删除；标签栏隐藏后仍可通过外部“添加页面”按钮恢复。

```cpp
auto* tabs = new qfw::RoundTabWidget(parent);
tabs->setMovable(true);
tabs->setScrollable(true);
tabs->setTabBarAutoHide(true);
tabs->addTab(new QWidget, tr("Home"));
tabs->addTab(new QWidget, qfw::FluentIconEnum::Settings, tr("Settings"));
connect(tabs, &qfw::TabWidget::tabCloseRequested, tabs, [tabs](int index) {
    auto* page = tabs->widget(index);
    tabs->removeTab(index);
    if (page) page->deleteLater(); // Only when the application intends to discard the page.
});
```

回归覆盖页面所有权、通知次数、输入状态、空/重复参数、元数据、基类接口调用、
隐藏/禁用/自动隐藏、快捷键、关闭/添加请求、亮暗/RTL/1×及2×渲染、接缝像素和重入。

## 第二十五批：紧凑菜单栏

新增 `MenuBar`，通过 `menu_bar.h` 或 `qtfluentwidgets.h` 导入，接入 Gallery「导航」。
沿用 Python 的 32px 高度、14px 字体、文字宽度加 16px 内边距、9px 分隔项、
5px 圆角，以及亮暗悬停/按下背景和禁用透明度。支持 QWidget 的命令 QAction、
分隔项和 RoundMenu 顶层菜单；字符串重载创建由菜单栏拥有的 RoundMenu。

`addMenu` / `insertMenu` / `menus` / `removeMenu` / `clear` 管理顶层入口；
`activeMenu` / `setActiveMenu` 和 `activeMenuChanged` 访问弹出状态，
`triggered` 汇总直接命令及子菜单的 QAction 触发。移除或清空只解除注册，
不会删除菜单；外部菜单的所有权不变。无效插入锚点、外来活动菜单和将子菜单
用作顶层入口会抛出 `std::invalid_argument`。

弹出非阻塞，定位限制在屏幕可用区域；打开菜单后可悬停其他标题切换。
F10 聚焦，方向键切换标题/菜单项，Home/End 跳到首尾可用项，
Enter/Space 打开子菜单或执行命令，Left/Esc 返回上级，根菜单 Esc 关闭。
支持 Alt 助记符和叶子 QAction 快捷键；动态插入/移除和禁用/隐藏父菜单会
同步注册范围。关闭、隐藏、禁用、父窗口移动/缩放及销毁时关闭整个弹出树，
停止延迟子菜单和动画，并在正常关闭时恢复之前的输入焦点。

示例含文件、视图、直接帮助命令、最近文档子菜单、禁用命令和快捷键。
回归覆盖尺寸/亮暗/RTL、菜单复用、动态快捷键、父菜单禁用、方向键/子菜单/
焦点恢复、悬停切换和关闭生命周期。

同步修复旧 RoundMenu 的子菜单行类型识别：快捷键/勾选委托不再把
RoundMenu 指针当作 QAction 读取，避免 Qt 5 绘制崩溃，并补上子菜单箭头。
菜单析构前通知使用方，移除子菜单时清除父菜单/行绑定，防止遗留弹出状态和悬空引用。

## 第二十六批：日历时间选择器

新增 `CalendarTimePicker` / `FastCalendarTimePicker`，通过 `calendar_time_picker.h`
或总头文件导入，加入 Gallery「日期和时间」页顶部。
两者继承 `CalendarPicker` 的按钮、重置开关和年份范围接口；快速版复用
FastCalendarView，普通版复用 CalendarView，而非替换成原生日期时间编辑器。

同一弹层左侧为日期，右侧为 24 小时时间轮盘，底部共享确认/取消栏。
选日期和滚动轮盘只改变草稿，确认或 Enter 一次提交；取消、Esc 或点外部不提交。
日历重置按钮清空已提交值。关闭、隐藏、禁用或销毁选择器时同步关闭弹层。
默认显示秒列；隐藏秒列不改变已提交秒数，再次确认时秒设为零。
运行中切换秒列会放弃当前草稿并关闭弹层，下次打开使用新结构。

`dateTime` / `date` / `time`、`setDateTime` / `setDate` / `setTime`、
`dateTimeFormat` / `setDateTimeFormat` / 字符串 `setDateFormat`、
`isSecondVisible` / `setSecondVisible` 和浮层动画接口可配置状态。
日期/时间单独修改会保留另一部分和原时区；空值下设置日期使用零点，
设置时间使用当天。无效日期/时间等价于重置。
值变化发送一次 `dateTimeChanged`，仅在对应部分变化时发送 `dateChanged` /
`timeChanged`；同值设置和确认不重复通知。日期/重置/格式接口经 CalendarPicker
基类调用仍动态分派。时区改变也会通知，不丢弃原 QDateTime 的时区语义。

视觉沿用 Python 的单一外框、36px 底部按钮栏、37px 轮盘行高、
9/11px 轮盘边距。嵌入日历移除独立阴影和内层背景，轮盘遮罩根据真实视口映射
当前可见行（包含动画中的上下两行），亮暗和 RTL 不依赖旧弹窗的固定偏移。
普通日历在嵌入弹层显示、滚动范围建立后重新定位已选日期，避免首次打开停在年初。
回归覆盖两种日历、秒列开关、原子确认/取消、快捷键、时区、格式、基类调用、
重置/外部点击/关闭生命周期、初始日期可见、亮暗和 RTL 的 1x/2x 渲染及中文翻译。

## 第二十七批：日期区间选择器

对照 Python `components/date_time/__init__.py` 的实际导出及
`range_calendar_picker.py`，新增 `RangeCalendarPicker` / `FastRangeCalendarPicker`。
通过 `range_calendar_picker.h` 或总头文件导入，接入 Gallery「日期和时间」。
普通版沿用滚动日历及 150ms 弹出动画，快速版沿用分页日历与 Flyout 动画；
弹层分别为 338×383 / 314×355，日期格 44×44、14px 字体，保留年月导航和重置。

- 首次点击设置锚点，鼠标悬停预览完整区间，移出恢复单日预览；第二次点击才提交。
  反向区间自动排序；Esc、外部点击及关闭不改变原值，再打开恢复已提交区间。
- 日期间绘制相连的 38px 高半透明带，首尾绘制主题色 1.4px 圆环；今天保持主题色实心圆。
  亮暗背景、悬停与非当前页日期文字透明度遵循 Python；独立弹层显式继承宿主布局方向，
  RTL 时日期格和首尾半格连带一起镜像，避免仅镜像连带却仍按 LTR 排列日期。
- `startDate` / `endDate` / `dateRange` / `setDateRange` 和 `rangeChanged` 管理完整区间。
  `CalendarPicker` 基类的 `date` 返回起点，`setDate` 选择单日区间；重置及两种格式重载
  通过基类调用仍动态分派。与 Python 一样，仅使用 `rangeChanged` 通知，不额外发送 `dateChanged`。
- `setDisabledWeekDays` / `disabledWeekDays` 使用星期一=1 至星期日=7。
  禁用星期只限制端点，内部日期仍包含在区间中；禁用日期文字降透明度并在当前页加删除线。
  配置使现有端点失效时清空区间；越界星期、单边无效日期、禁用端点抛出 `std::invalid_argument`，
  不改变原值；两端都无效则重置。同值设置、重复重置不重复通知。
- Gallery 提供两种选择器、周末端点开关、选择结果及中文说明；库提示和 Gallery TS/QM 同步更新。
  隐藏、禁用、销毁和修改禁用星期时关闭弹层，放弃草稿，避免旧配置仍可提交。

回归覆盖值/通知/格式/基类调用/参数校验、两次选择/反向选择/禁用端点、初始日期定位、
重置、Esc/外部点击及关闭生命周期；两种日历的亮暗、RTL、1×/2× 共 16 组绘制检查
验证连带像素、首尾半格、悬停和移出预览，并输出渲染图供核对。

本批最终验证：Qt 6.7.3 / Qt 5.15.2 的完整 Release 检查各 893 项通过，
0 失败、0 跳过（15 个 CTest 测试组）。两套原成品目录的 EXE 已更新，
均与对应构建 EXE 的 SHA256 一致；没有自动启动 Gallery，没有提交或推送 Git。

- Qt 6：`C22FD9230F689F364678F1B3499A3BF5B59BFE680D10AB58815758BD7BA1CCB5`
- Qt 5：`3C4E56F644DE029FD4BA3EC3A9260BA5ED102C42D88A06FCDCBC39D19E5FB4D9`

## 第二十八批：紧凑可移除标签

根据 Python `widgets/__init__.py` 的实际导出，新增 `SubClip`，而非把 33px 的
可勾选 `Chip` 当作已迁移实现。`SubClip` 使用独立的 24px 行高、13px 字体、
8px 左内边距、4px 标签/按钮间距和 5px 圆角；尺寸提示在布局最小宽度外另加 16px。
亮暗常态/悬停/按下背景直接移植 Python ComboBox QSS，无选中或切换行为。

关闭按钮为 24×24，手绘 1.5px 叉号，悬停/按下背景分别为 20/11 透明度，
按下图标透明度 0.6，保留 Python 的亮暗图标颜色。`closed(text)` 发出当前标签文本，
不会隐藏或销毁标签；应用负责移除。支持关闭按钮的鼠标与 Space 键操作，
禁用时不发关闭信号，隐藏或禁用时清除主体按下状态。

`sub_clip.h` 及总头文件导出组件，`text` / `setText` / `textLabel` / `closeButton`
提供文本和子控件访问；修改文本重新测量尺寸。RTL 沿用 QHBoxLayout 的真实镜像布局，
关闭按钮在左侧，物理左边距仍为 8px，与 Python 一致。
同时补充 `TogglePushButton = ToggleButton`，与 Python 的导出别名一致，无额外实现或状态。

Gallery「基本输入」新增标签关闭/恢复、结果提示和禁用标签示例，紧凑流布局关闭后回收空位。
Gallery 和库中文 TS/QM 同步更新，关闭按钮补可访问名称及工具提示。
回归覆盖文本信号、关闭不自动删除、应用在信号中删除、键盘、禁用、主体按下清理、
尺寸变化、别名与中文翻译，以及亮暗/RTL/1×/2× 的布局和状态像素，共 8 组渲染。
另外使用同一 Segoe UI 字体实际渲染 Python/Qt 6 C++ `SubClip("Selection")`：
亮暗及 LTR/RTL 四种 1× 图像均为 104×24，逐像素比较差异为 0。

本批最终 Qt 6.7.3 / Qt 5.15.2 完整 Release 检查各 903 项通过，0 失败、0 跳过。
两套固定成品目录的 EXE 已更新并核对对应构建文件 SHA256；没有自动启动 Gallery、提交或推送 Git。

- Qt 6：`36971F0A6D98A5B5F1A752BE068F2902CFFBA3AA98C34714A58D9B8CC9942775`
- Qt 5：`C42061F0C08B3B3099D3445270BAF4CD109D2BBC061FB92ABE886989BB410A79`

## 第二十九批：自适应流布局

对照 Python `layout/__init__.py` 的真实导出及 `flow_layout.py` 实现，新增
`AdaptiveFlowLayout`，复用现有 `FlowLayout` 的布局所有权、80ms 防抖和几何动画。
默认最小卡片宽度 200px、横纵间距 10px；按最小宽度确定列数，再均分可用行宽，
取整数余数，不拉伸最后一行。行高取该行卡片的最大自然高度。
`setWidgetMinimumWidth` / `widgetMinimumWidth` 和最大宽度接口完整导出；
最大宽度 `-1` 对应 Python 的 `None`，设置上限不会重新增加列数。
接口在既有总头文件和 CMake 注册的 `flow_layout.h/.cpp` 中提供。

支持边距、非零布局原点、紧凑模式跳过隐藏卡片，以及 RTL 内容区镜像。
测量高度不移动卡片或重启动画；快速改宽从当前几何续接，隐藏父窗口停止计时和动画，
再次显示恢复目标。修复 FlowLayout 插入 spacer 后动画索引错配，以及取出卡片后
`flowAni` 属性悬空的问题，保留普通流布局的自然尺寸策略。

Gallery「布局」加入无动画和动画两套卡片示例，可拖动宽度、限制最大宽度和隐藏末张卡片，
使用现有 Fluent 卡片、标题和说明字体，边距 16/12px、文字间距 6px。
中文 TS/QM 同步更新。回归采用实际 Python 运行得到的 199/200/409/410/620/686px
位置和高度金值，检查列数阈值、余数、原点、边距、宽度上限、RTL、隐藏/移除/重加、
动画测量隔离、连续调整宽度、隐藏恢复、计时器销毁及亮暗/RTL/1×/2× 卡片渲染。

Qt 5 offscreen 的父窗口 `render` 在 RTL / 模拟 2× DPR 下会裁错部分子控件，
截图回归在这一组合逐卡片渲染实际子控件后按真实坐标合成，并检查每张卡片的背景；
1× 和 Qt 6 保留完整父窗口渲染。显式加载 Segoe UI / 微软雅黑，避免中文离屏渲染缺字。

本批最终 Qt 6.7.3 / Qt 5.15.2 完整 Release 检查各 921 项通过，0 失败、0 跳过
（15 个 CTest 测试组，扩展布局组 38 项）。两套原目录成品已更新，
对应构建文件与成品 EXE 的 SHA256 一致；没有自动启动 Gallery、提交或推送 Git。

- Qt 6：`FFA7403E2900326CBA817EB04F0C4C4B522445139877356138BEB620115D501C`
- Qt 5：`F4094CF784A77E2999D3CC951C96305C91428DB81E215C72B80B05B95EF1E8B7`

## 第三十批：补齐已有组件的功能与扩展接口

本批先处理第二十九批末列出的已有组件缺口，不新增整套未迁移组件。

- `PasswordLineEdit` 对齐按住眼睛查看、松开隐藏，保留 Python 的 VIEW 图标；
  添加查看按钮可见性、清除按钮对应的 30/28px 边距和禁止 IME。
  隐藏、禁用、失去鼠标抓取或查看按钮焦点时恢复密码，防止明文状态残留。
  外部直接改 echoMode 后，查询/设置也以真实模式为准。
- 分裂按钮补齐 `setDropIcon` / `setDropIconSize`，自定义图标使用原工具按钮绘制而非固定箭头，
  移除箭头位移动画过滤器。保留 QMenu API，同时支持 QWidget 与 `exec(pos)` 回调，
  对应 Python 动态方法调用；用 QPointer 处理菜单/浮层销毁，定位考虑屏幕边界。
- `CardGroupWidget.setContentWordWrap` 和 `GroupHeaderCardWidget.addGroup(..., wordWrap)`
  支持逐组换行、文本拉伸和右侧控件自然宽度。尺寸使用当前宽度的 Qt heightForWidth，
  避免 Python 静态扣除虚高后，调整宽度仍留旧高度的问题。
- `PickerColumnButton` / `PickerBase` 补逐列 `initialValue` / `setColumnInitialValue`。
  Python 的 PickerBase 原代码调用了列按钮上不存在的方法；C++ 将其补为可用接口。
  初值只用于未提交的弹层草稿，提交值优先，取消不提交，重置保留初值配置；
  已有日期/时间专用定位策略不变。确认时按可见列映射并解码，避免隐藏列或 formatter 错位。
- 公开 `ToastManager.get`、add/remove、margin 和可覆写的滑入/最终位置，现有 ToastStack
  调用管理器，保持六方向、独立窗口、关闭和尺寸变化的既有行为。
  Toast/InfoBar/InfoBadge/Flyout/Menu 管理器均补 `registerFactory`；C++ 的 register 为关键字，
  此接口对应 Python 注册扩展。支持恢复默认，工厂返回空值显式报错；通知管理器在
  有活动通知时禁止替换，避免断开活动堆叠的生命周期。
- `InfoBar.desktopView` 复用并显示透明桌面容器，单例使用 QPointer，容器删除后可重建。
- `AcrylicBrush.setBlurPicSize` 配置采样上限并重建模糊与缩放缓存；空 QSize 恢复全尺寸。

Gallery 的文本、基本输入、日期时间、布局和状态页补可操作示例，中文 TS/QM 同步。
回归检查按住/释放/禁用/隐藏/IME/边距、真实自定义图标、浮层销毁、按宽度换行、
列初值/重置/取消/隐藏列、管理器扩展实际显示路径、独立窗口与自定义通知位置、
活动注册保护、桌面单例删除重建及采样缓存；新增亮暗/RTL/1×/2× 组合渲染。

最终 Qt 6.7.3 / Qt 5.15.2 完整 Release 检查各 938 项通过，0 失败、0 跳过
（15 个 CTest 测试组，本批新增 17 个回归用例）。检查日志为对应构建目录中的
`batch30-release-check-verified.log`。两套固定原目录成品 EXE 已更新，SHA256 与对应
构建 EXE 一致；没有自动启动 Gallery，没有提交或推送 Git。

- Qt 6：`F677498B4FD58AB952B7925C53E599B931D7CF3D41F2072CA49DC92BDF3B362E`
- Qt 5：`08834F3D6CBFB7CA6F43727455FEEC652FD7FB631C2945A0DC46DF7BB541A3C9`

## 验证

Windows 下直接运行 Release 检查脚本，自动编译库、Gallery 和无界面测试：

```powershell
.\scripts\check_release.ps1
.\scripts\check_release.ps1 -QtPrefix D:\Qt\5.15.2\msvc2019_64 -BuildDirectory build-qt515
```

默认通过 `windeployqt` 打包成品到 `build/package/Release/Qt-Fluent-Widgets`，包括 Gallery EXE、Qt DLL 和平台插件，可直接双击检查。
也可使用 `-SkipTests` 跳过测试、`-SkipDeploy` 跳过打包、`-Jobs 4` 限制并行数。
后续检查固定更新该目录，不为每轮迁移建立新目录。如果成品仍在运行，请先关闭，再重新部署；脚本不会终止进程或另建目录绕过锁定。
已有构建目录的 Qt SDK 不匹配时会报错，不会删除或覆盖缓存。

可选无界面测试（默认不构建，不会额外要求普通使用者安装 Qt Test）：

```sh
cmake -S . -B build -DQFW_BUILD_TESTS=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

运行时需将对应 Qt 的 `bin` 放入 PATH。测试通过 Qt 的 `offscreen` 后端执行，不打开桌面窗口。
覆盖亮/暗主题、五种配色、勾选和禁用、图标所有权、RTL、键盘操作、链接激活及光晕边界，
以及进度按钮的状态/取消策略、动画生命周期、Chip 关闭和选中隔离、Tag 类型和绘制。
第三批另有容器测试，覆盖骨架几何/扫光倾斜和裁切、水印绑定与穿透、卡片开关与主题背景、
抽屉四方向/遮罩/Esc/动画反转，以及 ToolBox 动画互斥、快速切换、键盘、动态高度和移除页面所有权。
第四批输入测试覆盖连续评分预览与提交、只读/键盘/无焦点框、范围钳制/极值/手柄拖动、提示生命周期、
前后缀边距与 QAction、PIN 移焦/退格/粘贴/校验器/重建，以及亮暗主题和 RTL 绘制。
第五批补充副标题换行/点击/三态/互斥、日期时间字段增减和边界/闰年、RTL 编辑区避让、
紧凑数值按钮、浮层关闭/销毁、只读设置保留、重复按钮、错误状态与翻译测试。

## 第三十一批：富文本编辑器、主色工具下拉按钮与 DWM 菜单

- `TextEdit` 按 Python `QTextEdit` 实现移植，保留 HTML/Markdown、富文本粘贴、
  撤销/重做、选择和只读交互，接入 Fluent 样式、14px 字体、平滑滚动及 `TextEditMenu`。
  编辑菜单关闭后释放，不累积隐藏菜单；只读状态同时禁止撤销/重做等修改操作。
- 公开 `EditLayer` 还原透明鼠标穿透的焦点绘制层：底部 10px 圆角路径减去顶部 7.5px，
  留下 2.5px 主题色底线。随父控件尺寸/焦点/主题变化更新，禁用不绘制；
  同时补齐现有 `PlainTextEdit` / `TextBrowser` 缺失的同一绘制层。
- `PrimaryDropDownToolButton` 继承 `PrimaryToolButton`，默认 60×32、图标 16×16，
  保留 Python 的领先侧 12px 图标、尾侧 10px 箭头、亮暗主色与图标反色，
  以及 150ms 下移/500ms 弹性恢复动画。补齐原 QSS 缺失的主色 hover/pressed/disabled
  选择器，支持 RTL 镜像、鼠标及 Space/Enter/Down 打开菜单。
  复用 RoundMenu 按屏幕上下空间选择展开方向的定位；原生 QMenu 也可绑定。
  菜单使用 QPointer，销毁后不留下悬空指针。
- `DWMMenu` 忠实保留 Python 的 QMenu 包装：Frameless/Popup/NoDropShadow 标志，
  StyledBackground、14px 小图标代理样式和 Menu QSS，支持 QAction 勾选、禁用及子菜单。
  Windows 原生句柄每次创建/重建时应用 DWM 非客户区渲染策略和 {-1,-1,-1,-1} 边距。
  Qt 6 原生菜单样式默认请求透明分层窗口，该窗口无法启用 DWM 非客户区阴影。
  本组件在样式初始化和主题样式刷新后关闭这一透明背景属性，保留原生菜单绘制；只读取已有缓存句柄，
  不在销毁事件中调用 winId 重新创建窗口。
  Python 的 Menu QSS 没有专属 DWMMenu 背景规则，故保留 QMenu 本身的菜单绘制行为，
  不将 RoundMenu 的列表圆角外观误用到此组件。
- Gallery：文本页原先名为 TextEdit 的 TextBrowser 示例换成真正可编辑 TextEdit，
  加入只读开关；基础输入页加入主色工具下拉菜单；菜单页加入 DWM 菜单示例。
  新增 13 条中文翻译，原有总头文件和 CMake 已登记这些源码，新增类随原文件公开构建。
- 回归覆盖富文本格式/粘贴/撤销/只读/右键菜单释放、滚动与父子生命周期，
  工具按钮几何/键盘/菜单删除和 8 组亮暗×RTL×1x/2x 渲染，焦点底线透明区域与像素颜色，
  DWM 菜单标志、勾选、Esc 与子菜单。另加 Windows 隐藏原生窗口测试，
  查询 DWM 非客户区渲染是否启用，并刷新样式、销毁/重建句柄再次查询，全程不显示 Gallery。

Qt 6.7.3 / Qt 5.15.2 完整 Release 检查均通过：16 组、各 955 项，0 失败、0 跳过。
检查日志分别为 `build/batch31-release-check-last.log` 和
`build-qt515/batch31-release-check-last.log`。两套固定成品目录已更新，未自动启动 Gallery；
构建 EXE 与成品 EXE 的 SHA256 一致：

- Qt 6：`FC8FDD024E5253CAC47E9C399DCDD72AE17635895156B4A4018619C8CEB049E7`
- Qt 5：`2C02D8FA72A91AE001660746B32156A725983A530B19C0916FEC5C78335F3EF8`

## Python 视觉对齐回归

- `GroupHeaderCardWidget` 使用基础布局已经包含全部分组行的尺寸提示，
  不再重复累加正文高度。保留自然最小高度以兼容设置页的 `ExpandLayout`，
  不在 Gallery 硬编码高度；亮暗主题、RTL、1/3/6 行和反复调整宽度均有几何回归。
- Toast 竖排文字使用当前宽度下的实际换行高度，标题与正文间距固定为 10px，
  上下内边距为 18px / 16px；标题使用 13px 半粗体，正文使用 14px。
  亮暗标题颜色、仅标题/仅正文/空通知、关闭按钮和 RTL 均有几何回归，
  避免布局拉伸或过期的尺寸提示导致卡片变高、文字之间出现额外空白。
- Toast 宽高直接通过不受固定高度限制的 QLabel 测量，计入系统字体回退、
  字形边缘与 DPI 取整，不再仅使用文字 advance 宽度；短中文标题不会差 1px 被挤换行。
  Windows 原生字体引擎在 100% / 125% / 150% / 175% / 200% 缩放下运行隐藏窗口回归，
  并验证长内容换回短内容、窗口变宽和字号恢复后，通知高度正确缩回。
- 副标题复选框和单选框分别沿用 Python 的双行基准高度、标题和描述绘制位置；
  保留 C++ 的换行、RTL、整块点击以及可调间距，长描述不会改变标题和指示器位置。
- `QConfig::themeColor()` 保存原始主题色，`qfw::themeColor()` 返回 Python `ThemeColor.PRIMARY`
  对亮/暗主题计算后的颜色；QSS 主色与手绘控件一致。派生色从原始配置色计算，避免二次提亮。
- 修正旧滑块、复选框/单选框、进度条/环、输入框、列表/树/表格、菜单和导航指示器的默认用色；
  自定义亮/暗颜色保持原值且优先。范围滑块和快捷键键帽使用同一主色。
- 新增四种主题色的 Python 七级配色金值、双行文字像素对照、横/纵滑块与键帽渲染回归。
- 树状单选/多选下拉框的悬停和选中背景覆盖整个可见行（含分支缩进），
  不叠加单元格背景；亮/暗主题、RTL、缩进区悬停和移出均有像素回归。
- 树状下拉框的半选横线使用 `QRectF` 几何中心，避免偶数尺寸的 `QRect::center()`
  向左上取整；亮/暗主题、RTL 和 1x/2x 缩放均检查横线像素中心。
- Python/C++ 树状单选与多选下拉框默认启用实时搜索。`setSearchEnabled`、
  `setSearchText`、`searchText` 和 `searchLineEdit` 可配置/访问搜索框；
  搜索是忽略大小写的字面匹配，父节点匹配保留子树，子项匹配保留祖先，关键词高亮。
  搜索只隐藏视图行，不改变源模型、选项身份或已选项；父节点批量勾选仍作用于
  全部可用后代。清空搜索恢复展开状态，键盘方向键/回车转入树，Esc 关闭弹层。
- C++ 树节点显式采用 Python 的 13px 字体和实测 32px 行高；单选行在领先边缘
  绘制主题色指示条，RTL 时镜像，多选行保留复选框而不额外添加指示条。

## 第三十二批：下拉取色器与屏幕取色器

- 对照 Python `drop_down_color_picker.py` / `screen_color_picker.py` 实现两个独立组件，
  加入总头文件、CMake 和 Gallery「基本输入」页，不以原生颜色对话框替代。
- `DropDownColorPicker`：54×32 按钮、320px 宽浮层、288×288 色相/饱和度面板，
  使用现有 Fluent Slider 的动画手柄绘制亮度和 RGBA/HSVA 梯度。
  RGB/HSV 模式、整数校验、六/八位 HEX、透明棋盘、透明度开关与固定确认栏均对应参考。
  草稿只发预览；确认后一次提交，取消、Esc、隐藏或禁用保留原值。
  灰色/黑色和透明度编辑保留色相，打开期间外部更新颜色同步草稿。
- `ScreenColorPicker`：60×32 按钮及原版滴管 SVG；开始前逐屏截图，默认冻结屏幕，
  通过每屏实际图像像素/逻辑矩形比映射坐标，支持负坐标、分数位置和混合 DPI。
  预览卡 106×40，含阴影容器 138×76，贴近指针并约束在当前屏幕。
  左键/回车提交，右键/Esc 取消，预览不提交；相同颜色确认仍发送 `colorPicked`。
- 实时模式每 33ms 重新读取指针处像素，Windows 10 2004+ 在显示覆盖层前设置
  `WDA_EXCLUDEFROMCAPTURE`，防止把自身预览读回。失败和不支持的平台发出明确错误，
  不静默退化；冻结模式保持可用。提供受保护的截图/实时采样/覆盖层准备钩子，
  便于应用自定义采集后端和测试注入，不增加可选第三方依赖。
- 单个屏幕取色会话互斥；屏幕拓扑/DPI 改变、应用失活、隐藏、禁用、切换模式和销毁
  均清理覆盖层、截图及定时器。信号回调销毁控件时使用 `QPointer` 防止后续访问。
  浮层关闭停止动画并延迟销毁，重复打开不积累旧编辑器；两个按钮支持 RTL 镜像。
- 新增组件与 Gallery 中文翻译，并补入 Python 原版亮暗滴管资源；枚举追加到末尾，
  保留原有图标枚举数值。新增取色器测试、Windows 隐藏窗口截图排除检查，
  以及只采样 240×180 受控测试色块的原生实时验证（覆盖层限制在该测试区域，
  不采集/保存桌面截图），检查预览覆盖位置仍能读到下方色块及其重绘后的颜色。
  渲染检查初始化静态资源并断言 QSS/滴管存在，覆盖亮暗主题、RTL、1×/2×、
  普通/hover/按下/禁用、RGB/HSV；与 Python 图像核对尺寸、字体、梯度和间距。
  Qt 5 小屏的编辑区可滚动，确认栏保留在浮层内。
- Qt 6.7.3 / Qt 5.15.2 完整 Release 检查各 18 组、1000 项通过，
  0 失败、0 跳过。新取色器回归各 41 项，原生检查各 4 项；
  Windows 实际取色、预览排除、下层色块重绘及确认在两个版本均通过。
  完整日志：`build/batch32-release-check.log`、`build-qt515/batch32-release-check.log`。
- 两套构建 EXE 已覆盖到固定的原目录成品，SHA256 均与对应构建 EXE 一致；
  更新时没有匹配路径的 Gallery 进程需要终止，没有自动启动 Gallery，也未提交/推送 Git。
  - Qt 6：`build/package/Release/Qt-Fluent-Widgets/qtfluentwidgets_app.exe`，
    `142D0B1AA7B5A0F25B87165599103A20475CDC8B979BFF8080423CC4D059283A`。
  - Qt 5：`build-qt515/package/Release/Qt-Fluent-Widgets/qtfluentwidgets_app.exe`，
    `91EAA0E59810DD6C203CEE4E6E0C6ED94FB996E85B3142C38A6EE2F854D0B7F8`。

## 第三十二批后补修：DWM 菜单白边

- 用户实拍指出 Qt 6 Windows 11 下 DWM 菜单右侧/底部出现白色条带。
  第三十一批的隐藏窗口检查只确认 DWM 非客户区渲染启用，未验证原生菜单显示结果，
  因而不能作为边缘/阴影视觉正确的证据。
- 用实际 Windows 测试菜单复现：Windows 11 原生 QMenu 绘制保留内部透明装饰，
  与为 DWM 阴影取消 layered/translucent 属性的处理冲突。
  为 DWM 菜单及其后代 QMenu 添加独立的亮暗主题背景/边框与菜单项样式，
  完整绘制不透明客户区，保留 DWM 阴影而避免未绘制白边。
  添加 Fluent 勾选与子菜单箭头资源、禁用色、hover/选中背景和分隔线；
  Gallery 子菜单也使用 DWMMenu。样式限定在 DWM 菜单，不修改其他菜单。
- 新增真实 Windows 菜单/子菜单截图与边缘像素断言，覆盖亮暗主题、RTL、
  当前系统 DPI 及绝对 1×/2×；截图等待系统菜单淡入完成。
  测试色块使用互斥锁，避免两套原生截图测试窗口互相遮挡；不自动启动 Gallery。
- 按后续实拍反馈，将 DWM 菜单项的横向留白从 2px 调整为 8px，图标/勾选标记
  向内移 6px，各行文字列保持对齐，RTL 使用相同的两侧间距。
  增加图标列像素扫描，断言图标/勾选标记距领先边框至少 10px。
  原生 1×/2× 测试显式关闭系统 DPI 倍率叠乘，并断言实际 DPR，
  避免 Qt 6 将 Windows 的 175% 再乘以测试倍率。
- 本次 Qt 6.7.3 / Qt 5.15.2 完整 Release 检查各 20 组、1022 项通过，
  0 失败、0 跳过，包含系统 DPI、1×/2×的真实菜单与子菜单截图检查。
  最终日志（含图标间距与实际 DPR 断言）：`build/dwm-inset-release-check.log`、
  `build-qt515/dwm-inset-release-check.log`。
  真实截图保存在对应构建目录 `tests/dwm-native-menu-*`、`dwm-native-submenu-*`
  与 `dwm-native-detail-*`。
- 两套原目录成品已更新，SHA256 与对应构建 EXE 一致；没有需要终止的匹配路径进程，
  未自动启动 Gallery，未提交/推送 Git。本次哈希取代第三十二批初版哈希：
  - Qt 6：`89CD91E1EBEB1FB77B4EA18F3B2749E5CDB8E6C0FE0C1CFB1969FD2B9E639C7A`。
  - Qt 5：`1199243906257BFE855730427BAE88C253677D561A1377B486739EE12F9E3556`。

## 第三十三批：AcrylicWidget / AcrylicToolTip / AcrylicToolTipFilter

- 对照 Python 的 `acrylic_widget.py`、`acrylic_tool_tip.py`、基础 ToolTip、
  嵌入资源 QSS 和 `image_utils.gaussianBlur` 继续迁移；保留此前所有改动。
- AcrylicWidget 在 C++ 中采用可派生的 QFrame 形式，提供 `acrylicBrush()`、
  `acrylicClipPath()` 及受保护的材质绘制方法，适配 Python 多重继承 mixin。
  默认模糊半径 30；暗色 tint=(32,32,32,200)，亮色 tint=(255,255,255,180)，
  luminosity 透明，沿用 0.03 噪点。动态路径取消后清除旧裁剪，避免残留裁剪。
- AcrylicToolTipContainer 保留 transparent 属性、内缩 1px/半径 3 的材质路径。
  ToolTip 改用受保护容器工厂构造函数，解决 C++ 构造期间虚函数不分派到派生类的问题；
  沿用外边距 (12,8,12,12)、容器边距 (8,6,8,6)、12px 字体、4px 外框，
  25px/偏移 (0,5)/alpha50 阴影与 150ms 淡入。
  每次显示前按容器全局位置和实际尺寸抓取背景；沿用定位、显示时长和自动隐藏行为。
- AcrylicToolTipFilter 提供 300ms 默认延时和全部 ToolTipPosition，继承悬停/离开/
  点击/隐藏/禁用逻辑。使用 QPointer 管理工具提示，过滤器销毁时释放由它创建的提示，
  提示被窗口提前释放后清空基类指针。
- 对照发现原 AcrylicBrush 的 QGraphicsBlurEffect 会模糊 alpha、产生透明边缘；
  Python 仅模糊 RGB，使用反射边界、4σ 截断及逐轴 uint8 输出。
  Brush 改为可分离高斯卷积，保留原 alpha；预计算边界索引，沿用缓存和采样尺寸。
  添加由 Python scipy 生成的四个数值基准与透明度回归。AcrylicLabel 原实现不在此改动中。
- 已加入总头文件、CMake、Gallery 材质页和中文翻译；新示例展示背景模糊/
  主题 tint/纹理与真实悬停工具提示。Gallery 示例使用既有图片并限制采样尺寸。
- DWM 菜单项横向 margin 从 8px 再调整至 14px，图标/勾选进一步内移 6px，
  保持文字列对齐；真实 Windows 亮暗、LTR/RTL、1×/2×图标断言提高至至少 16px。
- 新材质回归覆盖亮暗、RTL、1×/2×、字体/边距、颜色/背景、裁剪重置、
  重复显示刷新、文本重测、显示时长、延时取消、禁用、销毁与中文翻译。
  另加入真实 Windows 受控色块背景取样及中文截图；测试不启动 Gallery。
- Qt 6.7.3 / Qt 5.15.2 最终完整 Release 检查各 23 组、1049 项通过，
  0 失败、0 跳过。最终重建包含高斯模糊修复，并重新链接 Gallery 和全部测试程序。
  日志：`build/material-final-release-check.log`、
  `build-qt515/material-final-release-check.log`。
  截图：各构建目录 `tests/acrylic-tooltip-*` 和 `tests/dwm-native-detail-*`。
- 两套原目录成品 EXE 已更新，SHA256 与对应构建 EXE 一致；
  未发现需要终止的精确匹配路径进程，未自动启动 Gallery，未提交/推送 Git。
  - Qt 6：`03AC9A87B37B8E03D5B40801AEE7576F44198AA63FC34CA2467B48F2C2ECF498`。
  - Qt 5：`F40CF11812FC21AB520CAE2471EF39E0FFC06A39041690A9F9C787C9D29D0EFC`。

## 第三十四批：AcrylicFlyout 三个接口与瀑布流透明背景修复

- 已读取 Python `acrylic_flyout.py`、`acrylic_widget.py`、基础 Flyout/ScrollArea，
  在第三十三批材质画刷基础上继续实现 AcrylicFlyoutViewBase、AcrylicFlyoutView、AcrylicFlyout。
  C++ 材质视图分别继承对应基础视图，并持有 AcrylicBrush，适配 Python mixin。
- 视图保留内缩 1px/半径 8 的材质裁剪、主题 tint、透明 luminosity、0.03 噪点和边框。
  默认视图沿用标题/正文的 14px 字体、图标、32px 关闭按钮、图片圆角和自定义内容布局。
  图片按内容行布局宽度缩放，避免继承旧 FlyoutView 固定约 400px 的图片宽度。
  图片调整扩展点改为受保护虚函数；完整构建暴露 TeachingTipView 原先只声明未实现的
  同名方法，补为调用基类，保留该组件现有图片行为。
- AcrylicFlyout 提供 make/create、目标控件或 QPoint、六种动画、fadeOut 和关闭生命周期。
  基础 Flyout 增加显示前背景准备钩子；在动画定位完成计算后、窗口实际显示前取样。
  取样区域按视图真实位置/尺寸计算，排除阴影边距，避免参考代码重复位置修正和区域偏移。
  make 不提前 show，避免首次取样包含已经打开的弹窗。
- Gallery 材质页新增默认浮出组件与自定义视图两种示例，并补齐中文翻译、总头文件和 CMake。
- 用户实拍的瀑布流黑底来自 ScrollArea：先 enableTransparentBackground，后 setWidget，
  QScrollArea 会重新启用内容 autoFillBackground，且此前没有处理尚未加入的内容容器。
  ScrollArea 保存透明状态，在 setWidget 后重新应用；禁用视口/内容自动填充背景。
  样式只限定到视口/内容容器属性，不以 QWidget 全局规则覆盖子控件背景。
- 回归覆盖透明开关前/后两种顺序、内容替换、亮暗、1×/2×及空白区域像素；
  材质覆盖亮暗/RTL、字体/边距/圆角、图片宽度、关闭信号、六种动画定位、
  显示前取样、自动释放/保留实例，以及真实 Windows 受控色块背景截图。
  两套材质测试在 1×/2× 下各 27 项通过，真实 Windows 背景取样也通过。
- 根据继续工作期间的反馈，修正 DWM 图标内移同时缩窄 hover 背景的问题：
  item 横向 margin 恢复到 2px，padding 增至 38px 以保持文字列和整体尺寸；
  icon/indicator 使用 12px 相对绘制偏移，Qt 自动镜像到 RTL。
  图标保持此前内移的位置，hover 背景恢复到菜单两侧；新增选中行两端像素断言，
  与图标距领先边框至少 16px 的断言同时验证。
- Qt 6.7.3 / Qt 5.15.2 最终完整 Release 检查各 23 组、1079 项通过，
  0 失败、0 跳过；结果包含本轮 AcrylicFlyout、透明背景及最后的 DWM hover 修复。
  最终日志：`build/acrylic-dwm-hover-final-check.log`、
  `build-qt515/acrylic-dwm-hover-final-check.log`。
  截图保存在各构建目录 `tests/waterfall-transparent-*`、`tests/acrylic-flyout-*`
  和 `tests/dwm-native-detail-*`。
- 两套原目录成品 EXE 已更新，SHA256 与对应构建 EXE 一致；未发现需要终止的
  精确匹配路径进程，未自动启动 Gallery，未提交/推送 Git。
  - Qt 6：`568701E2C3C5EECBC7B10C78B906F596D0F037497E45B156C94F22F8B009D32A`。
  - Qt 5：`CE392FF0DDFF2D5C2FADC448B10D33A88E6DED0AC7CB5578BC4E53238DCE5924`。

## 第三十五批：五种 Acrylic 菜单

- 逐行对照 Python `material/acrylic_menu.py`、基础菜单及输入框上下文菜单，
  实现 AcrylicMenu、AcrylicCheckableMenu、AcrylicLineEditMenu、AcrylicSystemTrayMenu、
  AcrylicCheckableSystemTrayMenu，并加入 CMake、总头文件、Gallery 材质页与中文翻译。
- 材质列表使用 35px 模糊、8px 圆角、0.03 噪点；暗色 tint 为 (32,32,32,200)，
  亮色 tint 为白色 alpha 160、luminosity 白色 alpha 50。顶部/底部各 6px
  不可选择占位行随列表滚动，修改行高时保留边距。
- 基础菜单增加列表替换和显示前背景准备扩展点；保留动作绑定、快捷键、禁用状态、
  勾选/单选 delegate、分隔线、子菜单及输入框编辑动作。清空时保留材质边距，
  支持 Enter 激活动作。背景按最终 viewport 位置取样，排除阴影边距。
- 托盘菜单在屏幕边缘修正最终位置，同时更新动画终点和背景取样位置；覆盖普通和
  可勾选两种托盘菜单的五种动画。构造时取消默认边框，修正首次显示取样 1px 偏差。
- 截图复查发现基础菜单快捷键未镜像，修为按 Qt visualRect 放置；行高改变后同步
  调整菜单外框，避免视图增高而内容被裁剪。
- Gallery 提供普通、可勾选、托盘、可勾选托盘四种菜单按钮，以及输入框右键示例。
  此批仅迁移输入框上下文菜单，不将预览用基础 LineEdit 计为 AcrylicLineEdit 完成。
- 回归覆盖亮暗、RTL、1×/2×、材质像素、边距/行高、动作点击、Enter、增删/清空、
  单选/勾选 delegate、复制行为、五种动画与托盘屏幕边缘定位，另有 Windows
  受控色块背景取样截图。两套材质回归在 1×/2× 下各 43 项通过，
  真实 Windows 背景取样也通过；人工复查亮暗、RTL 和原生截图。
- Qt 6.7.3 / Qt 5.15.2 最终完整 Release 检查各 23 组、1111 项通过，
  0 失败、0 跳过；结果包含最终快捷键镜像和行高裁剪修复。
  日志：`build/acrylic-menu-release-check.log`、`build-qt515/acrylic-menu-release-check.log`。
  截图：各构建目录 `tests/acrylic-menu-*`。
- 两套原目录成品 EXE 已更新，SHA256 与对应构建 EXE 一致；未发现需要终止的
  精确匹配路径进程，未自动启动 Gallery，未提交/推送 Git。
  - Qt 6：`98AE870EB8FAEBBAF12015F00B2FA1FD2656289DE34FF88097E4D49050BD5ADD`。
  - Qt 5：`06B4DF5782097FEA433381E36D9346A0847F38F31FEA34C8D2DE910E094CDDB3`。

## 第三十六批：Acrylic 输入框与补全菜单、Gallery 系统缩放修复

- 对照 Python `acrylic_line_edit.py` 和 `acrylic_menu.py` 的内部补全实现，新增
  AcrylicLineEditBase 模板适配、AcrylicLineEdit、AcrylicSearchLineEdit、AcrylicCompleterMenu。
  输入框沿用基础 LineEdit/SearchLineEdit 的 33px 高度、字体、焦点/错误/禁用状态、
  清除与搜索按钮；右键菜单和补全菜单改用材质实现，不给文本编辑区另铺亚克力背景。
- 补全菜单保留 33px 行高、上下各 6px 不可选占位行、选中指示条、滚动条和开合动画。
  根据向下/向上展开裁剪靠近输入框的两个圆角；显示前按最终位置取样。
  点击及上下键/Enter 选择按占位行偏移映射模型索引，避免第一条补全对应错误索引；
  键盘移动不会落到占位行。支持模型刷新、重复设置/清除补全器及搜索信号。
- 基础 LineEdit 暴露受保护的补全菜单替换扩展点；替换时释放旧菜单，保留 activated
  文本/索引信号转发。CompleterMenu 增加可重写的数据设置与行索引偏移扩展点。
- Gallery 材质页加入两个输入示例和 A 开头的补全词库，提供中文翻译、CMake 与总头文件。
- 用户反馈 Qt 5 成品整体偏小、关闭按钮相对偏右：启动代码在 Auto 时未启用
  Qt 5 的 AA_EnableHighDpiScaling，且没有统一两版的小数缩放舍入策略。
  在 QApplication 创建前启用 Qt 5 系统缩放/高 DPI pixmap，统一为 PassThrough；
  手动缩放仍使用指定倍率。关闭按钮保持同一套 46×32 逻辑尺寸和右侧布局。
- 新增独立 startup 实际窗口回归，复用 Gallery 启动配置；检查 Auto 对应 Windows
  GetDpiForWindow 的倍率及 100/125/150/175/200% 手动倍率、关闭按钮右侧边距和物理像素尺寸。
  测试只显示短暂测试窗口，不自动启动 Gallery。材质回归补上亮暗/RTL/1×/2×
  补全裁剪、鼠标/键盘模型索引、替换释放和搜索信号。
- 本机 Auto 实测为 175%；两版关闭按钮截图在六档倍率下物理尺寸及 X 图标
  不透明黑色像素包围盒完全一致，Auto 均为 81×56px、X 范围 (31,19)-(48,36)。
  已人工复查补全菜单亮暗/RTL/1×/2×截图。
- Qt 6.7.3 / Qt 5.15.2 最终完整 Release 检查各 29 组、1139 项通过，
  0 失败、0 跳过；两套材质回归在 1×/2× 下各 48 项通过。
  日志：`build/acrylic-edit-release-check.log`、`build-qt515/acrylic-edit-release-check.log`。
  截图：各构建目录 `tests/acrylic-completer-*`、`tests/startup-close-*`。
- 两套原目录成品 EXE 已更新，SHA256 与对应构建 EXE 一致；未发现需要终止的
  精确匹配路径进程，未自动启动 Gallery，未提交/推送 Git。
  - Qt 6：`CB53E8514614C5EFC09622815CE62B4A899696A0E3D2870A8FCAF54FC6FFE4B5`。
  - Qt 5：`B09D7DC67C9FB82AAAEB37E569812F1A2B43AA7E6B2D67A8ADA577C585A8A3DD`。

## 第三十七批：剩余三个 Acrylic 材质组件

- 读取 Python `material/acrylic_combo_box.py`、下拉框与设置卡片实现，新增
  AcrylicComboBox、AcrylicEditableComboBox、AcrylicComboBoxSettingCard，
  以及内部 AcrylicComboBoxMenu。沿用已有材质画刷和菜单，不以原生 QComboBox 替代。
- 菜单保留 35px 模糊、亮暗 tint/luminosity、0.03 噪点和 8px 圆角，
  选项行高 33px、顶部占位行 2px、底部 6px；保留指示条、图标、禁用状态、
  滚动条、当前选项定位及向上/向下展开动画。
- 普通按钮和可编辑输入区沿用基础组件的字体、尺寸、间距和交互状态；
  可编辑版本同时继承 AcrylicLineEditBase，提供材质右键菜单和材质补全菜单。
- 基础下拉框原先按显示文字 trim 后 findText 查找选择项，重复文字和首尾空格
  会对应错误索引。改为绑定 QAction 的选项索引；选中定位通过 QAction 关联的
  列表项获取，避免材质菜单占位行造成偏移。键盘 Enter 与鼠标共用动作选择路径。
- 设置卡片保留 SettingCard 布局、图标、标题/内容字体、右侧控件与 16px 末尾间距。
  选项按 texts/options 的交集绑定 userData，支持初值、用户选择、外部配置更新和 setValue；
  通过数据定位相同显示文字的不同选项，无效或未映射值不改变选择。
  外部配置更新仍发出控件索引信号，ConfigItem 相同值保护避免递归通知；
  QPointer 保护配置项销毁后的调用。
  设置卡片在 LayoutDirectionChange 时镜像原 16px 领先边距，修正 RTL 图标贴边。
  基础下拉按钮绘制区域与箭头按布局方向镜像，修正 RTL 箭头与文字重叠。
  可编辑下拉框按方向保留 29px 下拉按钮空间，启用清除按钮时再保留 28px，
  避免固定右边距在 RTL 下失效、以及清除开关覆盖原下拉按钮边距。
  截图使用与 Gallery 一致的亮/暗容器背景，避免透明卡片在默认浅色测试背景上误判。
- 已加入 CMake、总头文件、Gallery 材质页三种示例和中文翻译；可编辑示例展示
  自定义文本、补全与右键菜单，设置卡片展示配置绑定。
- 新回归覆盖重复文字的数据/当前项映射、禁用点击、鼠标与 Enter 激活信号、
  可编辑自定义项、材质补全/右键菜单及销毁、配置双向同步/信号/空配置/无效值，
  以及亮暗、RTL、1×/2× 控件/菜单/卡片截图、边距、行高和材质像素。
  Windows 受控色块背景用例增加实际材质下拉菜单取样和截图。
- 完整检查定位旧输入回归的 Qt 5 文件选择失败：测试在文件模型加载/排序前缓存
  QModelIndex，排序后旧 row 会指向目录。测试改为在模型就绪后取得持久索引，
  点击打开前断言实际 selectedFiles 与预期一致；未改动 DropWidget 生产实现。
- Qt 6.7.3 / Qt 5.15.2 最终完整 Release 检查各 29 组、1153 项通过，
  0 失败、0 跳过；两套材质回归在 1×/2× 下各 55 项通过，输入回归各 83 项通过。
  最终结果包含 RTL 卡片边距、普通按钮绘制、可编辑文本边距及持久模型索引修复。
  日志：`build/acrylic-combo-release-check.log`、`build-qt515/acrylic-combo-release-check.log`。
  亮暗/LTR/RTL/1×/2× 控件、菜单、卡片和原生背景截图：各构建目录 `tests/acrylic-combo-*`。
- 两套原目录成品 EXE 已更新，SHA256 与对应构建 EXE 一致；未发现需要终止的
  精确匹配路径进程，未自动启动 Gallery，未提交/推送 Git。
  - Qt 6：`657E41EF8DF232CA755561C62794EAFFC9C69364218DE7CF81D3D3DD3C30FBA2`。
  - Qt 5：`6FB31CDC61C05BBAB78AA64B5B4EF5DD72CB132E7734C5A0A9FF79E6AE08C09C`。

## 第三十八批：CodeEdit / CodeLanguage 核心实现

- 读取 Python `widgets/code_edit.py`、`tests/check_code_edit.py` 和 Gallery 的 20 种语言示例，
  新增 C++ `CodeEdit` / `CodeLanguage`，加入 CMake 和总头文件。继承已有 Fluent PlainTextEdit，
  保留圆角、亮暗/hover/禁用/焦点样式、平滑滚动、原生 IME/撤销和 Fluent 上下文菜单。
- 固定字体 11pt、NoWrap、默认四空格缩进；行号宽度为 `20 + 数字宽度 × 位数`，右边预留 8px。
  行号区紧邻 viewport；当前行亮暗分别为白色 alpha 18 / 黑色 alpha 24，末行只填充文本行高度。
  隐藏行号、只读取消当前行背景、字体变化后的缩进距离和行号宽度更新均已实现。
  按截图反馈补齐 FullWidthSelection 留下的文档左右边距背景，消除行号与正文之间的断缝；
  分别适配 Qt 5/6 不同的右侧绘制边界，避免透明色重复叠加产生深线；覆盖横向滚动后的边距。
  新增整行空白区逐像素连续性回归，不以只检查 viewport 几何代替背景检查。
- 保留 20 种语言和别名、languageChanged/highlightingFinished/highlightingFailed；切换语言保留正文，
  不支持语言抛 `std::invalid_argument`，不改变现有语言。单行 Tab 按列补空格，选区倒序缩进，
  排除末端恰好落在下一行起点的行；反向缩进、Python 冒号/C 系大括号自动缩进为单次撤销。
- C++ 后台词法器以不可变 QString 快照工作，不依赖 Python/Pygments 运行时、不访问 GUI 对象。
  80ms 防抖、最多一个正在执行的任务、版本丢弃、128 行分批应用、百万 Unicode 字符保护已实现。
  包括跨行注释/字符串、Rust 嵌套注释与 raw string、C++ raw string、PowerShell here-string、
  Bash heredoc、HTML script/style、Markdown 围栏，以及 Python/JS/C# 字符串插值；使用 UTF-16 定位。
  已修复嵌入扫描的临时字符串生命周期问题，递归深度限制为 32，编辑器销毁不留下 GUI 回调。
- 对照本地 Pygments 实际 token 输出校正了普通函数调用、CSS 属性/类名/色值、Java 成员、
  YAML scalar、TOML/INI 键值及各语言常量的分类，采用 Python 同样的亮暗 token 配色。
  **词法兼容性仍有缺口**：原版 Pygments 完整状态机尚未逐 token 移植；Python f-string 格式说明符、
  JSX、复杂 CSS selector/at-rule、Shell 参数展开等边界语法尚未完成差分验收。
  本批不能宣称与 Pygments 全语法等价，也不将 CodeEdit 列为已完全验收。
- Gallery「文本」页新增语言切换、加载示例、只读、行号开关和编辑区；每种语言均有示例。
  切换语言只改变高亮，只有加载示例会替换正文。新增四条 Gallery 中文翻译和一条库错误翻译。
- 新增 `qfw_code_edit_tests`，包括 20 语言关键 token、参考颜色分类、跨行修改、UTF-16、
  缩进/撤销/只读、5000 行、快速修改/语言切换、任务中销毁、超限恢复、亮暗/LTR/RTL 和翻译。
  加入完整检查脚本的 1×/2× 离屏与 Windows 字体引擎组；截图包含父背景，避免透明 PNG 被误判为白底。
- 本地 Python `tests/check_code_edit.py` 离屏检查也已运行并通过，确认参考的 20 语言、
  多行/Unicode、主题、缩进撤销、只读、行号与过期任务用例有效。
- Qt 6.7.3 / Qt 5.15.2 最终完整 Release 检查各 **32 组、1227 项通过**，0 失败、0 跳过。
  新增代码编辑器回归在 1×/2× 下各 34 项通过，Windows 字体渲染各 6 项通过；
  最终结果包含整行背景连续性、左右边距、横向滚动和 Qt 5/6 绘制边界修复。
  日志：`build/code-edit-release-check.log`、`build-qt515/code-edit-release-check.log`。
  截图：两套构建目录 `tests/code-edit-offscreen-*`、`tests/code-edit-windows-*`，已复查亮暗/RTL/缩放。
- 两套原目录成品 EXE 已更新，SHA256 与对应构建 EXE 一致；没有需要终止的精确路径 Gallery 进程，
  未自动启动 Gallery，未提交/推送 Git。
  - Qt 6：`389D8ED0439C505D3757790AB291725B5EE9872B17AD5CBDB06678E2C3FF9B63`。
  - Qt 5：`69CB283FF2670A8884E41181789495C80546930CCE38AE1A8CEA1ABAB904E0C1`。

## 第三十九批：CodeEdit 复杂词法补齐（2026-10-10）

- 仔细核对 Python CodeEdit 的 tokenFormat 与本地 Pygments 2.19.2 实际规则，
  将 Python、JavaScript、TypeScript、CSS、Bash 五种语言的规则顺序、分组输出、
  状态压栈/弹栈及 Python soft-keyword 内嵌扫描导出为原生 C++ 正则状态表。
  使用 Qt Core 解压和 QRegularExpression 执行，成品无需 Python/Pygments 运行时。
  保留规则生成脚本、版本、规则摘要与 Pygments LICENSE/AUTHORS；部署同步复制许可文件。
- 补齐 f-string conversion、debug expression、嵌套 format 字段、lambda/dict/slice；
  CSS selector、at-rule、函数、未知属性、变量及 url；Shell 参数、算术、命令展开、
  引号与 heredoc；JS/TS 正则与除法、嵌套模板。HTML/Markdown 的内嵌语言也使用这些规则。
- 更正第三十八批未验证的 JSX 假设：参考的 JavaScript/TypeScript lexer 没有 JSX 专用
  标签高亮；加入 JSX 输入差分用例，忠实保留参考分类，不宣称新增 JSX lexer。
- 新增 23 组 Python 实际输出的逐 UTF-16 字符差分数据，跨行不比较换行符本身；
  增加五种语言异步应用、亮暗 QTextLayout 格式与截图检查。保留原有整行背景连续性、
  Qt 5/6 左右绘制边界、RTL、缩放、撤销、任务寿命与超限保护回归。
- 首轮完整检查发现 Qt 5 旧材质测试在固定等待 50ms 后未观察到 30ms 工具提示定时器结果；
  Qt 5 的 2× 同组通过。将测试改为最多 1 秒等待实际可见状态，保留显示/隐藏与寿命断言，
  不修改组件行为；最终两套完整检查错开执行，首轮失败日志保留为
  `build-qt515/code-lexer-release-first-check.log`。
- Qt 6 首轮原生 2× 菜单截图组也出现选中行像素异常；单独复查时该深色 RTL 用例通过，
  另一个用例菜单提前隐藏。保留菜单可见性、图标内缩与两端 hover 像素断言，
  首轮报告保留为 `build/code-lexer-release-first-check.log`，最终以串行完整复查为准。
- Gallery 文本页新增“加载复杂示例”按钮，仅在上述五种语言可用，切换语言保留正文；
  增加两条中文翻译，Gallery 翻译共 877 条，0 未完成。
- 其他 15 种语言仍使用第三十八批词法器，完整 Pygments 全语法差分未完成；
  不将 CodeEdit 全部语言列为完全验收。最终检查与原目录成品校验结果见本节后续记录。
- 最终验证：Qt 5 完整 Release 检查 **32 组、1283 项通过，0 失败、0 跳过**。
  Qt 6 CodeEdit 定向检查通过，1×/2× 各 **62 项**，Windows 渲染 **6 项**；
  Qt 6 最近完整检查 **32 组、1281 项通过、2 项失败、0 跳过**，未通过项仍为
  原生菜单 100%/200% 截图组，不将这次完整检查标为通过。
  原生鼠标 hover 测试实验未验收，已回退；保留原有菜单测试和失败日志。
- 用户本轮明确要求“没问题了，直接编译成品吧”后，两套 Gallery Release 单独构建均成功，
  固定原目录 EXE 已更新；附带 Pygments 许可文件，SHA256 与对应构建 EXE 一致。
  未自动启动 Gallery，未提交/推送 Git。
  - Qt 6：`932BA83A23ACA432A959CDB5B4E914803439CE5773399F99D09B054780986D91`。
  - Qt 5：`3B94425D12F62353A95B8613D35151F0DB66967096ED82F99A6490D0314B2A7D`。
  - 构建日志：两套构建目录 `code-lexer-product-build.log`。
  - 完整检查日志：两套构建目录 `code-lexer-release-check.log`，首轮及 Qt 6 第二轮日志另行保留。

## 第四十批：CodeEdit 追加十二种语言参考规则（2026-10-10）

- 在第三十九批原生规则引擎上继续迁移 C、C++、C#、Java、HTML、XML、TOML、INI、
  PowerShell、SQL、Go、Rust；已接入参考表的语言由 5 种增加至 17 种，不增加 Python 运行时依赖。
- 保留 C/C++ 参考 lexer 默认启用的标准库、C99、C11 和平台类型后处理，只有原始 Name
  token 参与类型匹配，不把函数、成员等同色但不同原始类型的 token 错当类型。
  嵌套扫描支持指定初始状态栈；HTML 的 script/style 按实际参考分别委托 JS/CSS lexer。
- 修复单独 `#push` 状态切换，保留 Rust 嵌套注释；合并相邻同色片段，减少 PowerShell
  逐字符 here-string 的格式数量，逐 UTF-16 字符颜色保持参考结果。
- 发现 Qt 5 旧正则引擎会把 Python `{,n}` 视为字面文本而不报表达式错误，C++ raw string
  因而错误分类；统一转换为等价 `{0,n}`，也覆盖 Python 文档字符串的同类规则。
  加入空 delimiter、16 字符合法边界及 17 字符超长 delimiter 的差分回归。
- 更正先前 C# 插值表达式单独着色的假设：Pygments CSharpLexer 将插值整体分类为 String，
  现按实际参考保留统一字符串颜色。C#、Java、Go、Rust 等声明/成员/属性分类均来自原版规则。
- 差分数据增至 60 组，覆盖 17 种语言；异步应用与亮暗 QTextLayout 检查也扩展到 17 种。
  Gallery 复杂示例增至 17 种并更新中文说明；Qt 5/6 均保留已有字体、边距、行号、整行背景、
  RTL、缩放、缩进撤销、后台任务寿命与超限保护回归。
- JSON、YAML、Markdown 尚未完成原版专用 lexer 的迁移。Markdown 参考代码块 token 的
  相对偏移存在已观察到的缺陷，需要后续单独决定并记录兼容行为，不能宣称全 20 语言等价。
  本地两套 Qt SDK 当前均无 WebEngine 模块，图表迁移依赖尚未具备；不使用简单原生图表替代。
- Qt 5.15.2 与 Qt 6.7.3 最终完整 Release 检查各 **32 组、1381 项通过，0 失败、0 跳过**。
  CodeEdit 在 1×/2× 下各 **111 项通过**，Windows 原生渲染各 **6 项通过**；
  包含全部 60 组差分、17 语言异步颜色应用、raw string 边界和已有编辑/背景回归。
  本批原有菜单 100%/200% 截图组均通过，保留第三十九批历史失败记录，不覆盖历史日志。
  日志：`build/code-lexer17-release-check.log`、`build-qt515/code-lexer17-release-check.log`。
  截图：两套构建目录 `tests/code-lexer-*`、`tests/code-edit-*`；已复查复杂示例和整行背景。
- 两套固定原目录 Release 成品已更新；EXE 与对应构建文件 SHA256 一致，附带的
  Pygments LICENSE/AUTHORS 也与源文件哈希一致。未自动启动 Gallery，未提交/推送 Git。
  - Qt 6：`6177F53E5DA6E505BBCC151CE71B65DF433917EE195731E4A356AB913161CD97`。
  - Qt 5：`682A46F977AE671437A6B48D78FC7FFDA03B1F0430DD7F0969614EB5EB481762`。

## 第四十一批：CodeEdit 全二十种语言与编辑边界完善（2026-10-10）

- 继续第四十批，补齐 JSON、YAML、Markdown 专用 lexer；移除旧的简化 Scanner，
  公开的 20 种语言均使用原版 Pygments 2.19.2 的规则、回调或专用状态机，运行时无 Python 依赖。
- JSON 保留延迟队列判定键名，跨空白/单行和多行注释后遇到冒号才改为 Tag；
  未完成字符串、非法转义、数字/常量与未完成注释按实际参考分类，不用 JSON 解析器代替词法器。
- YAML 保留缩进栈、显式/隐式缩进、文档指令、流式集合、引号值、普通值跨行及
  literal/folded scalar 的缩进探测和退栈重扫；不把所有值统一当作字符串。
- Markdown 保留标题、列表、链接、行内代码和围栏的原版规则；围栏委托全部 20 种语言
  和它们的参考别名。修正原版 `_handle_codeblock` 中已标注 FIXME 的代码相对偏移，
  改为全文绝对偏移，防止代码高亮覆盖前面正文。差分导出仅对该回调偏移作明确归一化。
  公开 20 种以外的围栏语言采用 String fallback，不引入 Python 的额外几百种 lexer；
  此范围差异明确保留，不宣称所有 Pygments lexer 或任意语法输入的形式等价。
- 无效 CodeLanguage 枚举安全抛出 invalid_argument，保留已有语言/正文；通过
  QPlainTextEdit 基类接口或属性切换只读状态时同步更新当前行背景，字体变化同步制表距离/行号宽度。
- 差分增至 133 组，含三种新语言的未完成输入、非法缩进、转义边界以及所有 20 种
  Markdown 围栏；异步格式和亮暗截图检查覆盖全部 20 种。新增编辑重新分类、基类只读、
  字体/行号、制表对齐、混合缩进反缩进与撤销回归，保留整行背景连续性、RTL、缩放和任务寿命回归。
- Gallery 复杂示例覆盖全部 20 种，JSON 默认也可加载复杂示例；中文说明同步更新，
  Gallery 翻译 877 条，0 未完成。参考规则头文件与差分文件均验证可重现导出。
- Qt 5.15.2 与 Qt 6.7.3 最终完整 Release 检查各 **32 组、1539 项通过，0 失败、0 跳过**。
  CodeEdit 1×/2× 各 **190 项通过**，Windows 原生渲染各 **6 项通过**；原有菜单截图组也通过。
  日志：`build/code-complete-release-check.log`、`build-qt515/code-complete-release-check.log`。
  Qt 6 定向日志为 `build/code-complete-target-test.log`；亮暗截图在两套构建目录 `tests/code-lexer-*`。
- 两套固定原目录 Release 成品已更新；EXE 与对应构建文件 SHA256 一致，附带的
  Pygments LICENSE/AUTHORS 哈希也一致。未自动启动 Gallery，未提交/推送 Git，保留现有工作区改动。
  - Qt 6：`E1D02BBA617FB4D0EDDDCC137F089B9B4BAAD0F25A367B31224D0A1482D98FD2`。
  - Qt 5：`55BFD1163ACF84EF829CA76E11FE5FA49C5F4061723777C85F355125EF75E3A2`。

## 第四十二批：ChatWidget 原生聊天主体与 Gallery（2026-10-10）

- 在第四十一批 CodeEdit 上实现 ChatWidget、ChatMessage、splitMessage、ChatMarkdownBlock、
  ChatCodeBlock。消息快照按插入顺序返回，保留调用方原始内容/稳定 ID；支持 user/assistant/system、
  图片路径/QImage/QPixmap、显式 QWidget 卡片和头像；不连接网络或模型后端。
- 忠实移植 14px 字体、18px 头像半径、历史区 24px 边距/间距、气泡 14/10px 内边距/12px 圆角、
  680px 最大宽度和 76% 宽度约束、姓名/状态间距、亮暗气泡/代码块色。保留 6px 原生可拖动分隔条、
  最小历史/输入高度、隐藏输入区恢复比例、底部跟随与 32px 判断阈值、不抢回上滚位置、36px 圆形跳转按钮。
- 流式正文每 40ms 合并；相同 kind/language 内容块复用，完成围栏保留原 CodeEdit 实例。
  文本状态与 messageChanged 信号立即更新，flush 后应用内容；移除/清空清理待处理 ID。
  clear 只清历史，保留草稿/自定义输入工具；Busy/Stop/发送只管理界面与信号，不自动添加消息或停止后端。
- 移植默认六类消息操作、角色/kind 过滤、可增删工具定义、左右输入工具栏、图片/代码/消息复制；
  消息操作默认关闭。流式、主题和工具栏可见性切换保留调用方按钮的 checkable/enabled 状态；
  配置改变才重建按钮，延迟点击守卫已移除动作/消息，回调删除聊天组件时不继续解引用。
- Markdown 使用 GitHub 方言与 MarkdownNoHTML；loadResource 拒绝隐式本地/网络资源，链接只发信号。
  围栏按原函数支持 backtick/tilde、3 空格缩进、匹配字符/长度、CRLF 和未完成流式围栏。
  已知语言使用现有 CodeEdit，未知语言保留只读原文；代码区字体、复制、横向滚动与 360px 高度上限按参考。
- 公式回调(expression, QColor, pointSize) → QImage、行内代码排除、DPR 尺寸、display 居中、
  无效/异常回调警告及保留原文已实现。**默认 Matplotlib MathText 排版引擎仍未迁移**：
  无回调时保留公式源码并发 renderWarning；不把普通 QLabel、预制公式图或缺字布局伪装为默认完整排版。
  成品无 Python/Matplotlib/WebEngine 运行时依赖。此缺口单独保留，不能将聊天列为全功能完全等价。
- 修复直接影响聊天的 AvatarWidget 依赖缺陷：C++ 基类构造期间虚调用不会执行派生 postInit，
  默认背景色未初始化使文字头像成为黑色圆。派生构造显式初始化后恢复参考的亮暗透明背景，
  加入头像与跳转按钮不透底的像素回归，不复制 Python setBackgroundColor 两色误用的原有缺陷。
- Gallery 新增聊天页/首页样例入口，两种主导航均注册；提供图片选择、可停止本地流式演示、
  重置/清空、动作信号、主题切换及只在点击后打开的独立 FluentWidget 窗口。隐藏演示结束模拟输出。
  默认示例不冒充公式引擎已完成。Gallery 中文翻译 912 条、库翻译 208 条，均 0 未完成。
- 对 Python 实际 splitMessage 生成 56 组差分（含逐字符流式前缀）；回归覆盖代码块/按钮复用、
  IME 预编辑回车、快捷键、清空保留草稿、图像/卡片、动作过滤与回调、公式注入/回退、Markdown 安全、
  滚动跟随、分隔比例、窄窗口换行、任务寿命及亮暗/RTL/1×/2×/Windows 原生截图。
  同内容 Python 原组件截图已用于视觉对照。Qt 5.15.2 / Qt 6.7.3 完整 Release 检查各
  35 组、1683 项通过，0 失败、0 跳过；聊天 1×/2× 各 69 项，Windows 渲染各 6 项通过。
  完整日志保留在 `build/chat-release-check.log` 与 `build-qt515/chat-release-check.log`。
- 两套固定原目录 Release 成品已更新，EXE 与对应构建文件 SHA256 一致；Pygments 许可文件
  同步并核对哈希。Qt 6：`34D3123E6F32814751F73635194C02696371501422E2571BFC28C8987FB2BDB4`；
  Qt 5：`FBBB54975103A30BED040C40909DED6EEC4D732A4573103ACA86879930D8BF7E`。
  没有自动启动 Gallery，没有提交或推送 Git。

## 第四十三批：聊天默认公式的原生排版（2026-10-10）

- 读取 Pro `common/chat_math.py` 与 Matplotlib 3.9.4 `_mathtext.py`，接入默认 `renderFormula`；
  没有自定义回调时也可渲染公式。保留调用方回调优先级、关闭数学渲染、警告/原文回退、
  Markdown 行内代码排除、display 居中及固定 DPR 2 的契约，无 Python/Matplotlib/TeX 子进程依赖。
- 内嵌原版 DejaVu、STIX 与 prime 使用的 Computer Modern 字体，通过 QRawFont 绘制实际字形；
  从实际参考导出 16 套字体度量、符号分类、间距、字符对 kerning、大小分隔符备选表，
  由 C++ 解析并组合字形路径，不使用预制公式图片或用普通文本冒充结构排版。
- 支持分式/dfrac/binom/genfrac、单/双上下标/prime、求和/连乘上下限、积分、函数与 operatorname、
  根号/根指数、left/right/middle 大小分隔符、重音/宽重音/overline、overset/underset/substack、
  字体组/boldsymbol/text 和显式间距；分组上下标使用末尾字形度量，函数脚标沿用参考的操作符间距状态。
- 默认渲染限制 4096 个 Unicode 字符；保护递归深度和栅格尺寸，缓存最多 256 条且有内存预算。
  返回值采用隐式共享，调用方修改 QImage 不会污染缓存；亮暗色与 pointSize 区分缓存键。
  未支持的命令、未导出字形或不完整表达式仍保留原始公式并发出警告，不隐藏消息内容。
- Gallery 聊天页增加“渲染公式”开关与行内/独立公式示例，独立聊天窗口同步可用；
  Gallery 中文翻译 914 条、0 未完成。Matplotlib、DejaVu、STIX、BaKoMa 许可及变更说明
  随资源内嵌，并加入固定成品许可目录的部署流程。
- 从 Python Pro 实际 `renderFormula` 导出 68 组公式/错误结果及成功 PNG；回归对照物理尺寸、
  双向字形覆盖及 alpha 面积，覆盖 kerning、分组脚标、缓存隔离、颜色/字号、非法输入回退，
  并增加亮暗/RTL/1×/2×/Windows 原生聊天公式截图。Qt 的矢量抗锯齿与 Agg 的栅格偏移
  存在细微差别；当前尺寸容差 3 个物理像素，字形匹配允许 2 个物理像素偏移。
  双向笔画覆盖至少 75%，alpha 覆盖面积偏差不超过 20%；16 套度量表、规则与 kerning
  数据均验证可重现导出，18 个字体文件与原版 SHA256 一致。
  **不宣称有限用例构成任意 MathText 输入的形式等价**；导出表覆盖 ASCII 与原版命名符号，
  其他 Unicode 字形及极深嵌套边界仍需继续核对，未支持时保留源码。
- Qt 5.15.2 / Qt 6.7.3 最终完整 Release 检查各 **35 组、1833 项通过，0 失败、0 跳过**；
  聊天 1×/2× 各 142 项、Windows 原生渲染各 10 项通过，CodeEdit 与原菜单截图组也通过。
  最终日志为 `build/chat-math-release-check-final.log` 与
  `build-qt515/chat-math-release-check-verified.log`。前期定向失败日志保留，不冒充最终通过。
  Qt 5 最终候选完整检查有 1 项原生菜单 2× 暗色 RTL 可见性断言失败（1832 通过、1 失败、0 跳过），
  原日志为 `build-qt515/chat-math-release-check-final.log`；未改测试，单独复测 7 项通过，
  日志为 `build-qt515/chat-math-menu-isolated.log`，之后重新执行完整检查。
- 两套固定原目录 Release 成品已更新；EXE 与对应构建文件 SHA256 一致，Matplotlib/字体
  许可与变更说明已复制并核对哈希，既有 Pygments 许可文件哈希保持一致。
  Qt 6：`F7E0D79424887BFC4AAAFB9447587BA4BB55615F660DA51350F37B4A3F6209C1`；
  Qt 5：`60AB2858601B5CDD4B43A58341BDD97E8B060CADC4B2CDB3080EF70104EE86AB`。
  未发现需要终止的精确路径 Gallery 进程；未自动启动 Gallery，未提交或推送 Git。

## 第四十四批：聊天公式 Unicode 字形、原版字距与嵌套回归

- 继续审计第四十三批的原生公式实现，读取实际 Matplotlib 3.9.4 字体回退与虚拟字体规则。
  补齐 10 种样式的直接 Unicode 字形：48291 条映射共享 24345 条度量记录，覆盖原字体
  支持的 U+0080～U+1FFFF 范围，包括带重音字母、俄文、希伯来文和补充平面数学字母。
  新增原版 STIXNonUniBol.ttf；19 个字体文件与 Python 原版 SHA256 全部一致。
  字形与排版仍为 C++ / QRawFont，不依赖运行时 Python 或预渲染公式图片。
- 导出 5887 条原版 Unicode/命名符号字距调整，沿用实际 FT get_kerning 参数。
  普通 Qt 文本字距会改变原公式尺寸，初次尝试的 5 个尺寸失败保留于
  `build/chat-math-unicode-first.txt`；修正后原有用例全部通过，不降低既有视觉阈值。
  Unicode 字距从资源初始化时合并，避免渲染顺序影响结果；缺字保持源码并发出警告，
  不采用 Matplotlib 的货币占位字形。孤立 UTF-16 代理码显式拒绝。
- Gallery 聊天页与独立窗口加入“Unicode 字符公式”示例，中文翻译 915 条、0 未完成。
  Python Pro 实际公式参考扩展至 102 组（93 个成功 PNG、9 个错误），包括字体组、
  分式/根号/脚标中的 Unicode，以及 1～8 层嵌套分式与上标。新增聊天源码回退、
  亮暗颜色、透明背景与无效代理码检查；原生主题/RTL 截图同步包含 Unicode。
  继续使用第四十三批的尺寸/笔画/alpha 容差。**1～8 层用例通过不代表任意深度等价**：
  原版 Node 在第六个缩小层停止缩字号，当前整体 Box 缩放尚未逐节点还原该极深边界。
- Qt 5.15.2 / Qt 6.7.3 最终完整 Release 检查各 **35 组、1903 项通过，0 失败、0 跳过**。
  聊天 1×/2× 各 177 项、Windows 原生绘制各 10 项通过；Qt 5 另有公式定向 109 项通过。
  已查看 Qt 6 Windows 亮色 LTR 1.75× 与 Qt 5 暗色 RTL 2× 的 Unicode 聊天截图。
  最终日志为 `build/chat-unicode-release-check-final.log` 与
  `build-qt515/chat-unicode-release-check-final.log`；Qt 6 第一轮完整检查也通过，
  随后将最后的代理码校验同步到所有旧链接目标并完整复查。原定向失败日志继续保留。
- 两套固定原目录 Release 成品已更新，EXE 与对应构建文件 SHA256 一致；每套 7 个
  Matplotlib/字体与 Pygments 许可/说明文件已同步并逐个核对哈希。
  Qt 6：`7E3A5F35147CBF3C5ABD3D25CD58811AFA85CC1F339791BAD93D0BDF6911C5A7`；
  Qt 5：`9A3C0A270DDB6A7B1DF0CE9F4619C1D09DADE1D9319732246249922A27D63C59`。
  无精确路径 Gallery 进程需要终止；未自动启动 Gallery，未提交或推送 Git。

## 第四十五批：聊天组件深层公式与生命周期完善

- 重新逐项核对 Python `ChatWidget`、`ChatMessage`、`chat_content` 与实际
  Matplotlib 3.9.4 排版。公式保留水平/垂直/居中节点、各节点缩小次数和打包尺寸，
  补齐第六次停止缩小的边界；分式、根号、脚标、上下标注、重音和 substack 均按节点出图。
  根号改为重新量取字形，分开 72dpi 尺寸与 192dpi 输出位置；补入原 Agg 的 103 种
  可变尺寸字形度量，运行时仍使用 C++ / QRawFont 绘制，无 Python/Matplotlib 调用。
- 字体组保留独立的 rm/it/bf/bfit 状态；新增 406 条共享度量、8956 条字体状态差异、
  8800 条对应字距，以及 2270 条命名符号字距。粗体数字、运算符和希腊符号按原版选字体，
  不再将整组强制为粗斜体。资源可由 `export_chat_math.py` 的对应参数重新导出。
- 实际 Python 公式对照增至 **170 组，156 个成功 PNG、14 个错误**，覆盖 16 层
  分式/上标/根号/重音、混合 substack、嵌套字体组和 6～48 pt 字号。
  其中 2 个深层 substack 在 Python 中触发 RecursionError，C++ 在 64 层边界内正常
  渲染，作为明确的运行时差异记录；另外 12 个原版错误保持源码回退。
  保持既有尺寸、双向笔画和 alpha 阈值，不宣称有限语料证明任意 MathText/TeX 等价。
- 消息注册后再调用公式渲染器；回调期间清空、删除父对象或再次更新内容时停止旧渲染。
  警告延后到文档更新完成后发出。动作/工具回调异常转为可翻译警告；复制代码时也保护
  剪贴板监听者删除控件的情形。清空采用对象快照，允许移除信号重入和新消息使用旧 ID。
- 工作线程公式缓存、字体与轮廓独立，避免跨线程复用 QRawFont。通过 queued connection
  更新或结束已清除的回复会被忽略，结束时若回调已移除该行，也不发出过期的变更信号。
  直接访问未知 ID 仍报告错误；UI 方法仍须在 GUI 线程调用。
  跨会话流式回复应使用新消息 ID，不能将已移除 ID 复用于仍有旧回调的另一条回复。
- DPR=2 图片按逻辑尺寸布局，缩略图按目标屏幕 DPR 生成，复制保留原图分辨率。
  代码流式更新保留横纵滚动和选区，并保护 textChanged 重入。公式不再误识别多反引号
  代码片段里的字面量；这是对原 Python 简单正则行为的修正。自定义公式图像和默认
  渲染都限制到 8192 物理像素边长、16 MP，避免按点数误放大分配。
- Gallery 聊天页与独立窗口加入深层公式/字体示例、混合内容流式按钮、输入区开关、
  Ctrl+Enter 切换；流式演示包含完整公式结束标记。应用中文翻译 922 条、库 212 条，
  均 0 未完成。新增线程、生命周期、重入、排队取消、图片 DPR、代码选区和代码字面量回归。
- 最终源码 Qt 5.15.2 / Qt 6.7.3 完整 Release 检查各 **35 组、2053 项通过，
  0 失败、0 跳过**；聊天 1×/2× 各 252 项、Windows 原生渲染各 10 项通过。
  最终日志为 `build/chat-complete-release-check-verified.log` 与
  `build-qt515/chat-complete-release-check-final.log`。Qt 6 的
  `chat-complete-release-check-final.log` 为补入结束回复回调保护前的中间检查，
  历史失败与中间日志保留，最终结果不覆盖历史运行。
- 已查看 Qt 6 亮色及 Qt 5 暗色 RTL 原生公式截图，确认深层根号和字体组显示。
  两套固定原目录 EXE 已更新，分别与构建文件 SHA256 一致；Pygments 及 Matplotlib
  共 7 份许可/说明文件在每套成品中均同步并核对 SHA256。
  Qt 6：`1859E011E238D414B1E671E69EE4EA009A9A64A35F8F0B119E80F212EEA0D617`；
  Qt 5：`281784B9B69CCB54D59BF8B1FD2C37A99DEA3A492054A28779623030C34B2A4E`。
  未自动启动 Gallery，未提交或推送 Git，保留所有已有工作区改动。

## 第四十六批：ECharts 图表组件与 Gallery

- 对照 Python `chart_widget.py`、`chart.html`、图表窗口及实时页逐项实现 `ChartWidget`。
  保留 400×300 默认尺寸、15 px 内边距、亮色 `#f3f3f3` / 暗色 `#202020`、
  hover/按下不变色；使用真正的 Qt WebEngine 与原版 ECharts 5.6.0，不以原生图表替代。
- 补装与 SDK 精确对应的 Qt 6.7.3 WebEngine/WebChannel/Positioning、Qt 5.15.2 WebEngine。
  Qt 6 定位插件的 SerialPort 依赖也已补装并部署，最终部署无依赖缺失警告。
  图表独立链接 `qtfluentwidgets_chart`，不加入总头文件，与 Python 显式子模块导入一致。
  `QFW_BUILD_CHART=OFF` 可关闭图表及其 Gallery；基础 `qtfluentwidgets` 不链接 WebEngine。
  宿主须在 QApplication 前启用 `Qt::AA_ShareOpenGLContexts`，再创建图表后显示窗口；
  Gallery 与测试入口均已配置。Qt 5 离屏测试另显式初始化 COM STA。
- `setOption(QJsonObject)` / `setOptionJS(QString)` 保留完整选项替换、JS 函数和数据生成器；
  同轮更新合并、隐藏期间保存最新选项、禁用动画、清空及 resize() 均复用已有页面。
  再次进入只重播动画、不重新加载或执行 JS 数据生成器，主题切换才重建 ECharts 实例。
  JS 选项与原版一样执行可信应用代码，不能直接接入不可信输入。
  工具箱保存图片沿用 WebEngine 下载机制；应用须连接 profile 的 downloadRequested，
  Python 原版亦未提供默认下载处理器。可通过 browser()->page()->profile() 接入。
- 重新显示时先盖住旧纹理，确认真实 Qt 呈现缓冲中的私有标记帧，再确认背景帧后出图。
  没有按固定延时揭盖，没有通过 JS/rAF 回执假定 Qt 已显示。Qt 6 读取 QQuickWidget
  帧缓冲，Qt 5 读取 QWidget 呈现缓冲；离屏平台不呈现 GPU 纹理，另设 Windows 原生帧测试。
  原 HTML 的 `inset:0` 改为等价四边定位以支持 Qt 5 旧 Chromium。
- HTML 和 ECharts 均随资源离线打包，WebEngine 共享无磁盘配置文件，最后一个页面销毁后
  释放；页面先于配置文件销毁。`loadFinished` 忽略初始化空页，脚本错误发出 renderWarning。
- Gallery 主导航与首页加入图表入口，独立窗口保留折线、柱状、饼图、散点、K 线、雷达、
  盒须、热力、关系、漏斗、仪表盘、日历 12 个原版数据/JS 示例及实时页。
  独立窗口采用原版逐页图标、透明背景及直接放入布局的图表，不套重复示例卡片。
  实时页按原版 400×400、120 px 按钮、30 px 布局间距、100 ms 更新，支持动画开关、开始/停止；
  离开页面停止演示定时器。应用中文翻译 950 条、0 未完成。
- 新增真实 ECharts 懒加载、13 个隐藏控件、实例复用、JS 回调、隐藏更新、主题、尺寸、
  清空、异常、所有示例和 1×/2× 亮暗/RTL 回归；原生检查包含忙碌时切页、动画重播、
  无边框宿主、鼠标悬停及 tooltip，以及全部 12 类示例的原生 1×/2× 截图。
  ECharts 与 ZRender/d3 许可文件随成品分发。
- 最终 Qt 5.15.2 / Qt 6.7.3 完整 Release 检查各 **39 组、2137 项通过，
  0 失败、0 跳过**。图表离屏 1×/2× 各 22 项，原生 1×/2× 各 20 项通过；
  两套全部 12 类示例截图已查看，亮暗/RTL、鼠标与旧帧遮挡均完成验证。
  最终日志为 `build/chart-release-check-gallery-final.log` 与
  `build-qt515/chart-release-check-final.log`。Qt 6 `chart-release-check-final.log`、
  `chart-release-check-verified.log` 为 Gallery 最终间距或原生示例截图补齐前的中间检查。
- 首次构建的导航 QVariant 适配、测试导入目标作用域、Qt 5 离屏 COM 初始化、
  页面加载前探测 JS 错误、Qt 5 `inset` 遮罩及 tooltip 计算样式问题均已修复。
  原失败与中间日志保留，未降低帧判据或将编译成功计为测试通过。
- 两套固定成品已部署 WebEngine DLL、QtWebEngineProcess、PAK/ICU/V8 资源和语言包。
  各在成品目录使用随包依赖运行额外 **9 项**绘制、交互、懒加载与寿命探测，全部通过；
  临时测试 EXE / QtTest DLL 已移除，未启动 Gallery。WebEngine 核心、控件及进程文件
  SHA256 与对应 SDK 一致；每套 12 份第三方许可/说明文件同步并核对哈希。
  成品 EXE 与最终构建文件 SHA256 一致：
  Qt 6：`2BCC2BB8AA1C1DED8100AE28A50608257CD00F378F7C00174F39000B0BBDFC1C`；
  Qt 5：`6839839603AB6C1AC94057A415B46BF9C34EB9CB9A25E578A51CDB0CB9749F9E`。
  未提交或推送 Git，保留所有既有改动，包括原先删除的 issue_13_interface 文件。

## 第四十七批：WebEngine 宿主背景恢复

- 用户实机截图确认第四十六批遗漏了 WebEngine 的宿主适配：普通浏览器会使 Win11
  云母背景、圆角和阴影失效。原图表控件截图与交互通过并不足以证明 DWM 合成正确。
- 对照实际 `qframelesswindow/webengine/__init__.py` 添加独立
  `FramelessWebEngineView`，在浏览器构造前设置无边框宿主透明属性，初始化空白页面，
  并调用公开 `updateFrameless()` 刷新 Windows 宿主样式、动画与阴影。
  保留 C++ 的原生 NCCALCSIZE 实现，不在已显示窗口上重设 Qt flags 或重建 HWND。
- Gallery 主窗口和图表独立窗口在 `show()` 后恢复当前云母开关状态。
  已启用云母时使用 `refreshMicaEffect()`，避免重复 FRAMECHANGED 干扰 Chromium
  鼠标追踪；首次启用仍执行完整设置。QOpenGLWidget 可使用同一 Windows 宿主
  `updateFrameless()` 和显示后的云母恢复接口。
- 新增实际桌面合成回归，比较无图表与含图表宿主的标题背景，覆盖亮暗主题、侧栏/
  顶部导航、首页中的隐藏图表、隐藏后再显示、1×/2×，同时验证 DWM 背景、圆角策略、
  非客户区渲染和非分层窗口样式。测试窗口置顶避免其他应用遮挡，仅测试窗口参与捕获。
  两套 Qt 定向检查各尺度均为 7 项通过，包含真实鼠标 hover/tooltip。
  首轮未置顶时截到了其他应用，背景基准与鼠标检查失败；已纠正测试窗口遮挡条件，
  保留 `build/mica-focused-1.txt`、`build/mica-focused-fixed-1.txt`，未降低背景比较判据。
  两套 Qt 全部 48 张桌面合成截图已通过四张缩略对照图及代表原图核对。
- 2026-10-11，Qt 5.15.2 / Qt 6.7.3 最终完整 Release 检查各 **39 组、2145 项通过，
  0 失败、0 跳过**。图表离屏 1×/2× 各 22 项，原生 1×/2× 各 24 项通过。
  完整日志：`build/mica-webengine-release-check.log` 与
  `build-qt515/mica-webengine-release-check.log`。
- 两套原目录成品已更新；部署日志无缺失依赖警告。使用随包 DLL / WebEngine 资源
  各额外运行 **13 项**背景、绘制、鼠标、懒加载和生命周期检查，全部通过；
  日志为对应构建目录的 `mica-package-probe.txt`，临时探测 EXE / QtTest DLL 已移除。
  每套 12 份许可/说明文件已同步并核对 SHA256。
- 成品 EXE 与最终构建文件 SHA256 一致：
  Qt 6：`85B7D725302B9A7F46978AC06F6A4CE0E5C210A097E7C2F220A9486DCFA1A626`；
  Qt 5：`99F2329557F98E9191BD4726A0E2D06EAA56D7CAF6DF4715CE6F29E62466BB88`。
  未自动启动 Gallery，未提交或推送 Git；保留全部既有改动和删除状态。

## 第四十八批：FilledFluentWindow

- 用户指出整组件遗漏；逐行核对本地 Python `filled_fluent_window.py` 与
  `page_search.py`，补齐 `window/filled_fluent_window.h/.cpp`，加入总头文件和 CMake。
- 默认展开 216 px 侧栏；导航项宽 192 px、间距 6 px、圆角 4 px，以主题色填充
  整行选中项。还原亮暗主题下的白/黑图标和文字、hover 提亮、按下及禁用透明度、
  嵌套条目与箭头旋转；宽度为面板内的实例状态，不影响普通导航控件。
- 搜索框高 32 px，沿用 Fluent 搜索编辑器。弹层为不激活的工具窗口，带阴影和
  8 px 卡片圆角；按注册页面名称进行大小写折叠、多词匹配和稳定排序。
  精确匹配优先，其次前缀匹配；禁用项和非页面操作项不会成为搜索结果。
  上下方向键移动，Enter 跳转并发送一次 `pageSearchActivated(QWidget*)`；
  Esc/Tab/外部点击关闭弹层并保留查询，清空输入关闭弹层；支持空结果、动态增删、
  主题切换保留选择、窗口移动/调整大小时定位、隐藏和销毁时清理。
- 标题栏覆盖完整窗口宽度，左边距 24 px；去除 Python 没有的 6 px 标题间距。
  保留既有路由、堆叠页面动画、云母和原生窗口恢复接口。
- 修复现有面板在不可折叠时仍随窗口变窄而收起的问题；填充式面板独立补齐透明
  1 px 边框，使搜索框 190×32 px、单结果弹层 210×84 px 与实际 Python 一致。
  首轮绘制核对发现替换代理后列表仍更新旧代理，已改为绑定实际使用的代理，
  并新增结果选中条的像素断言；没有用简单原生列表取代 Fluent 绘制。
- Gallery 导航页新增“填充式导航与页面搜索”示例，点击按钮才打开独立窗口；
  同步中文翻译。网站中英文示例和缺失标记已更新，按用户要求沿用 Python 展示图。
- 新增 `filled_window_test.cpp`，覆盖窗口几何、面板宽度隔离、嵌套页面搜索、排名、
  键盘、空结果、动态增删、禁用过滤、弹层关闭和定位、生命周期、中文翻译，
  以及亮暗/RTL/1×/2× 绘制；离屏和 Windows 原生检查均纳入完整 Release 脚本。
- 2026-10-11，Qt 5.15.2 / Qt 6.7.3 完整 Release 检查各 **43 组、2183 项通过，
  0 失败、0 跳过**。日志：`build/filled-release-check.log` 与
  `build-qt515/filled-release-check.log`。Qt 6 在最后的标题间距和宽度边界修正后，
  又补跑四组 Filled 检查，共 38 项通过；日志：`build/filled-final-focused.log`。
  两套 SDK 的 16 对原生亮暗/RTL/1×/2× 截图尺寸一致，最大平均 RGB 差异为
  0.104/255；已核对代表原图与 Python 实际渲染，完整窗口截图采用非云母背景以便
  离屏采样，不能将 QWidget::grab 的透明像素当成桌面 DWM 合成截图。
- 两套原目录成品 EXE 已更新，SHA256 与对应构建文件一致。各在固定成品目录
  使用随包 DLL/插件，额外执行 1×/2× 原生检查，共 **22 项通过**；日志为对应
  构建目录的 `filled-package-probe-1.txt`、`filled-package-probe-2.txt`。
  临时探测 EXE / QtTest DLL 已清理；未启动 Gallery。
- 网站 23 项测试通过，新增后的 96 段中英文 C++ 示例分别通过 Qt 5/6 编译；
  146 页静态构建成功，原 `docs` 目录 891 个当前输出文件 SHA256 全部一致。
- 最终成品 SHA256：
  Qt 6：`4211382832EBFCFC38A54C70E0F03CE043EE37EA9D70FE6742203BC44FDD94DC`；
  Qt 5：`BDAFB30B76493C0F71E106FAADA13DBC2BA1D7C64A3C78922BBEFF777524B5D5`。
  未提交或推送 Git，保留原有改动与 issue_13 文件删除状态。

## 后续批次（尚未迁移）

第四十七批后，按 Python 各组件包的 `__init__.py` 导出核对 C++ 头文件、实现和
已有迁移批次，再检查按模块导入的聊天、图表与代码编辑器。以下为已确认的剩余项；
名称不同但有等价实现的 getter、`Toast.new` → `newToast` 等 C++ 命名适配不计为缺失。

### 已确认整组件缺失项：4

- 后续网站接口审计及本轮用户复核确认，之前的“0”不准确。
  `FilledFluentWindow` 已列入第四十八批补齐；当前仍缺少 `GuideWindow`、
  `VideoWidget`、`SimpleMediaPlayBar`、`StandardMediaPlayBar`。
  视频相关接口尚未迁移，不能以其他音频控件或同名文档示例代替实现。

- `components.material` 的上述剩余导出已于第三十七批补齐，材质系列不再列为整组件缺失。
  内部补全/下拉菜单也已实现；这不代表其他基础组件已全部完成逐项视觉与交互验收。
- `CodeEdit` / `CodeLanguage`：第四十一批补齐公开 20 种语言，已从整组件缺失清单移除；已验收范围和 Markdown 围栏范围差异见该批记录，不宣称有限差分等于任意输入的形式等价。
- `ChatWidget` / `ChatMessage` 及 `chat_content`：第四十二批已迁移主体与 Gallery，第四十三批
  接入原生默认公式排版，第四十四批补齐 Unicode 字形，第四十五批补齐逐节点缩小、
  深层根号、字体状态及消息生命周期。已确认聊天缺口已补齐；未支持字形/语法保留源码，
  有限参考验收不代表任意 MathText 输入的形式等价。
- `ChartWidget`：第四十六批已实现 ECharts 容器、主题、懒加载、入场动画、实时更新，
  并还原图表入口、12 类示例和实时页。两套 SDK 依赖与成品部署已补齐。

### 第二十九批记录的已有组件缺口

第三十批已补齐上轮列出的 PasswordLineEdit、SplitWidgetBase、分组卡片、PickerBase、
桌面 InfoBar、公开 ToastManager 和 AcrylicBrush 采样尺寸缺口；同时补齐
InfoBar/InfoBadge/Flyout/Menu 管理器工厂注册。具体行为、参考代码缺陷和 C++ API
适配见第三十批记录，不再将这些项目列入待迁移清单。

以上清单区分整组件缺失与已有实现的差异；基础库其余同名组件仍需按交互和视觉逐项验收，
尚未执行该验收的项目不能仅凭导出名称宣称与 Python Pro 完全一致。

后续组件应逐批加入 Gallery 并补充测试；不能将 Python 的全部扩展接口视为当前已在 C++ 中提供。
