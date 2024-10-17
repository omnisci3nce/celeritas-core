CC := clang
INCLUDES := -I./include -Ideps/glfw-3.3.8/include/GLFW
CFLAGS := -Wall -Wextra -O2 $(INCLUDES)
LDFLAGS := -lglfw

# Detect OS
UNAME_S := $(shell uname -s)

# Directories
SRC_DIR := src
BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/objs
SHADER_DIR := assets/shaders
SHADER_OUT_DIR := $(BUILD_DIR)/shaders
EXAMPLES_DIR := examples

# Source files
SRCS := $(wildcard $(SRC_DIR)/*.c $(SRC_DIR)/**/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

# Library outputs
STATIC_LIB := $(BUILD_DIR)/libceleritas.a
ifeq ($(UNAME_S),Darwin)
    SHARED_LIB := $(BUILD_DIR)/libceleritas.dylib
    SHARED_FLAGS := -dynamiclib
    LDFLAGS += -framework Foundation -framework CoreFoundation -framework CoreGraphics -framework AppKit -framework QuartzCore -framework Metal -framework MetalKit
		SRCS += $(SRC_DIR)/backend_mtl.m
		OBJS += $(OBJ_DIR)/backend_mtl.o
else
    SHARED_LIB := $(BUILD_DIR)/libceleritas.so
    SHARED_FLAGS := -shared
endif

## Makefile notes
# $@ - target of current rule
# $^ - prerequisites of current rule separated by spaces
# $< - first prerequisite file only

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Objective C
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.m
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(SHARED_LIB): $(OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(SHARED_FLAGS) -o $@ $^ $(LDFLAGS)

$(STATIC_LIB): $(OBJS)
	@mkdir -p $(BUILD_DIR)
	ar rcs $@ $^

shared: $(SHARED_LIB)

static: $(STATIC_LIB)

.PHONY: all
all: shared static

.PHONY: triangle
triangle: $(EXAMPLES_DIR)/triangle.c $(SHARED_LIB)
		@mkdir -p $(BUILD_DIR)
		$(CC) $(CFLAGS) $(EXAMPLES_DIR)/triangle.c -L$(BUILD_DIR) -lceleritas $(LDFLAGS)
		MTL_DEBUG_LAYER=1 build/triangle.bin

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
