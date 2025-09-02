@echo off

REM variables
set exe_name="win32_main.exe" 
set pdb_name="TEMP_game_%date:~-4,4%%date:~-10,2%%date:~-7,2%_%time:~0,2%%time:~3,2%%time:~6,2%.pdb"


REM preamble
IF NOT EXIST %~dp0build mkdir %~dp0build
pushd %~dp0build
del *.pdb > NUL 2> NUL


REM compilations
cl @../build-flags.rsp ../code/game.cpp /Fmgame.map /LD /link /PDB:%pdb_name% /OUT:game.dll /INCREMENTAL:NO /EXPORT:game_update_and_draw /EXPORT:game_input_change_device

REM dont try to compile the platform if its running
tasklist /fi "ImageName eq %exe_name%" /fo csv 2>NUL | find /I %exe_name%>NUL
if "%ERRORLEVEL%"=="1" cl @../build-flags.rsp ../code/win32_main.cpp /Fe:%exe_name% @../build-libs.rsp /Fmwin32_main.map

popd
