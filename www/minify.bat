@echo off
rem install node.js and then run:
rem npm install -g clean-css
rem npm install -g uglify-js

set PATH=C:\MSYS\bin;%PATH%

pushd %~dp0

cleancss -o style.min.css style.css && ^
cat scripts/lightbox/builder.js scripts/lightbox/effects.js scripts/lightbox/scriptaculous.js scripts/lightbox/lightbox.js | uglifyjs -o scripts/lightbox/pack.js -nc
