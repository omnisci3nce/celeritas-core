CC := clang
INCLUDES := -I./include -Ideps/glfw-3.3.8/include/GLFW -Ideps/stb_image
CFLAGS := -Wall -Wextra -O2 -fPIC $(INCLUDES)
# TODO(low prio): split static object files and shared object files so we can remove -fPIC from static lib builds
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

# Format-able files
FORMAT_FILES :=  include/celeritas.h $(SRC_DIR)/*.c $(EXAMPLES_DIR)/*.c

# Shader files
METAL_SHADERS := $(wildcard $(SHADER_DIR)/*.metal)
METAL_AIR_FILES := $(patsubst $(SHADER_DIR)/%.metal,$(SHADER_OUT_DIR)/%.air,$(METAL_SHADERS))
METAL_LIB := $(SHADER_OUT_DIR)/default.metallib

# Library outputs
STATIC_LIB := $(BUILD_DIR)/libceleritas.a
SHARED_FLAGS := -fPIC
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

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c include/celeritas.h
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

# Shaders
$(SHADER_OUT_DIR)/%.air: $(SHADER_DIR)/%.metal
	@mkdir -p $(SHADER_OUT_DIR)
	xcrun -sdk macosx metal -c $< -o $@
	
$(METAL_LIB): $(METAL_AIR_FILES)
	xcrun -sdk macosx metallib $^ -o $(SHADER_OUT_DIR)/default.metallib

.PHONY: all
all: shared static

.PHONY: triangle
triangle: $(EXAMPLES_DIR)/triangle.c $(SHARED_LIB) $(SHADER_OUT_DIR)/triangle.air $(METAL_LIB)
		@mkdir -p $(BUILD_DIR)
		$(CC) $(CFLAGS) $(EXAMPLES_DIR)/triangle.c -L$(BUILD_DIR) -lceleritas $(LDFLAGS) -o $(BUILD_DIR)/triangle.bin
		MTL_DEBUG_LAYER=1 build/triangle.bin

.PHONY: cube
cube: $(EXAMPLES_DIR)/cube.c $(SHARED_LIB) $(SHADER_OUT_DIR)/cube.air $(METAL_LIB)
		@mkdir -p $(BUILD_DIR)
		$(CC) $(CFLAGS) $(EXAMPLES_DIR)/cube.c -L$(BUILD_DIR) -lceleritas $(LDFLAGS) -o $(BUILD_DIR)/cube.bin
		MTL_DEBUG_LAYER=1 build/cube.bin

.PHONY: format
format:
	clang-format -i $(FORMAT_FILES)

.PHONY: format-check
format-check:
	clang-format --dry-run -Werror $(FORMAT_FILES)

.PHONY: tidy-fix
tidy:
	clang-tidy -fix $(SRCS) $(EXAMPLES_DIR)/*.c -- $(CFLAGS)

.PHONY: tidy-check
tidy-check:
	clang-tidy $(SRCS) $(EXAMPLES_DIR)/*.c -- $(CFLAGS)

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
