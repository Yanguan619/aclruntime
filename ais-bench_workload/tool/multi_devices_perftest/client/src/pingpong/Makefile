INCLUDE:=./../../include
LIBS:=./../../lib64

ibv_client:pingpong.o rc_pingpong.o
	gcc -o ibv_client $^ -L$(LIBS) -libverbs -lnuma -lpthread

%.o:%.c
	gcc -Wall -I $(INCLUDE) -std=gnu99 -c -o $@ $^

clean:
	-rm *.o ibv_client

