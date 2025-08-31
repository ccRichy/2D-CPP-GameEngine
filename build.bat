@echo off

IF NOT EXIST %~dp0build mkdir %~dp0build
pushd %~dp0build

del *.pdb > NUL 2> NUL
set pdbName="game_temp_%date:/=%.pdb"
set pdbName_f=%pdbName: =%
set name="TMP_game_%date:~-4,4%%date:~-10,2%%date:~-7,2%_%time:~0,2%%time:~3,2%%time:~6,2%.pdb"
cl @../build-flags.rsp ../code/game.cpp /Fmgame.map /LD /link /PDB:%name% /OUT:game.dll /INCREMENTAL:NO /EXPORT:game_update_and_draw /EXPORT:game_input_change_device
cl @../build-flags.rsp ../code/win32_main.cpp @../build-libs.rsp /Fmwin32_main.map

popd
