#!/bin/bash
	for (( c=0; c<1571; c++ ))
	do
		ppm='out/PPM'$c'.PPM'
		png='out/IMG'$c'.png'
		convert $ppm $png
	done &
