set title "Heat Map generated from a file containing Z values only"
unset key
set tic scale 0
 
set palette rgbformula 7,5,15
set cbrange [0:1000]
set cblabel "Score"
unset cbtics

set yrange [-0.05:0.55]
set xrange [-0.5:256.5]

set pm3d map
set datafile separator ","
splot "glth_out.dat" using 1:($2/256):($3*$3) with image