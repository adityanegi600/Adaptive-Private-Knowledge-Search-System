@echo off
REM // Hides command execution on screen

setlocal
REM // Keeps environment variables local

cd /d "%~dp0"
REM // Opens the folder where this BAT file is located

echo Building Reason-Act-Reflect...
REM // Displays build message

g++ -std=c++14 -O2 -Wall -Wextra main.cpp -o reason_act_reflect.exe
REM // Compiles the C++ source file into an EXE

if errorlevel 1 (
    REM // Checks whether compilation failed
    echo.
    echo Build failed! Check MinGW-w64 GCC installation and PATH.
    pause
    exit /b 1
)

echo.
echo Build successful!
REM // Displays successful build message

echo Starting Reason-Act-Reflect...
REM // Displays startup message

echo.
reason_act_reflect.exe
REM // Runs the compiled program

echo.
echo Program finished.
REM // Displays completion message

pause
REM // Keeps the window open