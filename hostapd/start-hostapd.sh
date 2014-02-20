#!/bin/sh
ifconfig wlan0 10.10.10.1/24
hostapd hostapd-touchwax.conf
