#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "animation.h"
#include "colours.h"
#include "core.h"
#include "defines.h"
#include "file.h"
#include "loaders.h"
#include "log.h"
#include "maths.h"
#include "maths_types.h"
#include "mem.h"
#include "path.h"
#include "ral_types.h"
#include "render.h"
// #include "render_backend.h"
#include "render_types.h"
#include "str.h"

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

extern Core g_core;

struct face {
  cgltf_uint indices[3];
};
typedef struct face face;

KITC_DECL_TYPED_ARRAY(Vec3)
KITC_DECL_TYPED_ARRAY(Vec2)
KITC_DECL_TYPED_ARRAY(Vec4u)
KITC_DECL_TYPED_ARRAY(Vec4)
KITC_DECL_TYPED_ARRAY(face)
// KITC_DECL_TYPED_ARRAY(joint)

bool model_load_gltf_str(const char *file_string, const char *filepath, Str8 relative_path,
                         Model *out_model, bool invert_textures_y);

ModelHandle model_load_gltf(const char *path, bool invert_texture_y) {
  size_t arena_size = 1024;
  arena scratch = arena_create(malloc(arena_size), arena_size);

  TRACE("Loading model at Path %s\n", path);
  path_opt relative_path = path_parent(&scratch, path);
  if (!relative_path.has_value) {
    WARN("Couldnt get a relative path for the path to use for loading materials & textures later");
  }
  const char *file_string = string_from_file(path);

  ModelHandle handle;
  Model *model = Model_pool_alloc(&g_core.models, &handle);
  model->name = Str8_cstr_view(path);
  model->meshes = Mesh_darray_new(1);
  model->materials = Material_darray_new(1);

  // bool success =
  //     model_load_gltf_str(file_string, path, relative_path.path, model, invert_texture_y);

  // if (!success) {
  //   FATAL("Couldnt load GLTF file at path %s", path);
  //   ERROR_EXIT("Load fails are considered crash-worthy right now. This will change later.\n");
  // }

  // arena_free_all(&scratch);
  // arena_free_storage(&scratch);
  return handle;
}

void assert_path_type_matches_component_type(cgltf_animation_path_type target_path,
                                             cgltf_accessor *output) {
  if (target_path == cgltf_animation_path_type_rotation) {
    assert(output->component_type == cgltf_component_type_r_32f);
    assert(output->type == cgltf_type_vec4);
  }
}

// TODO: Brainstorm how I can make this simpler and break it up into more testable pieces

