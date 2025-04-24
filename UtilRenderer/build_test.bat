@echo off

pushd Util
--call build_release.bat
popd

pushd UtilWindow
--call build_release.bat
popd

pushd UtilRenderer

if not exist build_test mkdir build_test
pushd build_test

set source_files=../main.c ../src/*.c ../src/vendor/stb_image/stb_image.c

set compile_flags=/Zi /Od ^
-I../../

set linker_flags=gdi32.lib user32.lib kernel32.lib shell32.lib ole32.lib opengl32.lib ^
../../Util/build_release/util.lib ^
../../UtilWindow/build_release/util_window.lib

cl %source_files% %compile_flags% /link %linker_flags%

popd
popd