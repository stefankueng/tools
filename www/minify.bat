@echo off
rem install node.js and then run:
rem npm install -g clean-css
rem npm install -g uglify-js

pushd %~dp0

type css\jquery.fancybox.css css\style.css css\normalize.css | cleancss --s0 -o css\pack.css
type js\jquery.fancybox.js js\jquery.mousewheel.js | uglifyjs --no-copyright -o js\pack.js
