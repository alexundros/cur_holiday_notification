@echo off
@chcp 1251 > nul
cd /d "%~dp0" || exit 1
@setlocal enabledelayedexpansion

set c=0
for %%a in (test_xmlfiles\*.xml) do (
  set /a c=!c!+1
  echo.
  echo # TEST FILE !c!: %%a
  echo.
  echo # PY_PROJECT main.py - TEST FILE !c!:
  echo.
  python py_project\src\main.py "%%a" true
  echo.
  echo # PY_PROJECT - TEST FILE !c!:
  echo.
  py_project\build\win-amd64\py_project.exe "%%a" true
  echo.
  echo # GO_PROJECT - TEST FILE !c!:
  echo.
  go_project\build\go_project.exe "%%a" true
  echo.
  echo # RUST_PROJECT - TEST FILE !c!:
  echo.
  rust_project\target\release\rust_project.exe "%%a" true
  echo.
  echo # C_PROJECT - TEST FILE !c!:
  echo.
  c_project\build\c_project.exe "%%a" true
)
pause