
"Packing release..."

Add-Type -assembly "system.io.compression.filesystem"

if (-Not(Test-Path Release\TemplePlus.exe)) {
    Write-Error "Run this script after building TemplePlus. Release\TemplePlus.exe not found"
    Exit
}

if (-Not(Test-Path env:QTDIR)) {
    Write-Error "QTDIR environment variable is not defined"
    Exit
}

# Create a ZIP file for portable distribution
if (Test-Path dist) {
    Remove-Item -Recurse dist
}

mkdir dist
copy Release\TemplePlusConfig.exe dist
copy Release\TemplePlusConfig.exe.config dist
copy Release\Squirrel.dll dist
copy Release\Splat.dll dist
copy Release\NuGet.Squirrel.dll dist
copy Release\Mono.Cecil*.dll dist
copy Release\Microsoft.WindowsAPICodePack*.dll dist
copy Release\INIFileParser.dll dist
copy Release\ICSharpCode.SharpZipLib.dll dist
copy Release\FontAwesome.WPF.dll dist
copy Release\DeltaCompressionDotNet*.dll dist
copy Release\TemplePlus.exe dist
copy Release\TemplePlus.dll dist
copy -Recurse tpdata dist\tpdata
copy -Recurse dependencies\python-lib dist\tpdata\python-lib
copy dependencies\bin\d3dcompiler_47.dll dist
copy $env:VCINSTALLDIR\redist\x86\Microsoft.VC140.CRT\* dist

# Copy QT binaries
copy $env:QTDIR\bin\Qt5Core.dll dist
copy $env:QTDIR\bin\Qt5Gui.dll dist
copy $env:QTDIR\bin\Qt5Qml.dll dist
copy $env:QTDIR\bin\Qt5Quick.dll dist
copy $env:QTDIR\bin\Qt5Network.dll dist
mkdir dist\platforms
copy $env:QTDIR\plugins\platforms\qoffscreen.dll dist\platforms

# Copy QML libraries
mkdir dist\tpdata\qml\QtQuick.2
copy $env:QTDIR\qml\QtQuick.2\qtquick2plugin.dll dist\tpdata\qml\QtQuick.2
copy $env:QTDIR\qml\QtQuick.2\qmldir dist\tpdata\qml\QtQuick.2

if (Test-Path env:\APPVEYOR_BUILD_VERSION) {
    $distZipFile = "TemplePlus-$($env:APPVEYOR_BUILD_VERSION).zip"
} else {
    $distZipFile = "TemplePlus.zip"
}
$distZipFile = Join-Path (pwd) $distZipFile

if (Test-Path $distZipFile) {
    rm $distZipFile
}

"Compressing distribution archive to $distZipFile"
$srcDir = Join-Path (pwd) "dist"
[io.compression.zipfile]::CreateFromDirectory($srcDir, $distZipFile)

# Create the nuget package (with the right version number)
if (Test-Path env:\APPVEYOR_BUILD_VERSION) {
    .\.nuget\nuget.exe pack -Version $env:APPVEYOR_BUILD_VERSION TemplePlus.nuspec
} else {
    .\.nuget\nuget.exe pack TemplePlus.nuspec
}

"Finished packing the release."

