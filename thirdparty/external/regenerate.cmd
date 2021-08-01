@echo off
rd /s /q packages
cmake -GNinja -Bbuild . && cmake --build build && cmake --install build --prefix packages
