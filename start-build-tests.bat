@echo off
@chcp 1251 > nul
cd /d "%~dp0" || exit 1

for %%a in (test_xmlfiles\*.xml) do (
  echo # TEST XML: %%a
  echo.
  echo # PY_PROJECT:
  py_project\build\win-amd64\py_project.exe "%%a"
  echo.
  echo # GO_PROJECT:
  go_project\build\go_project.exe "%%a"
  echo.
  echo # RUST_PROJECT:
  rust_project\target\release\rust_project.exe "%%a"
)