@echo off

C:\VS2003\Vc7\bin\ml.exe /c /Fl /Sg /coff decoder.asm
C:\VS2003\Vc7\bin\ml.exe /c /Fl /Sg /coff native.asm
C:\VS2003\Vc7\bin\ml.exe /c /Fl /Sg /coff unpacker.asm

pause
