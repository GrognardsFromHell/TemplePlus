# TemplePlus

[![Build status](https://ci.appveyor.com/api/projects/status/github/GrognardsFromHell/TemplePlus?svg=true)]
(https://ci.appveyor.com/project/shartte/templeplus)

## Quickstart

* Get Microsoft Visual Studio
* Get a Git client (such as Source Tree). You can switch to Visual Studio's built in Source Control later.
* Clone the repository (use the HTTPS URL instead of the SSH URL so you can use Visual Studio's Git integration).
* Configure the solution so it points to your ToEE folder when running the debugger (right click ->  Properties -> Configuration Properties -> Debugging -> Working Directory)
* Create a file TemplePlus.ini in your ToEE directory and add the following (adjust the path to the TemplePlus dev folder)
```
[TemplePlus]
additionalTioPaths=D:\TemplePlus\tpdata;D:\TemplePlus\dependencies
```
* Download the latest [binary dependency release](https://github.com/GrognardsFromHell/TemplePlusDependencies/releases/) and unzip it into the ``dependencies`` folder inside your dev folder.
* Press F5 :)

Special thanks to Deleaker for providing a free copy and supporting open source :)  
www.deleaker.com