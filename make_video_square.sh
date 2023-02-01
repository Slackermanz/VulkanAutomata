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
	vz0="vid/VKAutomata"$sf"_120hz.mp4"
	vz1="vid/VKAutomata"$sf"_144hz.mp4"
	v1="vid/VKAutomata"$sf"_SCALE.mp4"
	v2="vid/VKAutomata"$sf"_SMALL.mp4"

	v6="vid/VKAutomata"$sf"_256x256.mp4"
	v7="vid/VKAutomata"$sf"_512x512.mp4"
	v8="vid/VKAutomata"$sf"_4096x4096.mp4"
	v10="vid/VKAutomata"$sf"_2048x2048.mp4"
	v9="vid/VKAutomata"$sf"_1024x1024.mp4"

#	Create MP4 videos
	buildtime=$(date +%s)
	echo "              VID A [1/3] "$v0 $(date +%T)
#	ffmpeg -loglevel 4 -framerate 60 -i out/IMG%00d.PAM -c:v libx264 -crf 12 -movflags +faststart -vf 'eq=gamma_r=1.26:gamma_g=1.19:gamma_b=1.08' -pix_fmt yuv420p $v0
	ffmpeg -loglevel 4 -framerate 60 -i out/IMG%00d.PAM -c:v libx264 -crf 12 -movflags +faststart -pix_fmt yuv420p $v0
	echo "                  Seconds elapsed: "$(($(date +%s)-$buildtime))
#	ffmpeg -loglevel 4 -i $v0 -i input.wav -c:v copy -c:a aac -shortest "vid/VKAutomata"$sf"_Audio.mp4"

	buildtime=$(date +%s)
	echo "              VID A [2/3] "$vz0 $(date +%T)
#	ffmpeg -loglevel 4 -framerate 60 -i out/IMG%00d.PAM -c:v libx264 -crf 12 -movflags +faststart -vf 'eq=gamma_r=1.26:gamma_g=1.19:gamma_b=1.08' -pix_fmt yuv420p $v0
	ffmpeg -loglevel 4 -framerate 120 -i out/IMG%00d.PAM -c:v libx264 -crf 12 -movflags +faststart -pix_fmt yuv420p $vz0
	echo "                  Seconds elapsed: "$(($(date +%s)-$buildtime))
#	ffmpeg -loglevel 4 -i $v0 -i input.wav -c:v copy -c:a aac -shortest "vid/VKAutomata"$sf"_Audio.mp4"

	buildtime=$(date +%s)
	echo "              VID A [3/3] "$vz1 $(date +%T)
#	ffmpeg -loglevel 4 -framerate 60 -i out/IMG%00d.PAM -c:v libx264 -crf 12 -movflags +faststart -vf 'eq=gamma_r=1.26:gamma_g=1.19:gamma_b=1.08' -pix_fmt yuv420p $v0
	ffmpeg -loglevel 4 -framerate 144 -i out/IMG%00d.PAM -c:v libx264 -crf 12 -movflags +faststart -pix_fmt yuv420p $vz1
	echo "                  Seconds elapsed: "$(($(date +%s)-$buildtime))
#	ffmpeg -loglevel 4 -i $v0 -i input.wav -c:v copy -c:a aac -shortest "vid/VKAutomata"$sf"_Audio.mp4"

	buildtime=$(date +%s)
	echo "              VID B [1/2] "$v2 $(date +%T)
#	ffmpeg -loglevel 4 -framerate 60 -i out/IMG%00d.PAM -c:v libx264 -crf 12 -movflags +faststart -vf 'scale=iw/2:ih/2:flags=bicubic,eq=gamma_r=1.26:gamma_g=1.19:gamma_b=1.08' -pix_fmt yuv420p $v2
	ffmpeg -loglevel 4 -framerate 60 -i out/IMG%00d.PAM -c:v libx264 -crf 12 -movflags +faststart -vf 'scale=iw/2:ih/2:flags=bicubic' -pix_fmt yuv420p $v2
	echo "                  Seconds elapsed: "$(($(date +%s)-$buildtime))
#	ffmpeg -loglevel 4 -i $v2 -i input.wav -c:v copy -c:a aac -shortest "vid/VKAutomata"$sf"_SMALL_Audio.mp4"


	buildtime=$(date +%s)
	echo "              VID C [1/5] "$v6 $(date +%T)
	ffmpeg -loglevel 4 -framerate 60 -i out/IMG%00d.PAM -s 256x256 -sws_flags neighbor -c:v libx264 -crf 14 -movflags +faststart -pix_fmt yuv420p $v6
	echo "                  Seconds elapsed: "$(($(date +%s)-$buildtime))

	buildtime=$(date +%s)
	echo "              VID C [2/5] "$v7 $(date +%T)
	ffmpeg -loglevel 4 -framerate 60 -i out/IMG%00d.PAM -s 512x512 -sws_flags neighbor -c:v libx264 -crf 14 -movflags +faststart -pix_fmt yuv420p $v7
	echo "                  Seconds elapsed: "$(($(date +%s)-$buildtime))

	buildtime=$(date +%s)
	echo "              VID C [3/5] "$v9 $(date +%T)
	ffmpeg -loglevel 4 -framerate 60 -i out/IMG%00d.PAM -s 1024x1024 -sws_flags neighbor -c:v libx264 -crf 14 -movflags +faststart -pix_fmt yuv420p $v9
	echo "                  Seconds elapsed: "$(($(date +%s)-$buildtime))

	buildtime=$(date +%s)
	echo "              VID C [4/5] "$v10 $(date +%T)
	ffmpeg -loglevel 4 -framerate 60 -i out/IMG%00d.PAM -s 2048x2048 -sws_flags neighbor -c:v libx264 -crf 14 -movflags +faststart -pix_fmt yuv420p $v10
	echo "                  Seconds elapsed: "$(($(date +%s)-$buildtime))

	buildtime=$(date +%s)
	echo "              VID B [2/2] "$v1 $(date +%T)
