#!/bin/bash
	starttime=$(date +%s)

#	Get filename
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

#	Set inital "Thumbnail" frame, create seperate slides from potential thumnail frames
	th="out/IMG0.PAM"
	if test -f $th; then
		cp $th "out/SCR_INITAL.PAM"
	fi
	th="out/IMG1.PAM"
	if test -f $th; then
		cp $th "out/IMG0.PAM"
		cp $th "out/SCR0.PAM"
	fi
	th="out/IMG10.PAM"
	if test -f $th; then
		cp $th "out/IMG0.PAM"
		cp $th "out/SCR1.PAM"
	fi
	th="out/IMG60.PAM"
	if test -f $th; then
		cp $th "out/IMG0.PAM"
		cp $th "out/SCR2.PAM"
	fi
	th="out/IMG180.PAM"
	if test -f $th; then
		cp $th "out/IMG0.PAM"
		cp $th "out/SCR3.PAM"
	fi
	th="out/IMG600.PAM"
	if test -f $th; then
		cp $th "out/IMG0.PAM"
		cp $th "out/SCR4.PAM"
	fi
	th="out/IMG2400.PAM"
	if test -f $th; then
		cp $th "out/IMG0.PAM"
		cp $th "out/SCR5.PAM"
	fi
	th="out/IMG6400.PAM"
	if test -f $th; then
		cp $th "out/IMG0.PAM"
		cp $th "out/SCR6.PAM"
	fi
	th="out/IMG18000.PAM"
	if test -f $th; then
		cp $th "out/IMG0.PAM"
		cp $th "out/SCR7.PAM"
	fi

	ffmpeg -loglevel 4 -i out/SCR%00d.PAM "img/VKAutomata$sf"SCR%00d.png

#	Video output filepaths/filenames
	v0="vid/VKAutomata"$sf".mp4"

	v1="vid/VKAutomata"$sf"_SCALE.mp4"
	v2="vid/VKAutomata"$sf"_SMALL.mp4"

	v3="vid/VKAutomata"$sf"_4096x2048.mp4"
	v4="vid/VKAutomata"$sf"_2048x1024.mp4"
	v5="vid/VKAutomata"$sf"_1024x512.mp4"
	v6="vid/VKAutomata"$sf"_512x256.mp4"
	v7="vid/VKAutomata"$sf"_256x128.mp4"

#	Create MP4 videos
	buildtime=$(date +%s)
	echo "              VID [1/8] "$v0
	ffmpeg -loglevel 4 -framerate 60 -i out/IMG%00d.PAM -c:v libx264 -crf 12 -movflags +faststart -vf 'eq=gamma_r=1.26:gamma_g=1.19:gamma_b=1.08' -pix_fmt yuv420p $v0
	echo "                  Seconds elapsed: "$(($(date +%s)-$buildtime))

	buildtime=$(date +%s)
	echo "              VID [2/8] "$v1
	ffmpeg -loglevel 4 -framerate 60 -i out/IMG%00d.PAM -c:v libx264 -crf 12 -movflags +faststart -vf 'scale=iw*4:ih*4:flags=neighbor,eq=gamma_r=1.26:gamma_g=1.19:gamma_b=1.08' -pix_fmt yuv420p $v1
	echo "                  Seconds elapsed: "$(($(date +%s)-$buildtime))

	buildtime=$(date +%s)
	echo "              VID [3/8] "$v2
	ffmpeg -loglevel 4 -framerate 60 -i out/IMG%00d.PAM -c:v libx264 -crf 12 -movflags +faststart -vf 'scale=iw/2:ih/2:flags=bicubic,eq=gamma_r=1.26:gamma_g=1.19:gamma_b=1.08' -pix_fmt yuv420p $v2
	echo "                  Seconds elapsed: "$(($(date +%s)-$buildtime))

	buildtime=$(date +%s)
	echo "              VID [4/8] "$v3
	ffmpeg -loglevel 4 -framerate 60 -i out/IMG%00d.PAM -s 4096x2048 -sws_flags neighbor -c:v libx264 -crf 14 -movflags +faststart -pix_fmt yuv420p $v3
	echo "                  Seconds elapsed: "$(($(date +%s)-$buildtime))

	buildtime=$(date +%s)
	echo "              VID [5/8] "$v4
	ffmpeg -loglevel 4 -framerate 60 -i out/IMG%00d.PAM -s 2048x1024 -sws_flags neighbor -c:v libx264 -crf 14 -movflags +faststart -pix_fmt yuv420p $v4
	echo "                  Seconds elapsed: "$(($(date +%s)-$buildtime))

	buildtime=$(date +%s)
	echo "              VID [6/8] "$v5
	ffmpeg -loglevel 4 -framerate 60 -i out/IMG%00d.PAM -s 1024x512 -sws_flags neighbor -c:v libx264 -crf 14 -movflags +faststart -pix_fmt yuv420p $v5
	echo "                  Seconds elapsed: "$(($(date +%s)-$buildtime))

	buildtime=$(date +%s)
	echo "              VID [7/8] "$v6
	ffmpeg -loglevel 4 -framerate 60 -i out/IMG%00d.PAM -s 512x256 -sws_flags bicubic -c:v libx264 -crf 14 -movflags +faststart -pix_fmt yuv420p $v6
	echo "                  Seconds elapsed: "$(($(date +%s)-$buildtime))

	buildtime=$(date +%s)
	echo "              VID [8/8] "$v7
	ffmpeg -loglevel 4 -framerate 60 -i out/IMG%00d.PAM -s 256x128 -sws_flags bicubic -c:v libx264 -crf 14 -movflags +faststart -pix_fmt yuv420p $v7
	echo "                  Seconds elapsed: "$(($(date +%s)-$buildtime))


