all:	grid.pdf average.pdf

clean:
	rm *.pdf *.plot

grid.pdf:	sfc-grid.plot
	gnuplot samplingbias-grid.plt

average.pdf:	sfc.plot
	gnuplot samplingbias2.plt

sfc-grid.plot:	0000020000.ppm
	./ppmToGnuPlot.pl 0000020000.ppm > sfc-grid.plot

sfc.plot:	0000020000-average.ppm
	./ppmToGnuPlot.pl 0000020000-average.ppm > sfc.plot