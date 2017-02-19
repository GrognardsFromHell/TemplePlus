
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

#------------------------

$path = join-path $PSScriptRoot TemplePlus\qml

$headers = Get-ChildItem -File -Filter *.h $path
$moc = join-path $env:QTDIR bin\moc.exe

$ui_moc = join-path $path qml_moc.cpp
Remove-Item -ErrorAction SilentlyContinue $ui_moc

# Generate the two plugins separately, they have conflicting declarations
& "$moc" "-fqmlplugin.h" $(join-path $path qmlplugin.h) "-o" $(join-path $path qmlplugin_moc.cpp)
& "$moc" "-fimageplugin.h" $(join-path $path imageplugin.h) "-o" $(join-path $path imageplugin_moc.cpp)

ForEach ($header in $headers) {

    if (($header.Name -eq "imageplugin.h") -or ($header.Name -eq "qmlplugin.h")) {
        continue;
    }

	$path = $header.FullName
    $shortname = $header.Name
    $ErrorActionPreference = "SilentlyContinue"
    try {
        & "$moc" "-f$shortname" "$path" >> $ui_moc
    } catch {
    }
}

