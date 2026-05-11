CC = gcc
CFLAGS = -std=c99 -Wall
TARGET = csvreader.exe
OBJS = main.o table.o comput.o utils.o
LIBS = -lm

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

main.o: main.c table.h comput.h
	$(CC) $(CFLAGS) -c main.c

table.o: table.c table.h utils.h
	$(CC) $(CFLAGS) -c table.c

comput.o: comput.c comput.h table.h
	$(CC) $(CFLAGS) -c comput.c

utils.o: utils.c utils.h
	$(CC) $(CFLAGS) -c utils.c

clean:
	del $(TARGET) $(OBJS) 2>nul || rm -f $(TARGET) $(OBJS)

.PHONY: clean