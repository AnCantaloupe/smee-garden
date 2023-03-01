@echo off

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build

set CommonCompilerFlags=-MTd -nologo -Gm- -GR- -EHsc -EHa- -Od -Oi -FC -Z7
set CommonWarningSwitches=-W4 -WX -wd4201 -wd4100 -wd4189 -wd4211 -wd4505
set CommonBuildModeFlags=-DSEEDS_INTERNAL=1 -DHANDMADE_SLOW=1 -DHANDMADE_WIN32=1
set OtherBuildModeFlags= -DPERFORMANCE_DIAGNOSTICS=0
set CommonLinkerFlags=-incremental:no -opt:ref user32.lib Gdi32.lib winmm.lib opengl32.lib
set TimeString="%date:~-4,4%%date:~-7,2%%date:~-10,2%_%time:~0,2%%time:~3,2%%time:~6,2%%time:~9,2%"

cl %CommonCompilerFlags% %CommonWarningSwitches% %CommonBuildModeFlags% %OtherBuildModeFlags% ../src/windows.cpp /link %CommonLinkerFlags%

popd

echo.
