@echo off
setlocal

set configuration=debug
if "%1"=="release" (
    set configuration=release
)

set build_dir=build\%configuration%

if not exist %build_dir% (
   build.bat %configuration%
)

cd %build_dir%

if exist marrow_test.exe (
   del marrow_test.exe
)

make

if "%2"=="" (
if exist marrow_test.exe (
   gdb -batch -ex "set logging on" -ex run -ex "bt full" -ex quit --args marrow_test
)
)

cd ../..
endlocal & exit /b %BUILD_RESULT%
