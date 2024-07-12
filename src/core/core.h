#pragma once

#include "input.h"
#include "render_types.h"
#include "scene.h"
#include "screenspace.h"
#include "terrain.h"
#include "text.h"

typedef struct Core Core;

Core* get_global_core();

/** @brief Throws error if the core cannot be instantiated */
void Core_Bringup();
void Core_Shutdown();
bool ShouldExit();

void Frame_Begin();
void Frame_Draw();
void Frame_End();
