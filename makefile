sofia: somma.o moltiplicazione.o carlo.o
	gcc -o sofia somma.o moltiplicazione.o carlo.o
master.o: master.c
	gcc -c master.c
moltiplicazione.o: moltiplicazione.c moltiplicazione.h
	gcc -c moltiplicazione.c
carlo.o: carlo.c 
	gcc -c carlo.c
clean:
	rm -f *.o
run:
	./sofia