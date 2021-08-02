@echo off
rd /s /q build > nul 2> nul
rd /s /q packages > nul 2> nul
cmake -GNinja -Bbuild . && cmake --build build && cmake --install build --prefix packages
