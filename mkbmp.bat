@echo off
if not exist flxx.o call mkflxx.bat
gcc -Wall -o u9bmp.exe u9bmp.c flxx.o -lalleg
