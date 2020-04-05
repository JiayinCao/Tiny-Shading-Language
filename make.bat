::
::    This file is a part of Tiny-Shading-Language or TSL, an open-source cross
::    platform programming shading language.
::
::    Copyright (c) 2020-2020 by Jiayin Cao - All rights reserved.
::
::    TSL is a free software written for educational purpose. Anyone can distribute
::    or modify it under the the terms of the GNU General Public License Version 3 as
::    published by the Free Software Foundation. However, there is NO warranty that
::    all components are functional in a perfect manner. Without even the implied
::    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
::    General Public License for more details.
::
::    You should have received a copy of the GNU General Public License along with
::    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
:: 

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
