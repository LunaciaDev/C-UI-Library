CC = clang
BUILD_FLAGS = -Wall -Werror -pedantic -std=c99 -MMD -xc
LINK_LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

SRC_DIR = src
SRCS = $(wildcard $(SRC_DIR)/*.c) # flat structure!

all: debug

DEBUG_FLAGS = -g -DDEBUG
DEBUG_DIR = build/debug
DEBUG_OBJS = $(patsubst $(SRC_DIR)/%.c,$(DEBUG_DIR)/%.o,$(SRCS))
DEBUG_BIN = $(DEBUG_DIR)/main
DEBUG_DEPS = $(patsubst $(SRC_DIR)/%.c,$(DEBUG_DIR)/%.d,$(SRCS))

debug: $(DEBUG_OBJS)
	$(CC) $(LINK_LIBS) $^ -o $(DEBUG_BIN)

$(DEBUG_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(BUILD_FLAGS) $(DEBUG_FLAGS) -c $< -o $@

-include $(DEBUG_DEPS)

RELEASE_FLAGS = -O2
RELEASE_DIR = build/release
RELEASE_OBJS = $(patsubst $(SRC_DIR)/%.c,$(RELEASE_DIR)/%.o,$(SRCS))
RELEASE_BIN = $(RELEASE_DIR)/main
RELEASE_DEPS = $(patsubst $(SRC_DIR)/%.c,$(RELEASE_DIR)/%.d,$(SRCS))

release: $(RELEASE_OBJS)
	$(CC) $(LINK_LIBS) $^ -o $(RELEASE_BIN)

$(RELEASE_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(BUILD_FLAGS) $(RELEASE_FLAGS) -c $< -o $@

-include $(RELEASE_DEPS)

clean:
	rm -rf build/debug/*
	rm -rf build/release/*