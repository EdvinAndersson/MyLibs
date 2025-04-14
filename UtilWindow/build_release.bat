@echo off

pushd UtilWindow

if not exist build_release mkdir build_release
pushd build_release

set compile_flags=/O2 /MP /Oi /W4 /WX /c ^
-I../../

set source_files=^
../src/*.c

set linker_flags=^
../../Util/build_release/util.lib ^
/out:util_window.lib

cl %compile_flags% %source_files%

if %ERRORLEVEL% == 0 (
    lib *.obj %linker_flags%
) else (
    echo ---- Error was found. Not Creating a .lib ----
)

popd
popd