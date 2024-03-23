INCLUDE:=./../../include
LIBS:=./../../lib64

ibv_rc_pingpong:pingpong.o rc_pingpong.o
	gcc -o ibv_rc_pingpong $^ -L$(LIBS) -libverbs

%.o:%.c
	gcc -Wall -I $(INCLUDE) -std=gnu99 -c -o $@ $^

clean:
	-rm *.o ibv_rc_pingpong

