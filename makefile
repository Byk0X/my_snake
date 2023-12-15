all: main

main: main.o screen.o
	g++ -g -Wall -pedantic $^ -o $@ -lncurses

main.o: main.cpp screen.h cpoint.h
	g++ -g -c -Wall  -pedantic $< -o $@

screen.o: screen.cpp screen.h
	g++ -g -c -Wall  -pedantic $< -o $@

.PHONY: clean

clean:
	-rm main main.o screen.o
