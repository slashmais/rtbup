#=====
CC = g++
LD = g++
CFLAGS = -Wall -pthread
LDFLAGS = -lpthread
RM = /bin/rm -f
SRCS = main.cpp rtbup.cpp monitor.cpp globs.cpp rthelp.cpp tparm.cpp
OBJS = main.o rtbup.o monitor.o globs.o rthelp.o tparm.o
PROG = rtbup
#=====
$(PROG): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(PROG)
#=====
%.o: %.cpp
	$(CC) $(CFLAGS) -c $<
#=====
clean:
	$(RM) $(PROG) $(OBJS)