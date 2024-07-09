#pragma once
#include "defines.h"
#include "ral_types.h"

struct GLFWwindow;

bool gpu_backend_init(const char* window_name, struct GLFWwindow* window);
void gpu_backend_shutdown();

bool gpu_device_create(gpu_device* out_device);
void gpu_device_destroy(gpu_device* device);

gpu_pipeline* gpu_graphics_pipeline_create(struct graphics_pipeline_desc description);
void gpu_pipeline_destroy(gpu_pipeline* pipeline);

// --- Renderpass
gpu_renderpass* gpu_renderpass_create(const gpu_renderpass_desc* description);
void gpu_renderpass_destroy(gpu_renderpass* pass);

// --- Swapchain
bool gpu_swapchain_create(gpu_swapchain* out_swapchain);
void gpu_swapchain_destroy(gpu_swapchain* swapchain);