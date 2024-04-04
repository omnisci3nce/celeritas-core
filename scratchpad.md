
```c
// engine.h
#pragma once

#include <celstd.h>
#include <renderer.h>
#include <renderer_types.h>
#include <maths_types.h>
#include "threadpool.h"

typedef struct frame_stats {} frame_stats;

typedef struct engine_stats {
  frame_stats frame;
} engine_stats;

typedef struct engine {
  // timing
  float startTime;
  float deltaTime;  // time between current frame and last frame
  float lastFrame;
  // stats
  engine_stats stats;
} engine;

static bool engine_init(engine *engine) {
  engine->startTime = glfwGetTime();
  engine->deltaTime = 0.0f;
  engine->lastFrame = 0.0f;

  return true;
}

static inline void engine_tick_start(engine *engine) {
  float currentFrame = glfwGetTime();
  engine->deltaTime = currentFrame - engine->lastFrame;
  engine->lastFrame = currentFrame;
}

static inline void engine_tick_end(engine *engine) {
  // TODO: clear frame stats
}

void celeritas_print_type_sizes();
```

```c
// engine.c
#include "engine.h"

#include <stdio.h>

#include "animation.h"
#include "application.h"

void celeritas_print_type_sizes() {
  printf("\e[1mType sizes: \e[m \n");
  printf("transform:    %ld bytes\n", sizeof(transform));
  printf("mesh:         %ld bytes\n", sizeof(mesh));
  printf("bh_material:  %ld bytes\n", sizeof(bh_material));
  printf("animation:    %ld bytes\n", sizeof(animation_clip));
  printf("model:        %ld bytes\n", sizeof(model));
  printf("rend_buffer:  %ld bytes\n", sizeof(rend_buffer));

  printf("application:  %ld bytes\n", sizeof(cel_application));
  printf("renderer:     %ld bytes\n", sizeof(renderer));
  printf("threadpool:   %ld bytes\n", sizeof(threadpool));
  printf("engine:       %ld bytes\n", sizeof(engine));
}
```