

if (-Not(Test-Path .\Squirrel\squirrel.windows.*)) {
    "Installing Squirrel"
    .\.nuget\nuget.exe install Squirrel.Windows -OutputDirectory Squirrel -Verbose
}
