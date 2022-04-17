CC       = g++
CFLAGS   = -Wall -g -O -fPIC
CXXFLAGS =
INCLUDE  = -I ./
TARGET   = libsocketproxy.so
LIBPATH  = ./build/lib/

#vpath %.h

SRCS     = ${wildcard *.cpp}
OBJS     = ${patsubst %.cpp, %.o, $(SRCS)}

$(OBJS):$(SRCS)
	$(CC) $(CFLAGS) $(INCLUDE) -c $^
all:$(OBJS)
	$(CC) -shared -fPIC -o $(TARGET) $(OBJS)
	mv $(TARGET) $(LIBPATH)

clean:
	rm -f *.o
	rm -f $(LIBPATH)*
