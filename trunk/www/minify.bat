@echo off
rem install node.js and then run:
rem npm install -g clean-css
rem npm install -g uglify-js

pushd %~dp0

type css\style.css css\normalize.css css\jquery.fancybox.css | cleancss --s0 -o css\pack.css
uglifyjs js\jquery.fancybox.js js\jquery.mousewheel.js --compress --mangle -o js\pack.js

popd
