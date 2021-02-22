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
set TSL_DIR=%~dp0

rem reset all variables first
set CLEAN=
set BUILD_RELEASE=
set BUILD_DEBUG=
set GENERATE_PROJ=
set UPDATE_DEP=
set FORCE_UPDATE_DEP=
set CLEAN_DEP=
set UPDATE=
set GENERATE_SRC=
set UNIT_TEST=
set FULL=
set INSTALL=
set VERIFY_BUILDS=
set RESOLVED_INSTALL_PATH="./tsl"

rem parse arguments
:argv_loop
if NOT "%1" == "" (
    if "%1" == "clean" (
        set CLEAN=1
        goto EOF
    )else if "%1" == "update" (
        set UPDATE=1
        goto EOF
    )else if "%1" == "clean_dep" (
        set CLEAN_DEP=1
        goto EOF
    )else if "%1" == "update_dep" (
        set UPDATE_DEP=1
        goto EOF
    )else if "%1" == "force_update_dep" (
        set FORCE_UPDATE_DEP=1
        goto EOF
    )else if "%1" == "generate_src" (
        set GENERATE_SRC=1
        goto EOF
    )else if "%1" == "generate_proj" (
        set GENERATE_PROJ=1
        goto EOF
    )else if "%1" == "release" (
        set BUILD_RELEASE=1
        goto EOF
    )else if "%1" == "debug" (
        set BUILD_DEBUG=1
        goto EOF
    )else if "%1" == "test" (
        set UNIT_TEST=1
        goto EOF
    )else if "%1" == "full" (
        set FULL=1
        goto EOF
    )else if "%1" == "install" (
        set INSTALL=1
        if "%2" == "INSTALL_PATH" (
            set RESOLVED_INSTALL_PATH=%3
        )
        goto EOF
    )else if "%1" == "verify_builds" (
        set VERIFY_BUILDS=1
        goto EOF
    )else (
        echo Unrecognized Command
        goto EOF
    )
)else if "%1" == "" (
    set BUILD_RELEASE=1
    goto EOF
)

:EOF

if "%CLEAN%" == "1" (
    echo [33mCleaning all temporary file[0m
    powershell Remove-Item -path ./bin -recurse -ErrorAction Ignore
    powershell Remove-Item -path ./generated_src -recurse -ErrorAction Ignore
    powershell Remove-Item -path ./proj_release -recurse -ErrorAction Ignore
    powershell Remove-Item -path ./proj_debug -recurse -ErrorAction Ignore
    powershell Remove-Item -path ./_out -recurse -ErrorAction Ignore
    goto EOF
)

if "%CLEAN_DEP%" == "1" (
    echo [33mCleaning all dependencies file[0m
    powershell Remove-Item -path ./dependencies -recurse -ErrorAction Ignore
    goto EOF
)

if "%UPDATE_DEP%" == "1" (
    echo [33mDownloading dependencies[0m
    py .\scripts\get_dependencies.py
    goto EOF
)

if "%FORCE_UPDATE_DEP%" == "1" (
    echo [33mDownloading dependencies[0m
    py .\scripts\get_dependencies.py TRUE
    goto EOF
)

if "%UPDATE%" == "1" (
    echo [33mSycning latest code[0m
    git pull
    goto EOF
)

if "%BUILD_RELEASE%" == "1" (
    echo [33mBuilding release[0m

    py .\scripts\get_dependencies.py

    powershell New-Item -Force -ItemType directory -Path proj_release
    cd proj_release
    cmake -A x64 ..
    msbuild /p:Configuration=Release TSL.sln

    :: catch msbuild error
    if ERRORLEVEL 1 ( 
        goto BUILD_ERR
    )

    cd ..
)

if "%BUILD_DEBUG%" == "1" (
    echo [33mBuilding debug[0m

    py .\scripts\get_dependencies.py
    
    powershell New-Item -Force -ItemType directory -Path proj_debug
    cd proj_debug
    cmake -A x64 ..
    msbuild /p:Configuration=Debug TSL.sln

    :: catch msbuild error
    if ERRORLEVEL 1 ( 
        goto BUILD_ERR
    )

    cd ..
)

if "%GENERATE_PROJ%" == "1" (
    echo [33mGenerating Visual Studio Project[0m
    
    powershell New-Item -Force -ItemType directory -Path _out
    cd _out
    cmake -A x64 ..
    cd ..
)

if "%GENERATE_SRC%" == "1" (
    echo [33mGenerating flex and bison source code[0m
    
    powershell Remove-Item -path ./generated_src -recurse -ErrorAction Ignore
    mkdir generated_src

    .\dependencies\flex_bison\win_bison.exe -d .\src\tsl_lib\compiler\grammar.y -o .\generated_src\compiled_grammar.cpp
    .\dependencies\flex_bison\win_flex.exe .\src\tsl_lib\compiler\lex.l
)

if "%UNIT_TEST%" == "1" (
    echo [33mRunning unit tests[0m
    .\bin\tsl_test_r.exe
    .\bin\llvm_test_r.exe
)

if "%FULL%" == "1" (
    make update
    make clean
    make
    make test 
)

if "%INSTALL%" == "1" (
    echo [33mBuild and install TSL[0m
    echo Install Path: %RESOLVED_INSTALL_PATH%
    make
    cmake -DCMAKE_INSTALL_PREFIX=%RESOLVED_INSTALL_PATH% -P ./proj_release/cmake_install.cmake
)

if "%VERIFY_BUILDS%" == "1" (
    echo [33mVerifying builds[0m
    py .\scripts\verify_builds.py
)

:EOF
exit /b 0
:BUILD_ERR
exit /b 1