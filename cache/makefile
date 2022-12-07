TARGET	=	cache
S_SRCS 	=	$(wildcard src/*.c)
OBJS	=	$(patsubst src/%.c, src/%.o, $(S_SRCS))

CFLAGS	+=	-std=c11 -Isrc -L../innobase/buf -lbuf -I../innobase/include
CFLAGS 	+= 	-D__LINUX__
CFLAGS	+=  -ggdb3 -O0 -w -fpermissive
USEDE	=	-DDEBUG

CC		=	gcc

$(TARGET):$(OBJS)
	$(CC) $(CFLAGS) $(USEDE) $(OBJS) -o $@ ../innobase/usr/libusr.a ../innobase/buf/libbuf.a

%.o: %.cpp
	$(CC) -c $^ $(CFLAGS) $(USEDE)  -o $@


.PHONY : clean

clean:
	rm $(TARGET) -f
	rm $(OBJS) -f

