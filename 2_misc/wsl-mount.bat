:loop
ping 127.0.0.1 -n 1 > nul
::echo GET-CimInstance -query "SELECT * from Win32_DiskDrive"
c:\windows\system32\wsl.exe --mount \\.\PHYSICALDRIVE0
if %errorlevel% == 0 goto loop