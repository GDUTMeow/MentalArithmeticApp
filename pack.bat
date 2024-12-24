@echo off
nuitka --standalone --include-data-dir=ui/templates=templates ^
--include-data-dir=ui/static=static ^
--include-data-file=database.dll ^
--include-data-file=app.dll ^
--include-data-file=initializer.dll ^
--output-dir=build ^
--lto=yes --assume-yes-for-downloads ui/app.py
