CC=gcc
CFLAGS=-g -Wall
RM=rm
DIR=src
TARGET=objbundler

all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) -o $(TARGET) $(DIR)/$(TARGET).c
	
clean:
	$(RM) $(TARGET)