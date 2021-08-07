#!/bin/bash
	killall 'RunVulkanAutomataSDL'
	killall 'buildrun.sh'
	killall 'script_buildrun.sh'
	killall 'RunVkAuto'
	killall 'buildrun'
	killall 'script_buildrun'

	kill $(pidof xfce4-panel)
	killall xfce4-panel
	xfce4-panel &
	disown

	kill $(pidof xfce4-taskmanager)
	killall xfce4-taskmanager
	xfce4-taskmanager

	kill $(pidof bash)
	killall bash


#	xfwm4 --replace
#	kill $(pidof xfwm4)
#	killall xfwm4
#	xfwm4
