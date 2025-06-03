@echo off
@chcp 1251 > nul
cd /d "%~dp0" || exit 1

echo.
echo # BUILD PY_PROJECT:
echo.
cd "%~dp0py_project"
python cx_setup___main.py build --compiler=mingw32
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
