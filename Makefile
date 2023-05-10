CC=gcc
CFLAGS=-std=c99 -Wall -Wextra -g
LOAD=load_balancer
SERVER=server
HT=hash_table
LL=linked_list

.PHONY: build clean

build: tema2

tema2: main.o $(LOAD).o $(SERVER).o $(HT).o $(LL).o
	$(CC) $^ -o $@

main.o: main.c
	$(CC) $(CFLAGS) $^ -c

$(SERVER).o: $(SERVER).c $(SERVER).h
	$(CC) $(CFLAGS) $^ -c

$(LOAD).o: $(LOAD).c $(LOAD).h
	$(CC) $(CFLAGS) $^ -c

$(HT).o: $(HT).c $(HT).h
	$(CC) $(CFLAGS) $^ -c

$(LL).o: $(LL).c $(LL).h
	$(CC) $(CFLAGS) $^ -c

clean:
	rm -f *.o tema2 *.h.gch

pack:
	zip -FSr 311CA_StaicuDan-Dominic_Tema2	.zip README.md Makefile *.c *.h
