d:
cd \

if "%date:~5,2%" lss "10" (set mm=0%date:~6,1%) else (set mm=%date:~5,2%)
if "%date:~8,2%" lss "10" (set dd=0%date:~9,1%) else (set dd=%date:~8,2%)
if "%time:~0,2%" lss "10" (set hh=0%time:~1,1%) else (set hh=%time:~0,2%)
if "%time:~3,2%" lss "10" (set nn=0%time:~4,1%) else (set nn=%time:~3,2%)
echo [%Date:~0,4%-%mm%-%dd%-%hh%-%nn%]

set datetime=[%Date:~0,4%-%mm%-%dd%-%hh%-%nn%]
echo %datetime%

mkdir logd\%datetime%
mkdir logd\%datetime%\host
mkdir logd\%datetime%\cell1
mkdir logd\%datetime%\cell2

adb shell cswitch host 000
adb wait-for-device

adb pull /data/misc/logd/.  logd\%datetime%\host
adb pull /data/tombstones/. logd\%datetime%\host

adb pull /data/cells/cell1-rw/data/misc/logd/.  logd\%datetime%\cell1
adb pull /data/cells/cell1-rw/data/tombstones/. logd\%datetime%\cell1

adb pull /data/cells/cell2-rw/data/misc/logd/.  logd\%datetime%\cell2
adb pull /data/cells/cell2-rw/data/tombstones/. logd\%datetime%\cell2

explorer logd\%datetime%
::pause
