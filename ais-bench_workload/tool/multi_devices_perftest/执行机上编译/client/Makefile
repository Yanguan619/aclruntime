ibv_client:client.o rc_client.o
	gcc -o ibv_client $^ -libverbs -lnuma -lpthread

%.o:%.c
	gcc -Wall -std=gnu99 -c -o $@ $^

clean:
	-rm *.o ibv_client