/*
typedef struct model {
  str8 name;
  mesh* meshes;
  u32 mesh_count;
} model;
*/
bool model_load_gltf_str(const char *file_string, const char *filepath, Str8 relative_path,
                         Model *out_model, bool invert_textures_y) {
  // TRACE("Load GLTF from string");

  // // Setup temps
  // vec3_darray *tmp_positions = vec3_darray_new(1000);
  // vec3_darray *tmp_normals = vec3_darray_new(1000);
  // vec2_darray *tmp_uvs = vec2_darray_new(1000);
  // vec4u_darray *tmp_joint_indices = vec4u_darray_new(1000);
  // vec4_darray *tmp_weights = vec4_darray_new(1000);
  // // FIXME
  // // joint_darray *tmp_joints = joint_darray_new(256);
  // // vertex_bone_data_darray *tmp_vertex_bone_data = vertex_bone_data_darray_new(1000);

  // cgltf_options options = { 0 };
  cgltf_data *data = NULL;
  // cgltf_result result = cgltf_parse_file(&options, filepath, &data);
  // if (result != cgltf_result_success) {
  //   WARN("gltf load failed");
  //   // TODO: cleanup arrays(allocate all from arena ?)
  //   return false;
  // }

  // cgltf_load_buffers(&options, data, filepath);
  // DEBUG("loaded buffers");

  //   // --- Skin
  //   size_t num_skins = data->skins_count;
  //   bool is_skinned = false;
  //   if (num_skins == 1) {
  //     is_skinned = true;
  //   } else if (num_skins > 1) {
  //     WARN("GLTF files with more than 1 skin are not supported");
  //     return false;
  //   }

  //   if (is_skinned) {
  //     cgltf_skin *gltf_skin = data->skins;
  //     DEBUG("loading skin %s", gltf_skin->name);
  //     size_t num_joints = gltf_skin->joints_count;
  //     DEBUG("# Joints %d", num_joints);

  //     cgltf_accessor *gltf_inverse_bind_matrices = gltf_skin->inverse_bind_matrices;

  //     // for each one we'll spit out a joint
  //     for (size_t i = 0; i < num_joints; i++) {
  //       cgltf_node *joint_node = gltf_skin->joints[i];

  //       joint joint_i = { .name = "testjoint" };
  //       if (joint_node->children_count > 0 && !joint_node->has_translation &&
  //           !joint_node->has_rotation) {
  //         WARN("joint Node with index %d is the root node", i);
  //         joint_i.transform_components = TRANSFORM_DEFAULT;
  //       } else {
  //         TRACE("Storing joint transform");
  //         joint_i.transform_components = TRANSFORM_DEFAULT;
  //         if (joint_node->has_translation) {
  //           memcpy(&joint_i.transform_components.position, &joint_node->translation, 3 *
  //           sizeof(f32));
  //         }
  //         if (joint_node->has_rotation) {
  //           memcpy(&joint_i.transform_components.rotation, &joint_node->rotation, 4 *
  //           sizeof(f32));
  //         }
  //         // TODO: support scaling as vec instead of float
  //       }
  //       joint_i.local_transform = transform_to_mat(&joint_i.transform_components);
  //       cgltf_accessor_read_float(gltf_inverse_bind_matrices, i,
  //       &joint_i.inverse_bind_matrix.data[0],
  //                                 16);
  //       // joint_darray_push(tmp_joints, joint_i);
  //     }
  //   }

  // --- Materials
  size_t num_materials = data->materials_count;
  TRACE("Num materials %d", num_materials);
  for (size_t m = 0; m < num_materials; m++) {
    cgltf_material gltf_material = data->materials[m];
    cgltf_pbr_metallic_roughness pbr = gltf_material.pbr_metallic_roughness;

    TRACE("Has PBR metallic roughness ");
    // we will use base color texture like blinn phong
    cgltf_texture_view albedo_tex_view = pbr.base_color_texture;  // albedo
    char albedo_map_path[1024];
    snprintf(albedo_map_path, sizeof(albedo_map_path), "%s/%s", relative_path.buf,
             albedo_tex_view.texture->image->uri);

    cgltf_texture_view metal_rough_tex_view = pbr.metallic_roughness_texture;
    char metal_rough_map_path[1024];
    snprintf(metal_rough_map_path, sizeof(metal_rough_map_path), "%s/%s", relative_path.buf,
             metal_rough_tex_view.texture->image->uri);

    cgltf_texture_view normal_tex_view = gltf_material.normal_texture;
    char normal_map_path[1024];
    snprintf(normal_map_path, sizeof(normal_map_path), "%s/%s", relative_path.buf,
             normal_tex_view.texture->image->uri);

    // material our_material =
    //     pbr_material_load(albedo_map_path, normal_map_path, true, metal_rough_map_path, NULL,
    //     NULL);

    // our_material.name = malloc(strlen(gltf_material.name) + 1);
    // strcpy(our_material.name, gltf_material.name);

    // material_darray_push(out_model->materials, our_material);
  }

  // TEMP
  u32 mat_idx = 9999;

  // --- Meshes
  size_t num_meshes = data->meshes_count;
  TRACE("Num meshes %d", num_meshes);
  for (size_t m = 0; m < num_meshes; m++) {
    cgltf_primitive primitive = data->meshes[m].primitives[0];
    DEBUG("Found %d attributes", primitive.attributes_count);
    // DEBUG("Number of this primitive %d", primitive.)

    for (cgltf_size a = 0; a < data->meshes[m].primitives[0].attributes_count; a++) {
      cgltf_attribute attribute = data->meshes[m].primitives[0].attributes[a];
      if (attribute.type == cgltf_attribute_type_position) {
        TRACE("Load positions from accessor");

        cgltf_accessor *accessor = attribute.data;
        assert(accessor->component_type == cgltf_component_type_r_32f);
        // CASSERT_MSG(accessor->type == cgltf_type_vec3, "Vertex positions should be a vec3");

        TRACE("Loading %d vec3 components", accessor->count);

        // for (cgltf_size v = 0; v < accessor->count; ++v) {
        //   vec3 pos;
        //   cgltf_accessor_read_float(accessor, v, &pos.x, 3);
        //   vec3_darray_push(tmp_positions, pos);
        // }

      } else if (attribute.type == cgltf_attribute_type_normal) {
        TRACE("Load normals from accessor");

        cgltf_accessor *accessor = attribute.data;
        assert(accessor->component_type == cgltf_component_type_r_32f);
        // CASSERT_MSG(accessor->type == cgltf_type_vec3, "Normal vectors should be a vec3");

        // for (cgltf_size v = 0; v < accessor->count; ++v) {
        //   vec3 pos;
        //   cgltf_accessor_read_float(accessor, v, &pos.x, 3);
        //   vec3_darray_push(tmp_normals, pos);
        // }

      } else if (attribute.type == cgltf_attribute_type_texcoord) {
        TRACE("Load texture coordinates from accessor");
        cgltf_accessor *accessor = attribute.data;
        assert(accessor->component_type == cgltf_component_type_r_32f);
        // CASSERT_MSG(accessor->type == cgltf_type_vec2, "Texture coordinates should be a
        // vec2");

        for (cgltf_size v = 0; v < accessor->count; ++v) {
          // vec2 tex;
          // bool success = cgltf_accessor_read_float(accessor, v, &tex.x, 2);
          // if (!success) {
          //   ERROR("Error loading tex coord");
          // }
          // vec2_darray_push(tmp_uvs, tex);
        }
      } else if (attribute.type == cgltf_attribute_type_joints) {
        // FIXME: joints
        // TRACE("Load joint indices from accessor");
        // cgltf_accessor *accessor = attribute.data;
        // assert(accessor->component_type == cgltf_component_type_r_16u);
        // assert(accessor->type == cgltf_type_vec4);
        // vec4u joint_indices;
        // vec4 joints_as_floats;
        // for (cgltf_size v = 0; v < accessor->count; ++v) {
        //   cgltf_accessor_read_float(accessor, v, &joints_as_floats.x, 4);
        //   joint_indices.x = (u32)joints_as_floats.x;
        //   joint_indices.y = (u32)joints_as_floats.y;
        //   joint_indices.z = (u32)joints_as_floats.z;
        //   joint_indices.w = (u32)joints_as_floats.w;
        //   printf("Joints affecting vertex %d :  %d %d %d %d\n", v, joint_indices.x,
        //   joint_indices.y,
        //          joint_indices.z, joint_indices.w);
        //   vec4u_darray_push(tmp_joint_indices, joint_indices);
        // }

      } else if (attribute.type == cgltf_attribute_type_weights) {
        // FIXME: weights
        // TRACE("Load joint weights from accessor");
        // cgltf_accessor *accessor = attribute.data;
        // assert(accessor->component_type == cgltf_component_type_r_32f);
        // assert(accessor->type == cgltf_type_vec4);

        // for (cgltf_size v = 0; v < accessor->count; ++v) {
        //   vec4 weights;
        //   cgltf_accessor_read_float(accessor, v, &weights.x, 4);
        //   printf("Weights affecting vertex %d : %f %f %f %f\n", v, weights.x, weights.y,
        //   weights.z,
        //          weights.w);
        //   vec4_darray_push(tmp_weights, weights);
        // }
      } else {
        WARN("Unhandled cgltf_attribute_type: %s. skipping..", attribute.name);
      }
    }
    // mesh.vertex_bone_data = vertex_bone_data_darray_new(1);

    // if (primitive.material != NULL) {
    //   ERROR("Primitive Material %s", primitive.material->name);
    //   for (u32 i = 0; i < material_darray_len(out_model->materials); i++) {
    //     printf("%s vs %s \n", primitive.material->name, out_model->materials->data[i].name);
    //     if (strcmp(primitive.material->name, out_model->materials->data[i].name) == 0) {
    //       INFO("Found material");
    //       mat_idx = i;
    //       // mesh.material_index = i;
    //       break;
    //     }
    //   }
    // }

    //     // FIXME
    //     // if (is_skinned) {
    //     //   mesh.is_skinned = true;
    //     //   // mesh.vertex_bone_data = vertex_bone_data_darray_new(tmp_joint_indices->len);
    //     //   mesh.bones = joint_darray_new(tmp_joints->len);
    //     //   for (int i = 0; i < tmp_joint_indices->len; i++) {
    //     //     vertex_bone_data data;
    //     //     data.joints = tmp_joint_indices->data[i];
    //     //     data.weights = tmp_weights->data[i];
    //     //     vertex_bone_data_darray_push(tmp_vertex_bone_data,
    //     //                                  data);  // Push the temp data that aligns with raw
    //     vertices
    //     //   }
    //     //   for (int i = 0; i < tmp_joints->len; i++) {
    //     //     joint data = tmp_joints->data[i];
    //     //     joint_darray_push(mesh.bones, data);
    //     //   }
    //     // }

    bool has_indices = false;
    Vertex_darray *geo_vertices = Vertex_darray_new(3);
    u32_darray *geo_indices = u32_darray_new(0);

    // Store vertices
    // printf("Positions %d Normals %d UVs %d\n", tmp_positions->len, tmp_normals->len,
    // tmp_uvs->len); assert(tmp_positions->len == tmp_normals->len); assert(tmp_normals->len ==
    // tmp_uvs->len); for (u32 v_i = 0; v_i < tmp_positions->len; v_i++) {
    //   vertex v = { .static_3d = {
    //                    .position = tmp_positions->data[v_i],
    //                    .normal = tmp_normals->data[v_i],
    //                    .tex_coords = tmp_uvs->data[v_i],
    //                } };
    //   vertex_darray_push(geo_vertices, v);
  }

  // Store indices
  // cgltf_accessor *indices = primitive.indices;
  // if (primitive.indices > 0) {
  // WARN("indices! %d", indices->count);
  // has_indices = true;

  // // store indices
  // for (cgltf_size i = 0; i < indices->count; ++i) {
  //   cgltf_uint ei;
  //   cgltf_accessor_read_uint(indices, i, &ei, 1);
  //   u32_darray_push(geo_indices, ei);
  // }

  // fetch and store vertices for each index
  // for (cgltf_size i = 0; i < indices->count; ++i) {
  //   vertex vert;
  //   cgltf_uint index = mesh.indices[i];
  //   vert.position = tmp_positions->data[index];
  //   vert.normal = tmp_normals->data[index];
  //   vert.uv = tmp_uvs->data[index];
  //   vertex_darray_push(mesh.vertices, vert);

  //   if (is_skinned) {
  //     vertex_bone_data vbd = tmp_vertex_bone_data->data[index];  // create a copy
  //     vertex_bone_data_darray_push(mesh.vertex_bone_data, vbd);
  //   }
  //   // for each vertex do the bone data
  // }
  // } else {
  // has_indices = false;
  // return false;  // TODO: handle this
  // }

  // geometry_data *geometry = malloc(sizeof(geometry_data));
  // geometry->format = VERTEX_STATIC_3D;
  // geometry->colour = (rgba){ 1, 1, 1, 1 };
  // geometry->vertices = geo_vertices;
  // geometry->indices = geo_indices;
  // geometry->has_indices = has_indices;

  // mesh m = mesh_create(geometry, true);
  // m.material_index = (u32_opt){ .has_value = mat_idx == 9999, .value = mat_idx };

  // mesh_darray_push(out_model->meshes, m);
  //    }

  //     // clear data for each mesh
  //     vec3_darray_clear(tmp_positions);
  //     vec3_darray_clear(tmp_normals);
  //     vec2_darray_free(tmp_uvs);
  //     vec4u_darray_clear(tmp_joint_indices);
  //     vec4_darray_clear(tmp_weights);
  //     joint_darray_clear(tmp_joints);
  //   }

  //   for (int i = 0; i < out_model->meshes->len; i++) {
  //     u32 mat_idx = out_model->meshes->data[i].material_index;
  //     printf("Mesh %d Mat index %d Mat name %s\n", i, mat_idx,
  //            out_model->materials->data[mat_idx].name);
  //   }

  //   // Animations
  //   TRACE("Num animations %d", data->animations_count);
  //   size_t num_animations = data->animations_count;
  //   if (num_animations > 0) {
  // // Create an arena for all animation related data
  // #define ANIMATION_STORAGE_ARENA_SIZE (1024 * 1024 * 1024)
  //     char *animation_backing_storage = malloc(ANIMATION_STORAGE_ARENA_SIZE);
  //     // We'll store data on this arena so we can easily free it all at once later
  //     out_model->animation_data_arena =
  //         arena_create(animation_backing_storage, ANIMATION_STORAGE_ARENA_SIZE);
  //     arena *arena = &out_model->animation_data_arena;

  //     if (!out_model->animations) {
  //       out_model->animations = animation_clip_darray_new(num_animations);
  //     }

  //     for (int anim_idx = 0; anim_idx < data->animations_count; anim_idx++) {
  //       cgltf_animation animation = data->animations[anim_idx];
  //       animation_clip clip = { 0 };

  //       for (size_t c = 0; c < animation.channels_count; c++) {
  //         cgltf_animation_channel channel = animation.channels[c];

  //         animation_sampler *sampler = arena_alloc(arena, sizeof(animation_sampler));

  //         animation_sampler **target_property;
  //         keyframe_kind data_type;

  //         switch (channel.target_path) {
  //           case cgltf_animation_path_type_rotation:
  //             target_property = &clip.rotation;
  //             data_type = KEYFRAME_ROTATION;
  //             break;
  //           case cgltf_animation_path_type_translation:
  //             target_property = &clip.translation;
  //             data_type = KEYFRAME_TRANSLATION;
  //             break;
  //           case cgltf_animation_path_type_scale:
  //             target_property = &clip.scale;
  //             data_type = KEYFRAME_SCALE;
  //             break;
  //           case cgltf_animation_path_type_weights:
  //             target_property = &clip.weights;
  //             data_type = KEYFRAME_WEIGHTS;
  //             WARN("Morph target weights arent supported yet");
  //             return false;
  //           default:
  //             WARN("unsupported animation type");
  //             return false;
  //         }
  //         *target_property = sampler;

  //         sampler->current_index = 0;
  //         printf("1 %d index\n", sampler->current_index);
  //         sampler->animation.interpolation = INTERPOLATION_LINEAR;

  //         // keyframe times
  //         size_t n_frames = channel.sampler->input->count;
  //         assert(channel.sampler->input->component_type == cgltf_component_type_r_32f);
  //         // FIXME: CASSERT_MSG function "Expected animation sampler input component to be type
  //         f32
  //         // (keyframe times)");
  //         f32 *times = arena_alloc(arena, n_frames * sizeof(f32));
  //         sampler->animation.n_timestamps = n_frames;
  //         sampler->animation.timestamps = times;
  //         cgltf_accessor_unpack_floats(channel.sampler->input, times, n_frames);

  //         assert_path_type_matches_component_type(channel.target_path, channel.sampler->output);

  //         // keyframe values
  //         size_t n_values = channel.sampler->output->count;
  //         assert(n_frames == n_values);

  //         keyframes keyframes = { 0 };
  //         keyframes.kind = KEYFRAME_ROTATION;
  //         keyframes.count = n_values;
  //         keyframes.values = arena_alloc(arena, n_values * sizeof(keyframe));
  //         for (cgltf_size v = 0; v < channel.sampler->output->count; ++v) {
  //           switch (data_type) {
  //             case KEYFRAME_ROTATION: {
  //               quat rot;
  //               cgltf_accessor_read_float(channel.sampler->output, v, &rot.x, 4);
  //               // printf("Quat %f %f %f %f\n", rot.x, rot.y, rot.z, rot.w);
  //               keyframes.values[v].rotation = rot;
  //               break;
  //             }
  //             case KEYFRAME_TRANSLATION: {
  //               vec3 trans;
  //               cgltf_accessor_read_float(channel.sampler->output, v, &trans.x, 3);
  //               keyframes.values[v].translation = trans;
  //               break;
  //             }
  //             case KEYFRAME_SCALE: {
  //               vec3 scale;
  //               cgltf_accessor_read_float(channel.sampler->output, v, &scale.x, 3);
  //               keyframes.values[v].scale = scale;
  //               break;
  //             }
  //             case KEYFRAME_WEIGHTS: {
  //               // TODO
  //               break;
  //             }
  //           }
  //         }
  //         sampler->animation.values = keyframes;

  //         sampler->min = channel.sampler->input->min[0];
  //         sampler->max = channel.sampler->input->max[0];

  //         // clip.rotation = sampler;
  //         // printf("%d timestamps\n", sampler->animation.n_timestamps);
  //         // printf("%d index\n", sampler->current_index);
  //       }

  //       WARN("stuff %ld", clip.rotation->animation.n_timestamps);
  //       animation_clip_darray_push(out_model->animations, clip);
  //     }
  //   }

  return true;
}

