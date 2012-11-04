@echo off
rem install node.js and then run:
rem npm install -g clean-css
rem npm install -g uglify-js

pushd %~dp0

type css\style.css css\lightbox.css css\normalize.css | cleancss --s0 -o css\pack.css && ^
uglifyjs --no-copyright -o js\lightbox.min.js js\lightbox.js
