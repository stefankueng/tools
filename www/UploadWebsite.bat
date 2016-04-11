@echo off
rem Run this batch file to upload the files to the SourceForge server.
rem To make this work, you have to create a file "serverlogin.bat"
rem which sets the USERNAME, PASSWORD and PSCP variables

rem example serverlogin.bat file:
rem
rem @echo off
rem set USERNAME=myname
rem set PASSWORD=mypassword
rem set PSCP="C:\Programme\PuttY\pscp.exe"


pushd %~dp0

if not exist "..\serverlogin.bat" (
  echo No login information provided!
  pause
  goto end
)

rem Make sure we have all the deps up to date
if exist "node_modules/" (
  cmd /c npm prune
  cmd /c npm update
)

cmd /c npm install
if %errorlevel% neq 0 (
  echo `npm install` failed. Please try again later.
  pause
  goto end
)

rem Run the actual build command
cmd /c grunt build
if %errorlevel% neq 0 (
  echo `grunt build` failed. Check the build log and try again later.
  pause
  goto end
)

rem Get the login info
call "..\serverlogin.bat"

pushd "dist\"
rem Upload the files to the server
%PSCP% -l %USERNAME% -pw %PASSWORD% -r *.* web.sourceforge.net:/home/project-web/stefanstools/htdocs/

popd

:end
exit /b
