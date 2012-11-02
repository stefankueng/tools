@echo off
rem install node.js and then run:
rem npm install -g clean-css
rem npm install -g uglify-js

set PATH=C:\MSYS\bin;%PATH%

pushd %~dp0

cleancss -o css/style.min.css css/style.css && ^
cat js/lightbox/builder.js js/lightbox/effects.js js/lightbox/scriptaculous.js js/lightbox/lightbox.js | uglifyjs -o js/lightbox/pack.js -nc
