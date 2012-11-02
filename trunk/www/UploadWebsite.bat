@echo off
rem run this batch file to upload the docs to our sourceforge server
rem to make this work, you have to create a file "docserverlogin.bat"
rem which sets the variables USERNAME and PASSWORD, and also set the PSCP
rem variable to point to your scp program the PLINK variable to the plink
rem program and the ZIP variable to your zip program

rem example docserverlogin.bat file
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
if exist website.zip del website.zip

%ZIP% a -r -tzip website.zip * > NUL

%PSCP% -r -l %USERNAME% -pw %PASSWORD% website.zip tortoisesvn.net:/var/www/vhosts/tools/

%PLINK% tortoisesvn.net -l %USERNAME% -pw %PASSWORD% unzip -o /var/www/vhosts/tools/website.zip -d /var/www/vhosts/tools/htdocs/;rm -f /var/www/vhosts/tools/website.zip

del website.zip

cd ..\stexbar\www
