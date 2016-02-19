# TemplePlus

[![Build status](https://ci.appveyor.com/api/projects/status/github/GrognardsFromHell/TemplePlus?svg=true)]
(https://ci.appveyor.com/project/shartte/templeplus)

## Quickstart

* Get [Microsoft Visual Studio 2015](https://www.visualstudio.com/)
* Clone the repository (use the HTTPS URL instead of the SSH URL so you can use Visual Studio's Git integration).
* Download the latest [binary dependency release](https://github.com/GrognardsFromHell/TemplePlusDependencies/releases/) and unzip it into the ``dependencies`` folder inside your dev folder.
* Press F5 :)
* Edit the `TemplePlus.ini` in the Debug or Release directory (whichever you launched) and add the following (adjust the path to the TemplePlus dev folder)
```
[TemplePlus]
additionalTioPaths=D:\TemplePlus\tpdata;D:\TemplePlus\dependencies
```
* Also check and modify the ToEE folder used by TemplePlus in the same file if you so desire.

Special thanks to Deleaker for providing a free copy and supporting open source :)  
www.deleaker.com
