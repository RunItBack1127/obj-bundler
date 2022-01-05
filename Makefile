CC=gcc
CFLAGS=-g -Wall
RM=rm
DIR=src
TARGET_NAME=objbundler
TARGET_DEP=obj_bundler

all: $(TARGET_NAME)

$(TARGET_NAME): $(TARGET_DEP).c
	$(CC) -o $(TARGET_NAME) $(DIR)/$(TARGET_DEP).c
	
clean:
	$(RM) $(DIR)/$(TARGET_NAME)