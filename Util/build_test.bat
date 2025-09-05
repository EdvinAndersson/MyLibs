@echo off

pushd Util

if not exist build_test mkdir build_test
pushd build_test

cl /Z7 /W4 /D_CRT_SECURE_NO_WARNINGS ../main.c ../src/*.c

popd
popd