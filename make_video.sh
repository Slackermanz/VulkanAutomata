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
	v1="vid/VKAutomata"$sf"_CRF24.mp4"
	v2="vid/VKAutomata"$sf"_SCALE.mp4"
	gs="gif/VKAutomata"$sf"_GSIZE.gif"
	gf="gif/VKAutomata"$sf"_GFULL.gif"

	echo "                  [1/3] "$v0
	ffmpeg -loglevel 4 -framerate 60 -i out/IMG%00d.PAM -c:v libx264 -crf 18 -movflags +faststart -pix_fmt yuv420p $v0
	echo "                  [2/3] "$v1
	ffmpeg -loglevel 4 -framerate 60 -i out/IMG%00d.PAM -c:v libx264 -crf 24 -movflags +faststart -pix_fmt yuv420p $v1
	echo "                  [3/3] "$v2
	ffmpeg -loglevel 4 -framerate 60 -i out/IMG%00d.PAM -c:v libx264 -crf 20 -movflags +faststart -vf 'scale=iw*4:ih*4:flags=neighbor' -pix_fmt yuv420p $v2

	echo "                  [1/2] "$gs
	ffmpeg -loglevel 4 -framerate 30 -i out/IMG%00d.PAM -vf "select='not(mod(n,1))',setpts=N/30/TB" $gs

	gsize=$(ls -s $gs | cut -d' ' -f 1)
	files=$(ls ./out | wc -l)
	let gsmod=($gsize/12288)+1

	echo "                  [2/2] "$gf
	ffmpeg -loglevel 4 -framerate 30 -i out/IMG%00d.PAM -vf "select='not(mod(n,"$gsmod"))',setpts=N/30/TB" $gf

	rm $gs
