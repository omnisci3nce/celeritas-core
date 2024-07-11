/**
 * @brief
 */

#pragma once
#include "defines.h"

typedef struct Renderer Renderer;
typedef struct RendererConfig RendererConfig;

PUB bool Renderer_Init(RendererConfig config, Renderer* renderer);
PUB void Renderer_Shutdown(Renderer* renderer);
