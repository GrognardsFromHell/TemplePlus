
if ($env:APPVEYOR_REPO_TAG -ne "true") {
    "Build is not running from a tag. Not updating version."
    Exit
}

"Tag Name: $env:APPVEYOR_REPO_TAG_NAME"

$version = $env:APPVEYOR_REPO_TAG_NAME
if ($version -notmatch "^v\d+\.\d+(\.\d+)*(|-.*)$") {
    "Tag name doesnt match release pattern. Not updating version."
    Exit
}

$version = $version.Substring(1); # Skip the v at the beginning

"Version: $version"
if (Get-Command "Update-AppveyorBuild" -errorAction SilentlyContinue) {
    Update-AppveyorBuild -Version $version
} else {
    "Not running in Appveyor environment. Skipping version change."
}
