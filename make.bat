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
	powershell Remove-Item -path ./generated_src -recurse -ErrorAction Ignore
	powershell Remove-Item -path ./proj_release -recurse -ErrorAction Ignore
	powershell Remove-Item -path ./proj_debug -recurse -ErrorAction Ignore
	powershell Remove-Item -path ./_out -recurse -ErrorAction Ignore
	goto EOF
)

if "%CLEAN_DEP%" == "1" (
	echo Cleaning all dependencies file
	powershell Remove-Item -path ./dependencies -recurse -ErrorAction Ignore
	goto EOF
)

if "%UPDATE_DEP%" == "1" (
	echo Downloading dependencies
	powershell .\build-files\win\getdep.ps1
	goto EOF
)

if "%UPDATE%" == "1" (
	echo Sycning latest code
	git pull
	goto EOF
)

if "%BUILD_RELEASE%" == "1" (
	echo Building

	powershell New-Item -Force -ItemType directory -Path proj_release
	cd proj_release
	cmake -A x64 ..
	msbuild /p:Configuration=Release TSL.sln
	cd ..
)

if "%BUILD_DEBUG%" == "1" (
	echo Building

	powershell New-Item -Force -ItemType directory -Path proj_debug
	cd proj_debug
	cmake -A x64 ..
	msbuild /p:Configuration=Debug TSL.sln
	cd ..
)

if "%GENERATE_PROJ%" == "1" (
	echo Generating Visual Studio Project
	
	powershell New-Item -Force -ItemType directory -Path _out
	cd _out
	cmake -A x64 ..
	cd ..
)

if "%GENERATE_SRC%" == "1" (
	echo Generate source code dependencies

	powershell Remove-Item -path ./src/generated -recurse -ErrorAction Ignore
	mkdir src\generated

	echo "Bison parsing ..."
	.\dependencies\flex_bison\win_bison.exe -d .\src\grammer.y -o .\generated_src\compiled_grammer.c

	echo "Laxer parsing ..."
	.\dependencies\flex_bison\win_flex.exe .\src\lex.l
)

:EOF
