#
# Exercise 2 - Makefile
# AUTHORS  Klaus Naumann
#          Christoph Klein
#          Günther Schindler
#
CC = mpicc
CFLAGS = -Wall
TEXC = pdflatex
DOCUNAME = docu_2

matrixmultiply :
	gcc $(CFLAGS)	src/part_3/main.c \
			src/part_3/matrix_multiply.c \
			src/part_3/time_measurement.c \
			-o matrixmultiply

barrier_synch :
	$(CC) $(CFLAGS)	src/part_2/barrier_synch.c \
			-o barrier_synch

ring_communication:
	$(CC) $(CFLAGS) src/part_1/main.c -o ring_communication

docu :
	$(TEXC) $(DOCUNAME).tex

clean :
	rm -f 	matrixmultiply \
		$(DOCUNAME).aux $(DOCUNAME).log $(DOCUNAME).pdf $(DOCUNAME).toc
	rm -f   barrier_synch \
		$(DOCUNAME).aux $(DOCUNAME).log $(DOCUNAME).pdf $(DOCUNAME).toc
	rm -f   ring_communication
