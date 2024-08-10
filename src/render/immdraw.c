#include "immdraw.h"
#include "log.h"
#include "primitives.h"
#include "ral_common.h"
#include "ral_impl.h"
#include "ral_types.h"
#include "render.h"
#include "shader_layouts.h"

void Immdraw_Init(Immdraw_Storage* storage) {
  INFO("Immediate drawing initialisation");
  // meshes
  // Geometry sphere_geo = Geo_CreateUVsphere(1.0, 8, 8);
  // storage->sphere = Mesh_Create(&sphere_geo, false);

  // pipeline / material
  ShaderDataLayout camera_data = Binding_Camera_GetLayout(NULL);
  GraphicsPipelineDesc pipeline_desc = {
    .debug_name = "Immediate Draw Pipeline",
    .data_layouts = { camera_data },
    .data_layouts_count = 1,

  };
  // storage->colour_pipeline = GPU_GraphicsPipeline_Create(pipeline_desc,
  // GPU_GetDefaultRenderpass());
}

void Immdraw_Sphere(Transform tf, f32 size, Vec4 colour, bool wireframe) {}