#!/bin/bash
	fn=$(basename -s .mp4 vid/VKAutomata[0-9][0-9][0-9][0-9].mp4)
	c0=${#fn}
	let c0=c0-3
	c1=${#fn}
	idn=$(echo $fn | cut -c$c0-$c1)
	idn=$(( 10#$idn ))
	let idn=idn+1
	let pad=4-${#idn}
	sf="0000"
	sf=$(echo $sf | cut -c1-$pad)
	sf=$sf$idn
	v1="vid/VKAutomata"$sf".mp4"
	v2="vid/VKAutomata"$sf"_CRF30.mp4"
	v3="vid/VKAutomata"$sf"_CRF34.mp4"
	v4="vid/VKAutomata"$sf"_CRF38.mp4"
	v5="vid/VKAutomata"$sf"_CRF42.mp4"
	echo "                  [1/5] "$v1
	ffmpeg -loglevel 4 -framerate 24 -i out/IMG%00d.png -c:v libx264 -crf 22 -movflags +faststart -vf format=yuv420p $v1
	echo "                  [2/5] "$v2
	ffmpeg -loglevel 4 -framerate 24 -i out/IMG%00d.png -c:v libx264 -crf 30 -movflags +faststart -vf format=yuv420p $v2
	echo "                  [3/5] "$v3
	ffmpeg -loglevel 4 -framerate 24 -i out/IMG%00d.png -c:v libx264 -crf 34 -movflags +faststart -vf format=yuv420p $v3
	echo "                  [4/5] "$v4
	ffmpeg -loglevel 4 -framerate 24 -i out/IMG%00d.png -c:v libx264 -crf 38 -movflags +faststart -vf format=yuv420p $v4
	echo "                  [5/5] "$v5
	ffmpeg -loglevel 4 -framerate 24 -i out/IMG%00d.png -c:v libx264 -crf 42 -movflags +faststart -vf format=yuv420p $v5

