/**
 * @file render.h
 * @author your name (you@domain.com)
 * @brief Renderer frontend
 * @version 0.1
 * @date 2024-03-21
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once

#include "camera.h"
#include "loaders.h"
#include "render_types.h"

// --- Lifecycle
/** @brief initialise the render system frontend */
bool renderer_init(renderer* ren);
/** @brief shutdown the render system frontend */
void renderer_shutdown(renderer* ren);

void renderer_on_resize(renderer* ren);

struct render_packet;

// --- Frame

void render_frame_begin(renderer* ren);
void render_frame_end(renderer* ren);
void render_frame_draw(renderer* ren);

// --- models meshes
void model_upload_meshes(renderer* ren, model* model);
void draw_model(renderer* ren, camera* camera, model* model, transform tf, scene* scene);
void draw_mesh(renderer* ren, mesh* mesh, transform tf, material* mat, mat4* view, mat4* proj);

void draw_skinned_model(renderer* ren, camera* cam, model* model, transform tf, scene* scene);

void model_destroy(model* model);

// ---
texture texture_data_load(const char* path, bool invert_y);  // #frontend
