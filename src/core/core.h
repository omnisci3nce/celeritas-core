#pragma once

#include "input.h"
#include "mem.h"
#include "render_types.h"
#include "scene.h"
#include "screenspace.h"
#include "terrain.h"
#include "text.h"

TYPED_POOL(Model, Model)
#define MODEL_GET(h) (Model_pool_get(&g_core.models, h))

typedef struct GLFWwindow GLFWwindow;

typedef struct Core {
  const char* app_name;
  GLFWwindow* window;
  Renderer* renderer;
  Input_State input;
  Model_pool models;
} Core;
extern Core g_core;

struct Renderer;

Core* get_global_core();

/**
  @brief Throws error if the core cannot be instantiated
  @param [in] optional_window - Leave NULL if you want Celeritas to instantiate its own window with
  GLFW, if you want to provide the glfw window then pass it in here.
*/
void Core_Bringup(GLFWwindow* optional_window);
void Core_Shutdown();
bool ShouldExit();

GLFWwindow* Core_GetGlfwWindowPtr(Core* core);
struct Renderer* Core_GetRenderer(Core* core);

void Frame_Begin();
void Frame_Draw();
void Frame_End();
