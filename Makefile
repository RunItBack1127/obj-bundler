CC=gcc
CFLAGS=-g -Wall
RM=rm
TARGET=objbundler

all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) -o $(TARGET) $(TARGET).c
	
clean:
	$(RM) $(TARGET)