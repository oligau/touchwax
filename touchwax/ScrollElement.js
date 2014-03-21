/*
 * ScrollElement - Kinetic scrolling for WebKit on touch devices.
 * http://webkitjs.org/ScrollElement/
 * 
 * Copyright (c) 2011 Niclas Norgren, http://niclas.n3g.net
 * Created 2011-03-13
 */

/**
 * Simple bind method
 */
if (!Function.prototype.bind) {
	Function.prototype.bind = function(context) {
		var fun = this;
		return function() {
			return fun.apply(context, arguments);
		};
	};
};

/**
 * Whether the browser supports touch (as we want to play with this on the PC as
 * well)
 */
SupportsTouch = (function() {
	try {
		document.createEvent("TouchEvent");
		return true;
	} catch (ex) {
		return false;
	}
})();

/**
 * ScrollElement class. Set up an element to scroll by dragging the content
 * @param {Element} el The element to enable scrolling for
 */
ScrollElement = function(el) {
	this.element = el;
	this.offset = {
		x : 0,
		y : 0
	};
	var ev = ScrollElement.event;
	el.addEventListener(ev.TOUCHSTART, this.touchStart.bind(this), false);
	el.addEventListener(ScrollElement.event.TOUCHMOVE, this.touchMove
			.bind(this), false);
	el.addEventListener(ev.TOUCHEND, this.touchEnd.bind(this), false);
	el.addEventListener("webkitTransitionEnd", this.transitionEnd.bind(this),
			false);

	var tr = ScrollElement.transition;
	el.style.webkitTransitionProperty = tr.PROPERTY;
	el.style.webkitTransitionDuration = tr.DURATION;
};
ScrollElement.prototype = {};

/**
 * Set up which type of events to listen for.
 */
ScrollElement.event = {
	TOUCHSTART : SupportsTouch ? "touchstart" : "mousedown",
	TOUCHMOVE : SupportsTouch ? "touchmove" : "mousemove",
	TOUCHEND : SupportsTouch ? "touchend" : "mouseup"
};

/**
 * Allow horizontal scrolling
 * @type {Boolean}
 */
ScrollElement.scrollX = false;

/**
 * Allow vertical scrolling
 * @type {Boolean}
 */
ScrollElement.scrollY = true;

/**
 * Frame rate described as milliseconds per frame
 * @type {number}
 */
ScrollElement.transitionDuration = 1000 / 17;

/**
 * The transition method to use
 */
ScrollElement.transition = {
	PROPERTY : "-webkit-transform",
	DURATION : ScrollElement.transitionDuration + "ms"
};

/**
 * Offset property 
 * @param {{ x: number, y: number, t: date }} o An object describing 
 */
ScrollElement.prototype.setOffset = function(o) {
	this.offset.x = ScrollElement.scrollX ? (o.x < this.maxX) ? this.maxX
			: (o.x > 0) ? 0 : o.x : 0;
	this.offset.y = ScrollElement.scrollY ? (o.y < this.maxY) ? this.maxY
			: (o.y > 0) ? 0 : o.y : 0;
	this.offset.t = o.t || new Date;
};
ScrollElement.prototype.getOffset = function() {
	var o = this.offset;
	// clone object values to make sure they are not overwritten
	return {
		x : o.x,
		y : o.y,
		t : o.t
	};
};

/**
 * Velocity property
 * @param {number} x horizontal velocity
 * @param {number} y vertical velocity
 */
ScrollElement.prototype.setVelocity = function(x, y) {
	this.velocity = { // The speed in pixels per millisecond
		x : x,
		y : y
	};
};
ScrollElement.prototype.getVelocity = function(posOld, posNew) {
	return this.velocity;
};

/**
 * Calculate and set Velocity
 * @param {Object} posOld
 * @param {Object} posNew
 */
ScrollElement.prototype.calculateAndSetVelocity = function(posOld, posNew) {
	var vx = posNew.x - posOld.x;
	var vy = posNew.y - posOld.y;
	var t = posNew.t - posOld.t;
	this.setVelocity(vx / t, vy / t);
};

/**
 * Handler for touch start event
 * @param {Event} e the current event
 */
ScrollElement.prototype.touchStart = function(e) {

	e.preventDefault();

	this.dragging = true;
	this.animating = false;
	this.setVelocity(0, 0);

	var el = this.element;
	var parent = el.parentElement;

	var w = el.clientWidth;
	var h = el.clientHeight;
	var pw = parent.clientWidth;
	var ph = parent.clientHeight;
	this.maxX = pw - w;
	this.maxY = ph - h;

	var x = SupportsTouch ? e.touches[0].pageX : e.pageX;
	var y = SupportsTouch ? e.touches[0].pageY : e.pageY;

	this.startOffset = {
		x : this.offset.x,
		y : this.offset.y
	};
	this.dragStart = {
		x : x,
		y : y,
		t : new Date
	};

};

/**
 * Handler for touch move event. Called while dragging
 *  @param {Event} e the current event
 */
ScrollElement.prototype.touchMove = function(e) {

	e.preventDefault();

	if (this.dragging) {
		var old = this.getOffset();
		var x = SupportsTouch ? e.touches[0].pageX : e.pageX;
		var y = SupportsTouch ? e.touches[0].pageY : e.pageY;
		this.setOffset( {
			x : x - this.dragStart.x + this.startOffset.x,
			y : y - this.dragStart.y + this.startOffset.y
		});
		var current = this.getOffset();

		this.calculateAndSetVelocity(old, current);
		this.scroll();
	}
};

/**
 * Handler for touch end event.
 * @param {Event} e the current event
 */
ScrollElement.prototype.touchEnd = function(e) {

	e.preventDefault();

	this.dragging = false;
	this.deceleratingScroll();
};

/**
 * Handler for transition end event
 */
ScrollElement.prototype.transitionEnd = function() {
	if (this.animating) {
		this.deceleratingScroll();
	}
};

/**
 * Scroll to the next position
 */
ScrollElement.prototype.scroll = function() {
	var pos = this.getOffset();
	var y = pos.y;
	var x = pos.x;
	this.translate3d(x, y);
};

/**
 * Perform a decelerating scroll to the next position
 */
ScrollElement.prototype.deceleratingScroll = function() {
	this.animating = true;
	var v = this.getVelocity();
	var pos = this.getOffset();
	var x = v.x * ScrollElement.transitionDuration;
	var y = v.y * ScrollElement.transitionDuration;
	if (x % 1 || y % 1) {
		this.setOffset( {
			x : pos.x + x,
			y : pos.y + y
		});
		this.scroll();
		v.x = v.x * 0.9;
		v.y = v.y * 0.9;
	}
};

/**
 * Perform a (CSS) translate3d transformation to given position
 * @param {number} x horizontal position
 * @param {number} y vertical position
 * @param {number} z depth position
 */
ScrollElement.prototype.translate3d = function(x, y, z) {
	this.element.style.webkitTransform = [ "translate3d(", (x || 0), "px,",
			(y || 0), "px,", (z || 0), "px)" ].join("");
};
