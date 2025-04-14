@echo off

if not exist build_test mkdir build_test
pushd build_test

cl /W4 /D_CRT_SECURE_NO_WARNINGS ..\main.c ../src/*.c

popd