// Converted from UnityScript to C# at http://www.M2H.nl/files/js_to_c.php - by Mike Hergaarden
// Do test the code! You usually need to change a few small bits.

using UnityEngine;
using System.Collections;

public class MYCLASSNAME : MonoBehaviour {
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
		FIXME_VAR_TYPE fun= this;
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
	FIXME_VAR_TYPE ev= ScrollElement.event;
	el.addEventListener(ev.TOUCHSTART, this.touchStart.bind(this), false);
	el.addEventListener(ScrollElement.event.TOUCHMOVE, this.touchMove
			.bind(this), false);
	el.addEventListener(ev.TOUCHEND, this.touchEnd.bind(this), false);
	el.addEventListener("webkitTransitionEnd", this.transitionEnd.bind(this),
			false);

	FIXME_VAR_TYPE tr= ScrollElement.transition;
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
	FIXME_VAR_TYPE o= this.offset;
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
	FIXME_VAR_TYPE vx= posNew.x - posOld.x;
	FIXME_VAR_TYPE vy= posNew.y - posOld.y;
	FIXME_VAR_TYPE t= posNew.t - posOld.t;
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

	FIXME_VAR_TYPE el= this.element;
	FIXME_VAR_TYPE parent= el.parentElement;

	FIXME_VAR_TYPE w= el.clientWidth;
	FIXME_VAR_TYPE h= el.clientHeight;
	FIXME_VAR_TYPE pw= parent.clientWidth;
	FIXME_VAR_TYPE ph= parent.clientHeight;
	this.maxX = pw - w;
	this.maxY = ph - h;

	FIXME_VAR_TYPE x= SupportsTouch ? e.touches[0].pageX : e.pageX;
	FIXME_VAR_TYPE y= SupportsTouch ? e.touches[0].pageY : e.pageY;

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
		FIXME_VAR_TYPE old= this.getOffset();
		FIXME_VAR_TYPE x= SupportsTouch ? e.touches[0].pageX : e.pageX;
		FIXME_VAR_TYPE y= SupportsTouch ? e.touches[0].pageY : e.pageY;
		this.setOffset( {
			x : x - this.dragStart.x + this.startOffset.x,
			y : y - this.dragStart.y + this.startOffset.y
		});
		FIXME_VAR_TYPE current= this.getOffset();

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
	FIXME_VAR_TYPE pos= this.getOffset();
	FIXME_VAR_TYPE y= pos.y;
	FIXME_VAR_TYPE x= pos.x;
	this.translate3d(x, y);
};

/**
 * Perform a decelerating scroll to the next position
 */
ScrollElement.prototype.deceleratingScroll = function() {
	this.animating = true;
	FIXME_VAR_TYPE v= this.getVelocity();
	FIXME_VAR_TYPE pos= this.getOffset();
	FIXME_VAR_TYPE x= v.x * ScrollElement.transitionDuration;
	FIXME_VAR_TYPE y= v.y * ScrollElement.transitionDuration;
	if (x % 1 || y % 1) {
		this.setOffset( {
			x : pos.x + x,
			y : pos.y + y
		});
		this.scroll();
		v.x = v.x * 0.9f;
		v.y = v.y * 0.9f;
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

}
