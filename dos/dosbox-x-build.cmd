@echo off
rem Runs dosbox-x-build.ps1 without changing the PowerShell execution
rem policy.  Arguments are passed on, e.g.:
rem   dos\dosbox-x-build.cmd -Iwad C:\Games\DOOM\DOOM.WAD
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0dosbox-x-build.ps1" %*
