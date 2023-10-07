
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
copy -Recurse tpdata dist\tpdata
copy -Recurse dependencies\python-lib dist\tpdata\python-lib
copy "C:\Program Files (x86)\Windows Kits\10\Redist\D3D\x86\d3dcompiler_47.dll" dist

if (Test-Path env:\TEMPLEPLUS_VERSION) {
    $distZipFile = "TemplePlus-$($env:TEMPLEPLUS_VERSION).zip"
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
if (Test-Path env:\TEMPLEPLUS_VERSION) {
    nuget.exe pack -Version $env:TEMPLEPLUS_VERSION TemplePlus.nuspec
} else {
    nuget.exe pack TemplePlus.nuspec
}

"Finished packing the release."