#	ffmpeg -loglevel 4 -framerate 60 -i out/IMG%00d.PAM -c:v libx264 -crf 12 -movflags +faststart -vf 'scale=iw*4:ih*4:flags=neighbor,eq=gamma_r=1.26:gamma_g=1.19:gamma_b=1.08' -pix_fmt yuv420p $v1
	ffmpeg -loglevel 4 -framerate 60 -i out/IMG%00d.PAM -c:v libx264 -crf 12 -movflags +faststart -vf 'scale=iw*4:ih*4:flags=neighbor' -pix_fmt yuv420p $v1
	echo "                  Seconds elapsed: "$(($(date +%s)-$buildtime))
#	ffmpeg -loglevel 4 -i $v1 -i input.wav -c:v copy -c:a aac -shortest "vid/VKAutomata"$sf"_SCALE_Audio.mp4"

	buildtime=$(date +%s)
	echo "              VID C [5/5] "$v8 $(date +%T)
	ffmpeg -loglevel 4 -framerate 60 -i out/IMG%00d.PAM -s 4096x4096 -sws_flags neighbor -c:v libx264 -crf 14 -movflags +faststart -pix_fmt yuv420p $v8
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
	echo "         SIZE GIF [1/2] "$gss
#	ffmpeg -loglevel 4 -i $v2 -lavfi palettegen=reserve_transparent=0:stats_mode=single[pal],[0:v][pal]paletteuse=new=1,fps=fps=30 $gss
	gssml=$(ls -s $gss | cut -d' ' -f 1)
	flsml=$(ls ./out | wc -l)
	let gsmod=($gssml/13312)+1
	echo "                  Seconds elapsed: "$(($(date +%s)-$buildtime))

#	Create GIFs
	buildtime=$(date +%s)
	echo "              GIF [1/4] "$gfs
#	ffmpeg -loglevel 4 -i $v2 -lavfi palettegen=reserve_transparent=0:stats_mode=single[pal],[0:v][pal]paletteuse=new=1,fps=fps=30,"select=not(mod(n\,"$gsmod"))*gt(n\,0)",setpts=N/30/TB $gfs
	echo "                  Seconds elapsed: "$(($(date +%s)-$buildtime))

	buildtime=$(date +%s)
	echo "              GIF [2/4] "$gcs
	ffmpeg -loglevel 4 -i $v2 -lavfi palettegen=reserve_transparent=0:stats_mode=single[pal],[0:v][pal]paletteuse=new=1,fps=fps=30,"select=not(mod(n\,"$gsmod"))*gt(n\,12)",setpts=N/30/TB $gcs
	echo "                  Seconds elapsed: "$(($(date +%s)-$buildtime))

#	Get the filesize of the GIF
	buildtime=$(date +%s)
	echo "         SIZE GIF [2/2] "$gs
#	ffmpeg -loglevel 4 -i $v0 -lavfi palettegen=reserve_transparent=0:stats_mode=single[pal],[0:v][pal]paletteuse=new=1,fps=fps=30 $gs
	gsize=$(ls -s $gs | cut -d' ' -f 1)
	files=$(ls ./out | wc -l)
	let gsmod=($gsize/13312)+1
	echo "                  Seconds elapsed: "$(($(date +%s)-$buildtime))

#	Create GIFs
	buildtime=$(date +%s)
	echo "              GIF [3/4] "$gf
#	ffmpeg -loglevel 4 -i $v0 -lavfi palettegen=reserve_transparent=0:stats_mode=single[pal],[0:v][pal]paletteuse=new=1,fps=fps=30,"select=not(mod(n\,"$gsmod"))*gt(n\,0)",setpts=N/30/TB $gf
	echo "                  Seconds elapsed: "$(($(date +%s)-$buildtime))

	buildtime=$(date +%s)
	echo "              GIF [4/4] "$gc
#	ffmpeg -loglevel 4 -i $v0 -lavfi palettegen=reserve_transparent=0:stats_mode=single[pal],[0:v][pal]paletteuse=new=1,fps=fps=30,"select=not(mod(n\,"$gsmod"))*gt(n\,12)",setpts=N/30/TB $gc
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


