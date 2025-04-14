@echo off

if not exist build_release mkdir build_release
pushd build_release

set compile_flags=^
/D_CRT_SECURE_NO_WARNINGS ^
/O2 /MP /Oi /W4 /WX /c

set source_files=^
../src/*.c

set linker_flags=^
/out:util.lib

cl %compile_flags% %source_files%

if %ERRORLEVEL% == 0 (
    lib *.obj %linker_flags%
) else (
    echo ---- Error was found. Not Creating a .lib ----
)

popd