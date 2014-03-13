#!/bin/sh
ifconfig wlan1 10.10.10.1/24
hostapd hostapd-touchwax.conf
