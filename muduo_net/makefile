CC=g++
CFLAGS+=-g -Wall -lmuduo_net -lmuduo_base -lpthread
OBJS=EventLoop_unittest.cc

eventlooptest: $(OBJS)
	$(CC) $^ $(CFLAGS) -o $@
