@echo off

set PATH=C:\MSYS\bin;%PATH%

pushd %~dp0

cat builder.js effects.js scriptaculous.js lightbox.js | uglifyjs -o pack.js -nc
