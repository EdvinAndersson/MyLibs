@echo off

pushd Util
call build_release.bat
popd

pushd UtilWindow
call build_release.bat
popd

pushd UtilRenderer

if not exist build_release mkdir build_release
pushd build_release

set source_files=../src/*.c ../src/vendor/stb_image/stb_image.c

set compile_flags=/O2 /MP /Oi /W4 /WX /c ^
-I../../ ^
-I../src/vendor/freetype/include

set linker_flags=gdi32.lib ^
/out:util_renderer.lib

cl %source_files% %compile_flags%

if %ERRORLEVEL% == 0 (
    lib *.obj %linker_flags%
) else (
    echo ---- Error was found. Not Creating a .lib ----
)

popd
popd