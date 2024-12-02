@echo off
setlocal enabledelayedexpansion

chcp 65001

set server=\\fileserver.xdja.com\
set local=target


:loop
if exist "%local%" (
    echo exists: %local%
    set /p target=<%local%
    echo target: %target%

    if exist "%target%" (
        mv -f %target% d:\%target%
        rem 7z a -t7z -m0=LZMA2 -mx=9 -mfb=64 -md=32m %server%\%target%.7z d:\%target%
    ) else (
        echo not exists: %target%
    )
) else (
    echo not exists: %local%
)
timeout /t 60 /NOBREAK

goto loop