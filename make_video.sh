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

	th="out/IMG60.PAM"
	if test -f $th; then
		cp $th "out/IMG0.PAM"
	fi

	v0="vid/VKAutomata"$sf".mp4"
	v1="vid/VKAutomata"$sf"_SCALE.mp4"
	v2="vid/VKAutomata"$sf"_1024.mp4"
	echo "                  [1/5] "$v0
	ffmpeg -loglevel 4 -framerate 60 -i out/IMG%00d.PAM -c:v libx264 -crf 16 -movflags +faststart -pix_fmt yuv420p $v0
	echo "                  [2/5] "$v1
	ffmpeg -loglevel 4 -framerate 60 -i out/IMG%00d.PAM -s 4096x2048 -sws_flags neighbor -c:v libx264 -crf 18 -movflags +faststart -pix_fmt yuv420p $v1
	echo "                  [3/5] "$v2
	ffmpeg -loglevel 4 -framerate 60 -i out/IMG%00d.PAM -s 1024x512 -sws_flags neighbor -c:v libx264 -crf 16 -movflags +faststart -pix_fmt yuv420p $v2

	gs="gif/VKAutomata"$sf"_GSIZE.gif"
	gf="gif/VKAutomata"$sf"_GFULL.gif"
	echo "                  [4/5] "$gs
	ffmpeg -loglevel 4 -i $v0 -lavfi palettegen=reserve_transparent=0:stats_mode=single[pal],[0:v][pal]paletteuse=new=1,fps=fps=30 $gs
	gsize=$(ls -s $gs | cut -d' ' -f 1)
	files=$(ls ./out | wc -l)
	let gsmod=($gsize/13312)+1
	echo "                  [5/5] "$gf
	ffmpeg -loglevel 4 -i $v0 -lavfi palettegen=reserve_transparent=0:stats_mode=single[pal],[0:v][pal]paletteuse=new=1,fps=fps=30,"select=not(mod(n\,"$gsmod"))*gt(n\,12)",setpts=N/30/TB $gf
	rm $gs

	echo "  Video Creation Complete!"
	echo ""
