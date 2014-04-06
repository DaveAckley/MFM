# set terminal postscript eps enhanced color size 3.5,2.75 "NimbusSanL-Regu,17" fontfile "/usr/share/texmf-texlive/fonts/type1/urw/helvetic/uhvr8a.pfb"
set terminal pdf
set output 'average.pdf'
set samples 32
set isosamples 32
set hidden3d
set title "Event rate vs site position\nDreg and Res, P1 Atom, N=32, R=4"
set key title ""
unset key
set xlabel "x_c"
set xtics 0,8
set ytics 0,8
# set xrange [0:32]
set ylabel "y_c"
# set yrange [0:32]
set zlabel "Events/sec (15 tile average)" rotate offset -1.3,0,0
# set zrange [80:120]
set xyplane 0
#set ztics ("to" 4000,"come" 5000, "soon" 6000, "now" 7000)
set ztics 0,10
set view 68,37
splot "sfc.plot" matrix with l lt 0
