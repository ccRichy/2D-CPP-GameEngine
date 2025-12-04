set datadir=%~dp0data
set builddir=%~dp0build
set packdir=%builddir%\package
IF NOT EXIST %packdir% mkdir %packdir%
copy "%builddir%\game.dll" "%packdir%\game.dll"
copy "%builddir%\*.exe" "%packdir%\"
xcopy "%datadir%" "%packdir%\data" /s /y
echo d
