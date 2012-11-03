@echo off
rem install node.js and then run:
rem npm install -g clean-css
rem npm install -g uglify-js

pushd %~dp0

type css\style.css css\lightbox.css | cleancss -o css\pack.css && ^
uglifyjs --no-copyright -o js\lightbox.min.js js\lightbox.js
