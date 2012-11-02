@echo off
rem install node.js and then run:
rem npm install -g clean-css
rem npm install -g uglify-js

set PATH=C:\MSYS\bin;%PATH%

pushd %~dp0

cleancss -o css/style.min.css css/style.css && ^
cat js/builder.js js/effects.js js/scriptaculous.js js/lightbox.js | uglifyjs -o js/pack.js -nc
