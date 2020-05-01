CC := gcc
DEPS := fuse3 libzip collectionc
CFLAGS := -g -Wall -Wextra -Iinclude $(shell pkg-config --cflags $(DEPS))
LIBS := $(shell pkg-config --libs $(DEPS))

SOURCE_PATh := src
BUILD_PATH := _build
SOURCES := $(shell find $(SOURCE_PATH) -name *.c )
OBJECTS := $(SOURCES:$(SOURCE_PATH)/%.c=$(BUILD_PATH)/%.o)

TARGET := cfs

.PHONY: all
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

$(BUILD_PATH)/%.o: $(SOURCE_PATH)/%.c
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f $(TARGET)
	rm -f $(BUILD_PATH)/*.o
