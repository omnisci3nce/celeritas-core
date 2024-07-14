#pragma once

#include "input.h"
#include "render_types.h"
#include "mem.h"
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

/** @brief Throws error if the core cannot be instantiated */
void Core_Bringup();
void Core_Shutdown();
bool ShouldExit();

struct Renderer* Core_GetRenderer(Core* core);

void Frame_Begin();
void Frame_Draw();
void Frame_End();
