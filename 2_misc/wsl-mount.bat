@echo off

:loop
ping 127.0.0.1 -n 1 > nul
c:\windows\system32\wsl.exe --mount \\.\PHYSICALDRIVE1
if %errorlevel% == 0 goto loop
