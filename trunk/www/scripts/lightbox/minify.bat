@echo off
rem install node.js and then run "npm install -g uglify-js"

set PATH=C:\MSYS\bin;%PATH%

pushd %~dp0

cat builder.js effects.js scriptaculous.js lightbox.js | uglifyjs -o pack.js -nc
