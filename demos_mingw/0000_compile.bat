@set PATH=C:\msys64\mingw64\bin;C:\msys64\usr\local\bin;C:\msys64\usr\bin;C:\msys64\bin;%PATH%
set MSYSTEM=MINGW64
copy C:\msys64\mingw64\bin\pdcurses.dll .
for %%i in (
    widetest inputtest
) do (
    gcc -g -O2 -Wall -Wextra -o %%i.exe %%i.c -lpdcurses
)
pause
