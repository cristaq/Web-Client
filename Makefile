CC=gcc
CFLAGS=-I.

client: client.c requests.c helpers.c buffer.c
	$(CC) -o client client.c requests.c helpers.c buffer.c cJSON.c -Wall

run: client
	./client

clean:
	rm -f *.o client
