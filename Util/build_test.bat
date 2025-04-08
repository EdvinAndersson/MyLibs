@echo off

if not exist build_test mkdir build_test
pushd build_test

cl /W4 ..\main.c ../src/*.c

popd