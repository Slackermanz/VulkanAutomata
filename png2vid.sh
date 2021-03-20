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
	v0="vid/VKAutomata"$sf".mp4"
	v1="vid/VKAutomata"$sf"_CRF32.mp4"
	v2="vid/VKAutomata"$sf"_scale.mp4"
	echo "                  [1/3] "$v0
	ffmpeg -loglevel 4 -framerate 24 -i out/PPM%00d.PAM -c:v libx264 -crf 22 -movflags +faststart -vf 'colorchannelmixer=rr=0:rb=1:br=1:bb=0' -pix_fmt yuv420p $v0
	echo "                  [2/3] "$v1
	ffmpeg -loglevel 4 -framerate 24 -i out/PPM%00d.PAM -c:v libx264 -crf 32 -movflags +faststart -vf 'colorchannelmixer=rr=0:rb=1:br=1:bb=0' -pix_fmt yuv420p $v1
	echo "                  [3/3] "$v2
	ffmpeg -loglevel 4 -framerate 24 -i out/PPM%00d.PAM -c:v libx264 -crf 22 -movflags +faststart -vf 'scale=1024:576:flags=neighbor, colorchannelmixer=rr=0:rb=1:br=1:bb=0' -pix_fmt yuv420p $v2
