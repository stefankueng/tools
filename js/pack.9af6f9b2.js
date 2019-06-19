!function(){"use strict";for(var e,t=function(){},n=["assert","clear","count","debug","dir","dirxml","error","exception","group","groupCollapsed","groupEnd","info","log","markTimeline","profile","profileEnd","table","time","timeEnd","timeStamp","trace","warn"],o=n.length,i=window.console=window.console||{};o--;)i[e=n[o]]||(i[e]=t)}(),function(e,t){"use strict";"function"==typeof define&&define.amd?define(t):"object"==typeof exports?module.exports=t():e.baguetteBox=t()}(this,function(){"use strict";var r,s,l,u,n,o='<svg width="44" height="60"><polyline points="30 10 10 30 30 50" stroke="rgba(255,255,255,0.5)" stroke-width="4"stroke-linecap="butt" fill="none" stroke-linejoin="round"/></svg>',i='<svg width="44" height="60"><polyline points="14 10 34 30 14 50" stroke="rgba(255,255,255,0.5)" stroke-width="4"stroke-linecap="butt" fill="none" stroke-linejoin="round"/></svg>',a='<svg width="30" height="30"><g stroke="rgb(160,160,160)" stroke-width="4"><line x1="5" y1="5" x2="25" y2="25"/><line x1="5" y1="25" x2="25" y2="5"/></g></svg>',d={},c={captions:!0,buttons:"auto",fullScreen:!1,noScrollbars:!1,bodyClass:"baguetteBox-open",titleTag:!1,async:!1,preload:2,animation:"slideIn",afterShow:null,afterHide:null,onChange:null,overlayBackgroundColor:"rgba(0,0,0,.8)"},f={},g=[],p=0,m=!1,b={},v=!1,h=/.+\.(gif|jpe?g|png|webp)/i,y={},w=[],t=null,k=function(e){-1!==e.target.id.indexOf("baguette-img")&&j()},x=function(e){e.stopPropagation?e.stopPropagation():e.cancelBubble=!0,D()},E=function(e){e.stopPropagation?e.stopPropagation():e.cancelBubble=!0,Y()},C=function(e){e.stopPropagation?e.stopPropagation():e.cancelBubble=!0,j()},B=function(e){b.count++,1<b.count&&(b.multitouch=!0),b.startX=e.changedTouches[0].pageX,b.startY=e.changedTouches[0].pageY},T=function(e){if(!v&&!b.multitouch){e.preventDefault?e.preventDefault():e.returnValue=!1;var t=e.touches[0]||e.changedTouches[0];40<t.pageX-b.startX?(v=!0,D()):t.pageX-b.startX<-40?(v=!0,Y()):100<b.startY-t.pageY&&j()}},N=function(){b.count--,b.count<=0&&(b.multitouch=!1),v=!1},A=function(){N()},S=function(e){"block"===r.style.display&&r.contains&&!r.contains(e.target)&&(e.stopPropagation(),H())};function L(e){if(y.hasOwnProperty(e)){var t=y[e].galleries;[].forEach.call(t,function(e){[].forEach.call(e,function(e){V(e.imageElement,"click",e.eventHandler)}),g===e&&(g=[])}),delete y[e]}}function P(e){switch(e.keyCode){case 37:D();break;case 39:Y();break;case 27:j();break;case 36:!function(e){e&&e.preventDefault();O(0)}(e);break;case 35:!function(e){e&&e.preventDefault();O(g.length-1)}(e)}}function F(e,t){if(g!==e){for(g=e,function(e){e=e||{};for(var t in c)d[t]=c[t],void 0!==e[t]&&(d[t]=e[t]);s.style.transition=s.style.webkitTransition="fadeIn"===d.animation?"opacity .4s ease":"slideIn"===d.animation?"":"none","auto"===d.buttons&&("ontouchstart"in window||1===g.length)&&(d.buttons=!1);l.style.display=u.style.display=d.buttons?"":"none";try{r.style.backgroundColor=d.overlayBackgroundColor}catch(e){}}(t);s.firstChild;)s.removeChild(s.firstChild);for(var n,o=[],i=[],a=w.length=0;a<e.length;a++)(n=W("div")).className="full-image",n.id="baguette-img-"+a,w.push(n),o.push("baguetteBox-figure-"+a),i.push("baguetteBox-figcaption-"+a),s.appendChild(w[a]);r.setAttribute("aria-labelledby",o.join(" ")),r.setAttribute("aria-describedby",i.join(" "))}}function q(e){d.noScrollbars&&(document.documentElement.style.overflowY="hidden",document.body.style.overflowY="scroll"),"block"!==r.style.display&&(U(document,"keydown",P),b={count:0,startX:null,startY:null},I(p=e,function(){R(p),z(p)}),M(),r.style.display="block",d.fullScreen&&(r.requestFullscreen?r.requestFullscreen():r.webkitRequestFullscreen?r.webkitRequestFullscreen():r.mozRequestFullScreen&&r.mozRequestFullScreen()),setTimeout(function(){r.className="visible",d.bodyClass&&document.body.classList&&document.body.classList.add(d.bodyClass),d.afterShow&&d.afterShow()},50),d.onChange&&d.onChange(p,w.length),t=document.activeElement,H(),m=!0)}function H(){d.buttons?l.focus():n.focus()}function j(){d.noScrollbars&&(document.documentElement.style.overflowY="auto",document.body.style.overflowY="auto"),"none"!==r.style.display&&(V(document,"keydown",P),r.className="",setTimeout(function(){r.style.display="none",document.fullscreen&&(document.exitFullscreen?document.exitFullscreen():document.mozCancelFullScreen?document.mozCancelFullScreen():document.webkitExitFullscreen&&document.webkitExitFullscreen()),d.bodyClass&&document.body.classList&&document.body.classList.remove(d.bodyClass),d.afterHide&&d.afterHide(),t&&t.focus(),m=!1},500))}function I(t,n){var e=w[t],o=g[t];if(void 0!==e&&void 0!==o)if(e.getElementsByTagName("img")[0])n&&n();else{var i=o.imageElement,a=i.getElementsByTagName("img")[0],r="function"==typeof d.captions?d.captions.call(g,i):i.getAttribute("data-caption")||i.title,s=function(e){var t=e.href;if(e.dataset){var n=[];for(var o in e.dataset)"at-"!==o.substring(0,3)||isNaN(o.substring(3))||(n[o.replace("at-","")]=e.dataset[o]);for(var i=Object.keys(n).sort(function(e,t){return parseInt(e,10)<parseInt(t,10)?-1:1}),a=window.innerWidth*window.devicePixelRatio,r=0;r<i.length-1&&i[r]<a;)r++;t=n[i[r]]||t}return t}(i),l=W("figure");if(l.id="baguetteBox-figure-"+t,l.innerHTML='<div class="baguetteBox-spinner"><div class="baguetteBox-double-bounce1"></div><div class="baguetteBox-double-bounce2"></div></div>',d.captions&&r){var u=W("figcaption");u.id="baguetteBox-figcaption-"+t,u.innerHTML=r,l.appendChild(u)}e.appendChild(l);var c=W("img");c.onload=function(){var e=document.querySelector("#baguette-img-"+t+" .baguetteBox-spinner");l.removeChild(e),!d.async&&n&&n()},c.setAttribute("src",s),c.alt=a&&a.alt||"",d.titleTag&&r&&(c.title=r),l.appendChild(c),d.async&&n&&n()}}function Y(){return O(p+1)}function D(){return O(p-1)}function O(e,t){return!m&&0<=e&&e<t.length?(F(t,d),q(e),!0):e<0?(d.animation&&X("left"),!1):e>=w.length?(d.animation&&X("right"),!1):(I(p=e,function(){R(p),z(p)}),M(),d.onChange&&d.onChange(p,w.length),!0)}function X(e){s.className="bounce-from-"+e,setTimeout(function(){s.className=""},400)}function M(){var e=100*-p+"%";"fadeIn"===d.animation?(s.style.opacity=0,setTimeout(function(){f.transforms?s.style.transform=s.style.webkitTransform="translate3d("+e+",0,0)":s.style.left=e,s.style.opacity=1},400)):f.transforms?s.style.transform=s.style.webkitTransform="translate3d("+e+",0,0)":s.style.left=e}function R(e){e-p>=d.preload||I(e+1,function(){R(e+1)})}function z(e){p-e>=d.preload||I(e-1,function(){z(e-1)})}function U(e,t,n,o){e.addEventListener?e.addEventListener(t,n,o):e.attachEvent("on"+t,function(e){(e=e||window.event).target=e.target||e.srcElement,n(e)})}function V(e,t,n,o){e.removeEventListener?e.removeEventListener(t,n,o):e.detachEvent("on"+t,n)}function G(e){return document.getElementById(e)}function W(e){return document.createElement(e)}return[].forEach||(Array.prototype.forEach=function(e,t){for(var n=0;n<this.length;n++)e.call(t,this[n],n,this)}),[].filter||(Array.prototype.filter=function(e,t,n,o,i){for(n=this,o=[],i=0;i<n.length;i++)e.call(t,n[i],i,n)&&o.push(n[i]);return o}),{run:function(e,t){return f.transforms=function(){var e=W("div");return void 0!==e.style.perspective||void 0!==e.style.webkitPerspective}(),f.svg=function(){var e=W("div");return e.innerHTML="<svg/>","http://www.w3.org/2000/svg"===(e.firstChild&&e.firstChild.namespaceURI)}(),f.passiveEvents=function(){var e=!1;try{var t=Object.defineProperty({},"passive",{get:function(){e=!0}});window.addEventListener("test",null,t)}catch(e){}return e}(),function(){if(r=G("baguetteBox-overlay"))return s=G("baguetteBox-slider"),l=G("previous-button"),u=G("next-button"),n=G("close-button");(r=W("div")).setAttribute("role","dialog"),r.id="baguetteBox-overlay",document.getElementsByTagName("body")[0].appendChild(r),(s=W("div")).id="baguetteBox-slider",r.appendChild(s),(l=W("button")).setAttribute("type","button"),l.id="previous-button",l.setAttribute("aria-label","Previous"),l.innerHTML=f.svg?o:"&lt;",r.appendChild(l),(u=W("button")).setAttribute("type","button"),u.id="next-button",u.setAttribute("aria-label","Next"),u.innerHTML=f.svg?i:"&gt;",r.appendChild(u),(n=W("button")).setAttribute("type","button"),n.id="close-button",n.setAttribute("aria-label","Close"),n.innerHTML=f.svg?a:"&times;",r.appendChild(n),l.className=u.className=n.className="baguetteBox-button",function(){var e=f.passiveEvents?{passive:!0}:null;U(r,"click",k),U(l,"click",x),U(u,"click",E),U(n,"click",C),U(s,"contextmenu",A),U(r,"touchstart",B,e),U(r,"touchmove",T,e),U(r,"touchend",N),U(document,"focus",S,!0)}()}(),L(e),function(e,a){var t=document.querySelectorAll(e),n={galleries:[],nodeList:t};return y[e]=n,[].forEach.call(t,function(e){a&&a.filter&&(h=a.filter);var t=[];if(t="A"===e.tagName?[e]:e.getElementsByTagName("a"),0!==(t=[].filter.call(t,function(e){if(-1===e.className.indexOf(a&&a.ignoreClass))return h.test(e.href)})).length){var i=[];[].forEach.call(t,function(e,t){function n(e){e.preventDefault?e.preventDefault():e.returnValue=!1,F(i,a),q(t)}var o={eventHandler:n,imageElement:e};U(e,"click",n),i.push(o)}),n.galleries.push(i)}}),n.galleries}(e,t)},show:O,showNext:Y,showPrevious:D,hide:j,destroy:function(){!function(){var e=f.passiveEvents?{passive:!0}:null;V(r,"click",k),V(l,"click",x),V(u,"click",E),V(n,"click",C),V(s,"contextmenu",A),V(r,"touchstart",B,e),V(r,"touchmove",T,e),V(r,"touchend",N),V(document,"focus",S,!0)}(),function(){for(var e in y)y.hasOwnProperty(e)&&L(e)}(),V(document,"keydown",P),document.getElementsByTagName("body")[0].removeChild(document.getElementById("baguetteBox-overlay")),y={},g=[],p=0}}}),function(){"use strict";var e=".content";document.querySelectorAll(e).length&&baguetteBox.run(e,{async:!1,buttons:!0,noScrollbars:!0})}(),function(e,t,n,o,i,a,r){e.GoogleAnalyticsObject=i,e.ga=e.ga||function(){(e.ga.q=e.ga.q||[]).push(arguments)},e.ga.l=1*new Date,a=t.createElement("script"),r=t.getElementsByTagName("script")[0],a.async=1,a.src="https://www.google-analytics.com/analytics.js",r.parentNode.insertBefore(a,r)}(window,document,0,0,"ga"),ga("create","UA-61301138-1","auto"),ga("set","anonymizeIp",!0),ga("send","pageview");