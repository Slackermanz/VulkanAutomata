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
	v1="vid/VKAutomata"$sf"_CRF28.mp4"
	v2="vid/VKAutomata"$sf"_SCALE.mp4"
	v3="gif/VKAutomata"$sf"_TWIT0.gif"
	v4="gif/VKAutomata"$sf"_TWIT1.gif"
	v5="gif/VKAutomata"$sf"_TWIT2.gif"
	v6="gif/VKAutomata"$sf"_TWIT3.gif"
	v7="gif/VKAutomata"$sf"_TWIT4.gif"
	v8="gif/VKAutomata"$sf"_TWIT5.gif"
	v9="gif/VKAutomata"$sf"_TWIT6.gif"
	gs="gif/VKAutomata"$sf"_GSIZE.gif"

	echo "                  [1/3] "$v0
	ffmpeg -loglevel 4 -framerate 60 -i out/IMG%00d.PAM -c:v libx264 -crf 18 -movflags +faststart -vf 'colorchannelmixer=rr=0:rb=1:br=1:bb=0' -pix_fmt yuv420p $v0
	echo "                  [2/3] "$v1
	ffmpeg -loglevel 4 -framerate 60 -i out/IMG%00d.PAM -c:v libx264 -crf 28 -movflags +faststart -vf 'colorchannelmixer=rr=0:rb=1:br=1:bb=0' -pix_fmt yuv420p $v1
	echo "                  [3/3] "$v2
	ffmpeg -loglevel 4 -framerate 60 -i out/IMG%00d.PAM -c:v libx264 -crf 18 -movflags +faststart -vf 'scale=iw*2:ih*2:flags=neighbor, colorchannelmixer=rr=0:rb=1:br=1:bb=0' -pix_fmt yuv420p $v2

	echo "                  [1/1] "$gs
	ffmpeg -loglevel 4 -framerate 30 -start_number 0 -i out/IMG%00d.PAM -vf "colorchannelmixer=rr=0:rb=1:br=1:bb=0, select='not(mod(n,2))',setpts=N/30/TB" $gs

	echo "                  [1/7] "$v3
	ffmpeg -loglevel 4 -framerate 30 -start_number 0 -i out/IMG%00d.PAM -frames:v 150 -vf "colorchannelmixer=rr=0:rb=1:br=1:bb=0, select='not(mod(n,2))',setpts=N/30/TB" $v3
	echo "                  [2/7] "$v4
	ffmpeg -loglevel 4 -framerate 30 -start_number 300 -i out/IMG%00d.PAM -frames:v 150 -vf "colorchannelmixer=rr=0:rb=1:br=1:bb=0, select='not(mod(n,2))',setpts=N/30/TB" $v4
	echo "                  [3/7] "$v5
	ffmpeg -loglevel 4 -framerate 30 -start_number 600 -i out/IMG%00d.PAM -frames:v 150 -vf "colorchannelmixer=rr=0:rb=1:br=1:bb=0, select='not(mod(n,2))',setpts=N/30/TB" $v5
	echo "                  [4/7] "$v6
	ffmpeg -loglevel 4 -framerate 30 -start_number 0 -i out/IMG%00d.PAM -frames:v 300 -vf "colorchannelmixer=rr=0:rb=1:br=1:bb=0, select='not(mod(n,2))',setpts=N/30/TB" $v6
	echo "                  [5/7] "$v7
	ffmpeg -loglevel 4 -framerate 30 -start_number 600 -i out/IMG%00d.PAM -frames:v 300 -vf "colorchannelmixer=rr=0:rb=1:br=1:bb=0, select='not(mod(n,2))',setpts=N/30/TB" $v7
	echo "                  [6/7] "$v8
	ffmpeg -loglevel 4 -framerate 30 -start_number 1200 -i out/IMG%00d.PAM -frames:v 300 -vf "colorchannelmixer=rr=0:rb=1:br=1:bb=0, select='not(mod(n,2))',setpts=N/30/TB" $v8
	echo "                  [7/7] "$v9
	ffmpeg -loglevel 4 -framerate 20 -start_number 0 -i out/IMG%00d.PAM -frames:v 200 -vf "colorchannelmixer=rr=0:rb=1:br=1:bb=0, select='not(mod(n,4))',setpts=N/30/TB" $v9
