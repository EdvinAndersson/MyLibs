@echo off

pushd UtilWindow

if not exist build_test mkdir build_test
pushd build_test

set compile_flags=^
-I../../

set linker_flags=gdi32.lib user32.lib kernel32.lib shell32.lib ole32.lib ../../Util/build_release/util.lib

cl ../main.c ../src/*.c %compile_flags% %linker_flags%

popd
popd