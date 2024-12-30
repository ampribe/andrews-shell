CC = clang++
CFLAGS = -Wall -g
ash: ash.cpp
	$(CC) $(CFLAGS) ash.cpp -o ash
clean: 
	rm -f ash *~
