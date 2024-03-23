ibv_server:server.o rc_server.o
	gcc -o ibv_server $^ -libverbs -lnuma -lpthread

%.o:%.c
	gcc -Wall -std=gnu99 -c -o $@ $^

clean:
	-rm *.o ibv_server


