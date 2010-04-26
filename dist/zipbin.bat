rem default-valued argument
set GAME=aok
if not (%2)==() set GAME=%2

ren readme_%GAME%.txt readme.txt
"%ProgramFiles%\7-zip\7z" -Tzip a %GAME%ts-%1.zip %GAME%ts.exe libexpatw.dll zlib1.dll aokts.cnt aokts.ini aokts.hlp readme.txt scx_format.txt data_%GAME%.xml source-%1.zip COPYING.txt
ren readme.txt readme_%GAME%.txt
