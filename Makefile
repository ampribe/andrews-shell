CC = g++
CFLAGS = -Wall -g
ash: ash.cpp
	$(CC) $(CFLAGS) ash.c -o ash
clean: 
	rm -f ash *~
