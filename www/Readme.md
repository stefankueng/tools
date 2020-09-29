# Getting started

* Install [node.js](https://nodejs.org/en/)
* Install grunt: `npm install -g grunt-cli`
* Install the node.js dependencies: `cd www && npm install`
* Run `grunt build` to build the static site, `grunt` to build and watch for changes (http://localhost:8001/)
* In case something does not work, run `rd /q /s node_modules && npm cache clean -fg && npm i`
* To test the site, run `npm test`
