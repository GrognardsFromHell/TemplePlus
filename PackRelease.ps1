
"Packing release..."

Add-Type -assembly "system.io.compression.filesystem"

if (-Not(Test-Path Release\TemplePlus.exe)) {
    Write-Error "Run this script after building TemplePlus. Release\TemplePlus.exe not found"
    Exit
}

# Create a ZIP file for portable distribution
if (Test-Path dist) {
    Remove-Item -Recurse dist
}

mkdir dist
copy Release\TemplePlus.exe dist
copy -Recurse tpdata dist\tpdata
copy -Recurse dependencies\python-lib dist\tpdata\python-lib
copy dependencies\bin\d3dcompiler_47.dll dist

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
