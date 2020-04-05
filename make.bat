@echo off
set SORT_DIR=%~dp0

rem reset all variables first
call "%SORT_DIR%\build-files\win\reset_variables.cmd"

rem parse arguments
call "%SORT_DIR%\build-files\win\parse_arguments.cmd" %*
if errorlevel 1 goto EOF

if "%CLEAN%" == "1" (
	echo Cleaning all temporary file
	powershell Remove-Item -path ./bin -recurse -ErrorAction Ignore
	powershell Remove-Item -path ./tmp -recurse -ErrorAction Ignore
	goto EOF
)

if "%BUILD_RELEASE%" == "1" (
	echo Building

	rem Making directories
	powershell Remove-Item -path ./bin -recurse -ErrorAction Ignore
	powershell Remove-Item -path ./tmp -recurse -ErrorAction Ignore
	mkdir tmp
	mkdir bin

	rem Bison parsing
	echo "Bison parsing ..."
	.\dep\win\flex_bison\win_bison.exe -d .\src\grammer.y -o .\tmp\compiled_grammer.c

	echo "Laxer parsing ..."
	.\dep\win\flex_bison\win_flex.exe .\src\lex.l

	
)

:EOF
