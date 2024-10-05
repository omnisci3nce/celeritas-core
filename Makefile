CC := clang
INCLUDES := -I./include -Ideps/glfw-3.3.8/include/GLFW
CFLAGS := -Wall -Wextra -O2 $(INCLUDES)
LDFLAGS := -lglfw

# Directories
SRC_DIR := src
BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/objs
SHADER_DIR := assets/shaders
SHADER_OUT_DIR := $(BUILD_DIR)/shaders

# Source files
SRCS := $(wildcard $(SRC_DIR)/*.c $(SRC_DIR)/**/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

# Library outputs
STATIC_LIB := $(BUILD_DIR)/libceleritas.a
SHARED_LIB := $(BUILD_DIR)/libceleritas.so

## Makefile notes
# $@ - target of current rule
# $^ - prerequisites of current rule separated by spaces
# $< - first prerequisite file only

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(SHARED_LIB): $(OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CC) -shared -o $@ $^ $(LDFLAGS)

$(STATIC_LIB): $(OBJS)
	@mkdir -p $(BUILD_DIR)
	ar rcs $@ $^

shared: $(SHARED_LIB)

static: $(STATIC_LIB)

.PHONY: all
all: shared static


.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
