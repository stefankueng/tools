/*jshint forin:true, noarg:true, noempty:true, eqeqeq:true, bitwise:true,
  strict:false, undef:true, unused:true, curly:true, browser:true, jquery:true,
  trailing:true*/

// Avoid `console` errors in browsers that lack a console.
(function() {
    var method;
    var noop = function () {};
    var methods = [
        'assert', 'clear', 'count', 'debug', 'dir', 'dirxml', 'error',
        'exception', 'group', 'groupCollapsed', 'groupEnd', 'info', 'log',
        'markTimeline', 'profile', 'profileEnd', 'table', 'time', 'timeEnd',
        'timeStamp', 'trace', 'warn'
    ];
    var length = methods.length;
    var console = (window.console = window.console || {});

    while (length--) {
        method = methods[length];

        // Only stub undefined methods.
        if (!console[method]) {
            console[method] = noop;
        }
    }
}());

// Place any jQuery/helper plugins in here.

$(document).ready(function() {
    $('.fancybox').fancybox({});

    $(window).scroll(function() {
        if ($(this).scrollTop() > 250) {
            $('.scrollup').fadeIn();
        } else {
            $('.scrollup').fadeOut();
        }
    });

    $('.scrollup').click(function() {
        $('html, body').animate({ scrollTop: 0 }, 'normal'); /* slow, normal, fast, 600 (ms) */
        return false;
    });

});
