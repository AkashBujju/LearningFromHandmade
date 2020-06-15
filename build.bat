@echo off
cls
mkdir build
pushd build
cl.exe /Zi ..\code\win32_handmade.cpp user32.lib gdi32.lib
popd
