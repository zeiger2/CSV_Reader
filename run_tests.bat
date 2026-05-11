@echo off
setlocal enabledelayedexpansion

if not exist "csvreader.exe" (
    echo [ERROR] csvreader.exe not found!
    echo.
    echo Please build the project first:
    echo   mingw32-make
    echo or manually:
    echo   gcc -std=c99 -Wall -o csvreader.exe main.c table.c compute.c utils.c -lm
    echo.
    pause
    exit /b 1
)

echo Running tests...
echo.
for %%f in (tests\*.csv) do (
    echo [TEST] %%~nxf
    csvreader.exe "%%f"
    echo.
    echo ------------------------------
    echo.
)

echo All tests completed.
pause