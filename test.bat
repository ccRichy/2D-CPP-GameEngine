@echo off
set exe_name="template.exe" 
tasklist /fi "ImageName eq %exe_name%" /fo csv 2>NUL | find /I %exe_name%>NUL
if "%ERRORLEVEL%"=="0" echo Program is running %exe_name%
if "%ERRORLEVEL%"=="1" echo Program is not running %exe_name%
