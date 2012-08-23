#!/usr/bin/env /bin/bash

echo "set terminal png" > animate.gp
echo "set yrange [0:1000]" >> animate.gp
echo "set datafile separator \",\"" >> animate.gp

for frame in $(seq 0 255); do
    idx=$(printf "%04d" $frame)
    cat animate.gp.tmpl | sed -e "s:XXXX:$idx:g" >> animate.gp
done

gnuplot < animate.gp
ffmpeg -f image2 -i "frame/out%04d.png" -r 25 "animate.mov"
rm animate.gp
