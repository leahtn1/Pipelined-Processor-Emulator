echo off
cls
del test.exe
cd Program Data
@REM check if this fails or not
python assempler.py
cd ..
gcc processor.c main.c -o test
test
