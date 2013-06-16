@echo off
rem run this batch file to upload the docs to our sourceforge server
rem to make this work, you have to create a file "serverlogin.bat"
rem which sets the variables USERNAME and PASSWORD, and also set the PSCP
rem variable to point to your scp program the PLINK variable to the plink
rem program and the ZIP variable to your zip program

rem example serverlogin.bat file
rem
rem @echo off
rem set USERNAME=myname
rem set PASSWORD=mypassword
rem set PSCP="C:\Programme\PuttY\pscp.exe"
rem set PLINK="C:\Programme\Putty\plink.exe"
rem set ZIP="C:\Programme\7-zip\7z.exe"

if exist ..\..\toolssite rd /s /q ..\..\toolssite
pushd scripts
python generatesite.py
popd

call ..\serverlogin.bat

cd ..\..\toolssite

%PSCP% -l %USERNAME% -pw %PASSWORD% -r *.* web.sourceforge.net:/home/project-web/stefanstools/htdocs/


cd ..\stexbar\www
