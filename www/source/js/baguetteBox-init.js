/* global baguetteBox */

(function() {
    'use strict';

    var selector = '.content';
    var el = document.querySelectorAll(selector);

    if (el.length) {
        baguetteBox.run(selector, {
            async: false,
            buttons: true,
            noScrollbars: true
        });
    }

})();
