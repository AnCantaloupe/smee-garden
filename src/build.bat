@echo off

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build

set CommonCompilerFlags=-MTd -nologo -Gm- -GR- -EHsc -EHa- -Od -Oi -FC -Z7
set CommonWarningSwitches=-W4 -WX -wd4201 -wd4100 -wd4189 -wd4211 -wd4505
set CommonBuildModeFlags=-DSEEDS_INTERNAL=1 -DSEEDS_WIN32=1
set OtherBuildModeFlags= -DPERFORMANCE_DIAGNOSTICS=0
set CommonLinkerFlags=-incremental:no -opt:ref user32.lib Gdi32.lib winmm.lib opengl32.lib

cl %CommonCompilerFlags% %CommonWarningSwitches% %CommonBuildModeFlags% %OtherBuildModeFlags% ../src/windows.cpp /link %CommonLinkerFlags%

popd

echo.
