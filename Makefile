CC := gcc
CFLAGS := -g -Wall -Wextra $(shell pkg-config --cflags fuse3 libzip collectionc)
LIBS := $(shell pkg-config --libs fuse3 libzip collectionc)
OBJECTS := main.o fs_tree.o log.o
TARGET := cfs

.PHONY: all
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f $(TARGET)
	rm -f *.o
