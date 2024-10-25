@echo off

set FLAGS=-Od -Ob1 -FC -Z7 -nologo 
REM -g -std=c99 -Wall -Wextra -pedantic
REM -lmingw32
set LIBS=..\lib\SDL2main.lib ..\lib\SDL2.lib opengl32.lib glu32.lib ..\lib\glew32.lib shell32.lib

pushd bin
cl ..\main.c %FLAGS% -Fe"demo.exe" -I ..\include %LIBS% /link /SUBSYSTEM:CONSOLE 
popd
