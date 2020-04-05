:argv_loop
if NOT "%1" == "" (
    if "%1" == "clean" (
        set CLEAN=1
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
