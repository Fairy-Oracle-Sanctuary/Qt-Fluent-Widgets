[CmdletBinding()]
param(
    [string]$QtPrefix = '',
    [string]$BuildDirectory = 'build',
    [string]$PackageDirectory = '',
    [ValidateRange(1, 64)]
    [int]$Jobs = 6,
    [switch]$SkipTests,
    [switch]$SkipDeploy
)

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
$buildPath = if ([IO.Path]::IsPathRooted($BuildDirectory)) {
    [IO.Path]::GetFullPath($BuildDirectory)
} else {
    [IO.Path]::GetFullPath((Join-Path $repoRoot $BuildDirectory))
}

$cmakeCommand = Get-Command cmake -ErrorAction SilentlyContinue
$cmakePath = if ($cmakeCommand) { $cmakeCommand.Source } else {
    'D:\Qt\Tools\CMake_64\bin\cmake.exe'
}
if (-not (Test-Path -LiteralPath $cmakePath)) {
    throw 'CMake was not found. Add its bin directory to PATH first.'
}
$ctestPath = Join-Path (Split-Path -Parent $cmakePath) 'ctest.exe'

# Reuse the build tree's Qt SDK rather than silently replacing a Qt 5 cache
# with Qt 6 (or the reverse). A new build tree defaults to this machine's Qt 6.
$cachePath = Join-Path $buildPath 'CMakeCache.txt'
$cachedQtPrefix = ''
if (Test-Path -LiteralPath $cachePath) {
    $qtEntry = Get-Content -LiteralPath $cachePath |
        Where-Object { $_ -match '^Qt[56]_DIR:PATH=' } | Select-Object -First 1
    if ($qtEntry) {
        $qtCmakePath = $qtEntry.Substring($qtEntry.IndexOf('=') + 1)
        $cachedQtPrefix = [IO.Path]::GetFullPath((Join-Path $qtCmakePath '../../..'))
    }
}
if (-not $QtPrefix) {
    $QtPrefix = if ($cachedQtPrefix) { $cachedQtPrefix } else { 'D:\Qt\6.7.3\msvc2019_64' }
}
$QtPrefix = [IO.Path]::GetFullPath($QtPrefix)
if (-not (Test-Path -LiteralPath (Join-Path $QtPrefix 'bin'))) {
    throw "Qt SDK was not found at '$QtPrefix'. Pass -QtPrefix with the SDK directory."
}
if ($cachedQtPrefix -and $cachedQtPrefix -ne $QtPrefix) {
    throw "This build directory uses '$cachedQtPrefix'. Select a different -BuildDirectory for '$QtPrefix'."
}

function Invoke-CheckedCommand {
    param([string]$Executable, [string[]]$Arguments)
    & $Executable @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "Command failed (exit $LASTEXITCODE): $Executable $($Arguments -join ' ')"
    }
}

$testOption = if ($SkipTests) { 'OFF' } else { 'ON' }
Write-Host "Release check: Qt=$QtPrefix; build=$buildPath"
$configureArgs = @('-S', $repoRoot, '-B', $buildPath,
    "-DCMAKE_PREFIX_PATH=$QtPrefix", '-DCMAKE_BUILD_TYPE=Release', "-DQFW_BUILD_TESTS=$testOption")
Invoke-CheckedCommand $cmakePath $configureArgs

$buildArgs = @('--build', $buildPath, '--config', 'Release', '--parallel', "$Jobs",
    '--target', 'qtfluentwidgets_app')
if (-not $SkipTests) { $buildArgs += @('qfw_extended_button_tests', 'qfw_container_tests', 'qfw_input_tests', 'qfw_navigation_tests', 'qfw_progress_tests', 'qfw_picker_tests', 'qfw_tree_combo_tests', 'qfw_extended_combo_tests', 'qfw_extended_layout_tests', 'qfw_status_tests', 'qfw_color_picker_tests') }
if (-not $SkipTests) { $buildArgs += @('qfw_material_tests','qfw_startup_tests','qfw_code_edit_tests','qfw_chat_tests') }
if (-not $SkipTests) { $buildArgs += @('qfw_filled_window_tests') }
if (-not $SkipTests -and (Select-String -LiteralPath $cachePath -Pattern '^QFW_BUILD_CHART:BOOL=ON$' -Quiet)) { $buildArgs += @('qfw_chart_tests') }
Invoke-CheckedCommand $cmakePath $buildArgs