/*
bool model_load_gltf(const char *path, model *out_model) {
  TRACE("Load GLTF %s", path);

  // Setup temp arrays
  kitc_darray *tmp_positions = kitc_darray_new(sizeof(vec3), 1000);
  kitc_darray *tmp_normals = kitc_darray_new(sizeof(vec3), 1000);
  kitc_darray *tmp_uvs = kitc_darray_new(sizeof(vec2), 1000);

  // may as well just init with max capacity as we're just gonna free at end of this function anyway
  bh_material_darray *materials = bh_material_darray_new(MAX_MATERIALS);
  CASSERT(materials->len == 0);

  cgltf_options options = {0};
  cgltf_data *data = NULL;
  cgltf_result result = cgltf_parse_file(&options, path, &data);
  if (result == cgltf_result_success) {
    DEBUG("gltf loaded succesfully");

    cgltf_load_buffers(&options, data, path);
    DEBUG("loaded buffers");

    // -- Load materials.
    // Each mesh will be handed a material
    TRACE("Num materials %d", data->materials_count);
    out_model->num_materials = data->materials_count;

    for (int m = 0; m < data->materials_count; m++) {
      cgltf_material gltf_material = data->materials[m];
      bh_material our_material = {0};

      str8 name = str8_copy(gltf_material.name);
      printf("Material name %s\n", name.buf);
      our_material.name = name;

      cgltf_pbr_metallic_roughness pbr = gltf_material.pbr_metallic_roughness;
      if (gltf_material.has_pbr_metallic_roughness) {
        // we will use base color texture like blinn phong
        cgltf_texture_view diff_tex = pbr.base_color_texture;
        strcpy(our_material.diffuse_tex_path, diff_tex.texture->image->uri);
      }

      bh_material_darray_push(materials, our_material);
    }

    // -- Load animations.
    TRACE("Num animations %d", data->animations_count);
    out_model->num_animations = data->animations_count;
    for (int anim_idx = 0; anim_idx < data->animations_count; anim_idx++) {
      cgltf_animation animation = data->animations[anim_idx];
      animation_clip our_animation = {0};

      // loop through each channel (track)
      for (int c = 0; c < animation.channels_count; c++) {
        // each channel (track) has a target and a sampler
        // for the time being we assume the target is the model itself
        cgltf_animation_channel channel = animation.channels[c];
        animation_track our_track = {0};
        our_track.interpolation = interpolation_fn_from_gltf(channel.sampler->interpolation);
        our_track.property = anim_prop_from_gltf(channel.target_path);

        // get the actual data out via the "accessor"
        // input will be the times

        // Keyframe times
        size_t n_frames = channel.sampler->input->count;
        our_track.num_keyframes = n_frames;
        f32 *times = malloc(sizeof(f32) * n_frames);
        our_track.keyframe_times = times;
        CASSERT_MSG(channel.sampler->input->component_type == cgltf_component_type_r_32f,
                    "Expected animation sampler input component to be type f32 (keyframe times)");
        cgltf_accessor_unpack_floats(channel.sampler->input, times, channel.sampler->input->count);

        // printf("keyframe times[\n");
        // for (int i = 0; i < n_frames; i++) {
        //   printf("  %f\n", times[i]);
        // }
        // printf("]\n");

        // Data!
        if (channel.target_path == cgltf_animation_path_type_rotation) {
          CASSERT(channel.sampler->output->component_type == cgltf_component_type_r_32f);
          CASSERT(channel.sampler->output->type == cgltf_type_vec4);
        }

        our_track.keyframes = malloc(sizeof(keyframe_data) * n_frames);
        for (cgltf_size v = 0; v < channel.sampler->output->count; ++v) {
          quat rot;
          cgltf_accessor_read_float(channel.sampler->output, v, &rot.x, 4);
          // vectors[v] = rot;
          // printf("Quat %f %f %f %f\n", rot.x, rot.y, rot.z, rot.w);
          our_track.keyframes[v].rotation = rot;
        }

        our_track.min_time = channel.sampler->input->min[0];
        our_track.max_time = channel.sampler->input->max[0];

        // printf("min time: %f max time %f\n", our_track.min_time, our_track.max_time);

        animation_track_darray_push(&our_animation.tracks, our_track);
      }

      out_model->animations[anim_idx] = our_animation;
    }

    // Load meshes
    TRACE("Num meshes %d", data->meshes_count);
    out_model->num_meshes = data->meshes_count;

    for (int m = 0; m < data->meshes_count; m++) {
      // at the moment we only handle one primitives per mesh
      // CASSERT(data->meshes[m].primitives_count == 1);

      // Load vertex data from FIRST primitive only
      cgltf_primitive primitive = data->meshes[m].primitives[0];
      DEBUG("Found %d attributes", primitive.attributes_count);
      for (int a = 0; a < data->meshes[m].primitives[0].attributes_count; a++) {
        cgltf_attribute attribute = data->meshes[m].primitives[0].attributes[a];
        if (attribute.type == cgltf_attribute_type_position) {
          TRACE("Load positions from accessor");

          cgltf_accessor *accessor = attribute.data;
          CASSERT(accessor->component_type == cgltf_component_type_r_32f);
          CASSERT_MSG(accessor->type == cgltf_type_vec3, "Vertex positions should be a vec3");

          for (cgltf_size v = 0; v < accessor->count; ++v) {
            vec3 pos;
            cgltf_accessor_read_float(accessor, v, &pos.x, 3);
            kitc_darray_push(tmp_positions, &pos);
          }

        } else if (attribute.type == cgltf_attribute_type_normal) {
          TRACE("Load normals from accessor");

          cgltf_accessor *accessor = attribute.data;
          CASSERT(accessor->component_type == cgltf_component_type_r_32f);
          CASSERT_MSG(accessor->type == cgltf_type_vec3, "Normal vectors should be a vec3");

          for (cgltf_size v = 0; v < accessor->count; ++v) {
            vec3 pos;
            cgltf_accessor_read_float(accessor, v, &pos.x, 3);
            kitc_darray_push(tmp_normals, &pos);
          }

        } else if (attribute.type == cgltf_attribute_type_texcoord) {
          TRACE("Load texture coordinates from accessor");
          cgltf_accessor *accessor = attribute.data;
          CASSERT(accessor->component_type == cgltf_component_type_r_32f);
          CASSERT_MSG(accessor->type == cgltf_type_vec2, "Texture coordinates should be a vec2");

          for (cgltf_size v = 0; v < accessor->count; ++v) {
            vec2 tex;
            bool success = cgltf_accessor_read_float(accessor, v, &tex.x, 2);
            if (!success) {
              ERROR("Error loading tex coord");
            }
            kitc_darray_push(tmp_uvs, &tex);
          }
        } else if (attribute.type == cgltf_attribute_type_joints) {
          // handle joints

        } else {
          WARN("Unhandled cgltf_attribute_type: %s. skipping..", attribute.name);
        }
      }

      // Create mesh
      mesh mesh;
      mesh.vertices =
          kitc_darray_new(sizeof(mesh_vertex), data->meshes[m].primitives[0].attributes_count);

      // Flatten faces from indices if present otherwise push vertices verbatim
      cgltf_accessor *indices = primitive.indices;
      if (primitive.indices > 0) {
        mesh.has_indices = true;

        kitc_darray *element_indexes = kitc_darray_new(sizeof(cgltf_uint), indices->count);
        TRACE("Indices count %ld\n", indices->count);
        for (cgltf_size i = 0; i < indices->count; ++i) {
          cgltf_uint ei;
          cgltf_accessor_read_uint(indices, i, &ei, 1);
          kitc_darray_push(element_indexes, &ei);
        }

        kitc_darray_iter indices_iter = kitc_darray_iter_new(element_indexes);
        cgltf_uint *cur;
        while ((cur = kitc_darray_iter_next(&indices_iter))) {
          mesh_vertex vert;
          memcpy(&vert.position, &((vec3 *)tmp_positions->data)[*cur], sizeof(vec3));
          memcpy(&vert.normal, &((vec3 *)tmp_normals->data)[*cur], sizeof(vec3));
          memcpy(&vert.tex_coord, &((vec2 *)tmp_uvs->data)[*cur], sizeof(vec2));
          kitc_darray_push(mesh.vertices, &vert);
          // mesh_vertex_debug_print(vert);
        }
        // printf("indices: %ld, positions: %ld\n", kitc_darray_len(element_indexes),
        kitc_darray_free(element_indexes);
      } else {
        mesh.has_indices = false;

        bool calc_normals = false;
        if (kitc_darray_len(tmp_normals) == 0) {
          TRACE("No normals data is present. Normals will be calculated for you.");
          calc_normals = true;
        }
        for (int v = 0; v < kitc_darray_len(tmp_positions); v++) {
          mesh_vertex vert;
          memcpy(&vert.position, &((vec3 *)tmp_positions->data)[v], sizeof(vec3));
          if (!calc_normals) {
            memcpy(&vert.normal, &((vec3 *)tmp_normals->data)[v], sizeof(vec3));
          }
          memcpy(&vert.tex_coord, &((vec2 *)tmp_uvs->data)[v], sizeof(vec2));
          kitc_darray_push(mesh.vertices, &vert);
        }

        if (calc_normals) {
          if (mesh.has_indices) {
            // generate_normals_nonindexed(mesh.vertices);
          } else {
            generate_normals_nonindexed(mesh.vertices);
          }
        }
      }

      // Material
      if (primitive.material != NULL) {
        for (int i = 0; i < bh_material_darray_len(materials); i++) {
          if (strcmp(primitive.material->name, cstr(materials->data->name))) {
            TRACE("Found material");
            mesh.material_index = i;
            break;
          }
        }
      }

      // mesh.material_index = 0;  // TODO: make sure DEFAULT_MATERIAL is added at material index 0
      // TODO: material handling
      mesh.material_index = bh_material_darray_len(materials) - 1;

      calc_mesh_bounding_box(&mesh);
      // out_model->meshes.data[m] = mesh;
      mesh_darray_push(&out_model->meshes, mesh);

      kitc_darray_clear(tmp_positions);
      kitc_darray_clear(tmp_normals);
      kitc_darray_clear(tmp_uvs);
    }
    // End Load meshes

    // Load animations
    DEBUG("Num animations %d", data->animations_count);
    out_model->num_animations = data->animations_count;

    // End Load animations

    cgltf_free(data);
  } else {
    ERROR("Load failed");
    kitc_darray_free(tmp_positions);
    kitc_darray_free(tmp_normals);
    kitc_darray_free(tmp_uvs);
    return false;
  }

  for (int i = 0; i < materials->len; i++) {
    out_model->materials[i] = materials->data[i];
  }

  calc_model_bounding_box(out_model);

  DEBUG("Num meshes %d", out_model->num_meshes);
  DEBUG("Num materials %d", out_model->num_materials);
  DEBUG("Num animations %d", out_model->num_animations);

  CASSERT(out_model->num_materials == 1);

  kitc_darray_free(tmp_positions);
  kitc_darray_free(tmp_normals);
  kitc_darray_free(tmp_uvs);
  bh_material_darray_free(materials);

  TRACE("Finished loading GLTF");
  return true;
}
*/
