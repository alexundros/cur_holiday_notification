@echo off
@chcp 1251 > nul
setlocal enabledelayedexpansion
cd /d "%~dp0" || exit 1

echo.
echo # BUILD PY_PROJECT:
echo.
cd "%~dp0py_project"
python cx_setup_build.py build --compiler=mingw32
echo. 
echo # BUILD GO_PROJECT:
echo.
cd "%~dp0go_project"
go build -ldflags="-s -w" -trimpath -buildvcs=false -o build/go_project.exe
echo.
echo # BUILD RUST_PROJECT:
echo.
cd "%~dp0rust_project"
cargo build --release
echo.
echo # BUILD C_PROJECT:
echo.
cd "%~dp0c_project"
cd src & set list= & for /r %%f in (*.c) do set "list=!list! %%f" & cd ..
:: help for flags: pkg-config --cflags --libs --static libxml-2.0
gcc -v !list! -Wall -Wextra -O3 -flto -o build/c_project.exe -Isrc/** ^
-I%MSYS2_HOME%/mingw64/include -L%MSYS2_HOME%/mingw64/lib ^
-I%MSYS2_HOME%/mingw64/include/libxml2 ^
-static -DLIBXML_STATIC -lxml2 -liconv -lws2_32 -lz -llzma