#	Temporary "Size" GIF output filepaths/filenames
	gs="gif/VKAutomata"$sf"_GSIZE.gif"
	gss="gif/VKAutomata"$sf"_GSSML.gif"

#	GIF output filepaths/filenames
	gf="gif/VKAutomata"$sf"_GFULL.gif"
	gc="gif/VKAutomata"$sf"_GCLIP.gif"

	gfs="gif/VKAutomata"$sf"_GFSML.gif"
	gcs="gif/VKAutomata"$sf"_GCSML.gif"

#	Get the filesize of the GIF
	buildtime=$(date +%s)
	echo "         SIZE GIF [1/2] "$gs
	ffmpeg -loglevel 4 -i $v0 -lavfi palettegen=reserve_transparent=0:stats_mode=single[pal],[0:v][pal]paletteuse=new=1,fps=fps=30 $gs
	gsize=$(ls -s $gs | cut -d' ' -f 1)
	files=$(ls ./out | wc -l)
	let gsmod=($gsize/13312)+1
	echo "                  Seconds elapsed: "$(($(date +%s)-$buildtime))

#	Create GIFs
	buildtime=$(date +%s)
	echo "              GIF [1/4] "$gf
	ffmpeg -loglevel 4 -i $v0 -lavfi palettegen=reserve_transparent=0:stats_mode=single[pal],[0:v][pal]paletteuse=new=1,fps=fps=30,"select=not(mod(n\,"$gsmod"))*gt(n\,0)",setpts=N/30/TB $gf
	echo "                  Seconds elapsed: "$(($(date +%s)-$buildtime))

	buildtime=$(date +%s)
	echo "              GIF [2/4] "$gc
	ffmpeg -loglevel 4 -i $v0 -lavfi palettegen=reserve_transparent=0:stats_mode=single[pal],[0:v][pal]paletteuse=new=1,fps=fps=30,"select=not(mod(n\,"$gsmod"))*gt(n\,12)",setpts=N/30/TB $gc
	echo "                  Seconds elapsed: "$(($(date +%s)-$buildtime))

#	Get the filesize of the GIF
	buildtime=$(date +%s)
	echo "         SIZE GIF [2/2] "$gss
	ffmpeg -loglevel 4 -i $v2 -lavfi palettegen=reserve_transparent=0:stats_mode=single[pal],[0:v][pal]paletteuse=new=1,fps=fps=30 $gss
	gssml=$(ls -s $gss | cut -d' ' -f 1)
	flsml=$(ls ./out | wc -l)
	let gsmod=($gssml/13312)+1
	echo "                  Seconds elapsed: "$(($(date +%s)-$buildtime))

#	Create GIFs
	buildtime=$(date +%s)
	echo "              GIF [3/4] "$gfs
	ffmpeg -loglevel 4 -i $v2 -lavfi palettegen=reserve_transparent=0:stats_mode=single[pal],[0:v][pal]paletteuse=new=1,fps=fps=30,"select=not(mod(n\,"$gsmod"))*gt(n\,0)",setpts=N/30/TB $gfs
	echo "                  Seconds elapsed: "$(($(date +%s)-$buildtime))

	buildtime=$(date +%s)
	echo "              GIF [4/4] "$gcs
	ffmpeg -loglevel 4 -i $v2 -lavfi palettegen=reserve_transparent=0:stats_mode=single[pal],[0:v][pal]paletteuse=new=1,fps=fps=30,"select=not(mod(n\,"$gsmod"))*gt(n\,12)",setpts=N/30/TB $gcs
	echo "                  Seconds elapsed: "$(($(date +%s)-$buildtime))

#	Delete temporary "Size" GIF files (cleanup)
	echo "REMOVING TEMP GIF [1/2] "$gs
	rm $gs
	echo "REMOVING TEMP GIF [2/2] "$gss
	rm $gss

#	Hope and pray that it worked!
	echo "  Video Creation Complete!"
	echo "  Total Seconds elapsed: "$(($(date +%s)-$starttime))
	echo ""


