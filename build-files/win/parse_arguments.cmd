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
    )else (
        echo Unrecognized Command
        goto EOF
    )
)else if "%1" == "" (
    set BUILD_RELEASE=1
    goto EOF
)

:EOF
exit /b 0
:ERR
exit /b 1