if (-not $SkipTests) {
    $previousTaskPath = $env:PATH
    $taskReportNames = @('extended-buttons-results.txt', 'container-results.txt', 'input-results.txt', 'navigation-results.txt', 'progress-results.txt', 'picker-results.txt', 'tree-combo-results.txt', 'extended-combo-results.txt', 'extended-layout-results.txt', 'status-results.txt', 'color-picker-results.txt')
    $taskReportNames += @('material-1-results.txt', 'material-2-results.txt')
    $taskReportNames += @('code-edit-1-results.txt', 'code-edit-2-results.txt')
    $taskReportNames += @('chat-1-results.txt','chat-2-results.txt')
    $taskReportNames += @('filled-window-1-results.txt','filled-window-2-results.txt')
    if (Select-String -LiteralPath $cachePath -Pattern '^QFW_BUILD_CHART:BOOL=ON$' -Quiet) {
        $taskReportNames += @('chart-1-results.txt','chart-2-results.txt')
        if ([Environment]::OSVersion.Platform -eq [PlatformID]::Win32NT) { $taskReportNames += @('chart-native-1-results.txt','chart-native-2-results.txt') }
    }
    foreach ($scale in @('Auto','1','1.25','1.5','1.75','2')) { $taskReportNames += "startup-$scale-results.txt" }
    if ([Environment]::OSVersion.Platform -eq [PlatformID]::Win32NT) {
        $taskReportNames += @('filled-window-native-1-results.txt','filled-window-native-2-results.txt')
        $taskReportNames += @('chat-native-results.txt')
          $taskReportNames += @('dwm-native-results.txt', 'color-picker-native-results.txt', 'material-native-results.txt', 'code-edit-native-results.txt')
        $taskReportNames += @('dwm-native-100-results.txt', 'dwm-native-200-results.txt')
        foreach ($scale in @(100, 125, 150, 175, 200)) {
            $taskReportNames += "status-native-$scale-results.txt"
        }
    }
    try {
        $env:PATH = (Join-Path $QtPrefix 'bin') + [IO.Path]::PathSeparator + $previousTaskPath
        Invoke-CheckedCommand $ctestPath @('--test-dir', $buildPath, '-C', 'Release',
            '--output-on-failure', '--no-tests=error')
        foreach ($reportName in $taskReportNames) {
            Get-Content -LiteralPath (Join-Path $buildPath "tests/$reportName") | Select-Object -Last 3
        }
    } catch {
        foreach ($reportName in $taskReportNames) {
            $testReport = Join-Path $buildPath "tests/$reportName"
            if (Test-Path -LiteralPath $testReport) { Get-Content -LiteralPath $testReport }
        }
        throw
    } finally {
        $env:PATH = $previousTaskPath
    }
}
if (-not $SkipDeploy) {
    $deployTool = Join-Path $QtPrefix 'bin/windeployqt.exe'
    if (-not (Test-Path -LiteralPath $deployTool)) {
        throw "windeployqt was not found: $deployTool"
    }
    $exeCandidates = @(
        (Join-Path $buildPath 'Release/qtfluentwidgets_app.exe'),
        (Join-Path $buildPath 'app/Release/qtfluentwidgets_app.exe'),
        (Join-Path $buildPath 'app/qtfluentwidgets_app.exe'),
        (Join-Path $buildPath 'qtfluentwidgets_app.exe')
    )
    $galleryExe = $exeCandidates | Where-Object { Test-Path -LiteralPath $_ } | Select-Object -First 1
    if (-not $galleryExe) { throw 'The compiled Gallery executable was not found.' }
    $packagePath = if (-not $PackageDirectory) {
        Join-Path $buildPath 'package/Release/Qt-Fluent-Widgets'
    } elseif ([IO.Path]::IsPathRooted($PackageDirectory)) {
        [IO.Path]::GetFullPath($PackageDirectory)
    } else {
        [IO.Path]::GetFullPath((Join-Path $repoRoot $PackageDirectory))
    }
    New-Item -ItemType Directory -Path $packagePath -Force | Out-Null
    $packageExe = Join-Path $packagePath 'qtfluentwidgets_app.exe'
    $runningGallery = Get-Process -Name qtfluentwidgets_app -ErrorAction SilentlyContinue |
        Where-Object { $_.Path -and [IO.Path]::GetFullPath($_.Path) -eq [IO.Path]::GetFullPath($packageExe) }
    if ($runningGallery) {
        throw "The packaged Gallery is running at '$packageExe'. Close it and rerun this script. No process was stopped and no alternate package directory was created."
    }
    Copy-Item -LiteralPath $galleryExe -Destination $packageExe -Force
    $pygmentsLicensePath = Join-Path $packagePath 'licenses/pygments'
    New-Item -ItemType Directory -Path $pygmentsLicensePath -Force | Out-Null
    foreach ($notice in @('LICENSE', 'AUTHORS')) {
        Copy-Item -LiteralPath (Join-Path $repoRoot "third_party/pygments/$notice") -Destination $pygmentsLicensePath -Force
    }
    $mathLicensePath = Join-Path $packagePath 'licenses/matplotlib_math'
    New-Item -ItemType Directory -Path $mathLicensePath -Force | Out-Null
    foreach ($notice in @('LICENSE', 'LICENSE_DEJAVU', 'LICENSE_STIX', 'LICENSE_BAKOMA', 'README.md')) {
        Copy-Item -LiteralPath (Join-Path $repoRoot "third_party/matplotlib_math/$notice") -Destination $mathLicensePath -Force
    }
    $previousTaskPath = $env:PATH
    $previousVcInstallDir = $env:VCINSTALLDIR
    if (Select-String -LiteralPath $cachePath -Pattern '^QFW_BUILD_CHART:BOOL=ON$' -Quiet) {
        $chartLicensePath = Join-Path $packagePath 'licenses/echarts'
        New-Item -ItemType Directory -Path $chartLicensePath -Force | Out-Null
        foreach ($notice in @('LICENSE','NOTICE','LICENSE-d3','LICENSE-zrender','README.md')) {
            Copy-Item -LiteralPath (Join-Path $repoRoot "third_party/echarts/$notice") -Destination $chartLicensePath -Force
        }
    }
    try {
        $env:PATH = (Join-Path $QtPrefix 'bin') + [IO.Path]::PathSeparator + $previousTaskPath
        $vswherePath = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
        if (-not $env:VCINSTALLDIR -and (Test-Path -LiteralPath $vswherePath)) {
            $vsInstallation = & $vswherePath -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
            if ($vsInstallation) { $env:VCINSTALLDIR = (Join-Path $vsInstallation 'VC') + '\' }
        }
        Invoke-CheckedCommand $deployTool @('--release', '--compiler-runtime', '--dir',
            $packagePath, $packageExe)
        # windeployqt may include only the redistributable installer. Also copy
        # the release CRT DLLs app-locally so the result can be inspected directly.
        if ($env:VCINSTALLDIR -and $QtPrefix -match 'msvc') {
            $redistRoot = Join-Path $env:VCINSTALLDIR 'Redist/MSVC'
            $redistVersion = Get-ChildItem -LiteralPath $redistRoot -Directory |
                Where-Object { $_.Name -match '^\d+\.\d+\.\d+$' } |
                Sort-Object { [version]$_.Name } -Descending | Select-Object -First 1
            if (-not $redistVersion) { throw 'The MSVC release redistributable directory was not found.' }
            $crtDirectories = Get-ChildItem -LiteralPath (Join-Path $redistVersion.FullName 'x64') -Directory |
                Where-Object { $_.Name -match '^Microsoft\.VC\d+\.CRT$' }
            if (-not $crtDirectories) { throw 'The x64 MSVC CRT DLL directory was not found.' }
            foreach ($crtDirectory in $crtDirectories) {
                Get-ChildItem -LiteralPath $crtDirectory.FullName -Filter '*.dll' |
                    Copy-Item -Destination $packagePath -Force
            }
        }
    } finally {
        $env:PATH = $previousTaskPath
        $env:VCINSTALLDIR = $previousVcInstallDir
    }
    Write-Host "Ready-to-run Gallery: $packageExe"
}
Write-Host 'Release check passed. No Gallery window was launched.'
