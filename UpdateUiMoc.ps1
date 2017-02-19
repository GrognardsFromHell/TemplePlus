
$path = join-path $PSScriptRoot TemplePlus\ui

$headers = Get-ChildItem -File -Filter *.h $path
$moc = join-path $env:QTDIR bin\moc.exe

$ui_moc = join-path $path ui_moc.cpp
Remove-Item -ErrorAction SilentlyContinue $ui_moc

ForEach ($header in $headers) {
	$path = $header.FullName
    $shortname = $header.Name
    $ErrorActionPreference = "SilentlyContinue"
    try {
        & "$moc" "-f$shortname" "$path" >> $ui_moc
    } catch {
    }
}
