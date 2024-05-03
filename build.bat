@echo off

IF NOT EXIST bin (
    mkdir bin
)
cl -nologo -O2 -Oi -c barbandwidth.c -Fo:"bin/"
nasm -f win64 write.asm -o "bin/write.obj"

link /NOLOGO bin/write.obj bin/barbandwidth.obj /OUT:"bin/bbw.exe"