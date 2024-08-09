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
#include "pbr.h"
#include "ral_types.h"
#include "render.h"
#include "render_types.h"
#include "str.h"

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

extern Core g_core;

/* GLTF Loading Pipeline
   ===================== */

struct face {
  cgltf_uint indices[3];
};
typedef struct face face;

KITC_DECL_TYPED_ARRAY(Vec3)
KITC_DECL_TYPED_ARRAY(Vec2)
KITC_DECL_TYPED_ARRAY(Vec4u)
KITC_DECL_TYPED_ARRAY(Vec4)
KITC_DECL_TYPED_ARRAY(face)

size_t GLTF_LoadMaterials(cgltf_data *data, Str8 relative_path, Material_darray *out_materials);

ModelHandle ModelLoad_gltf(const char *path, bool invert_texture_y) {
  size_t arena_size = MB(1);
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

  bool success =
      model_load_gltf_str(file_string, path, relative_path.path, model, invert_texture_y);

  if (!success) {
    FATAL("Couldnt load GLTF file at path %s", path);
    ERROR_EXIT("Load fails are considered crash-worthy right now. This will change later.\n");
  }

  arena_free_all(&scratch);
  arena_free_storage(&scratch);
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

void load_position_components(Vec3_darray *positions, cgltf_accessor *accessor) {
  TRACE("Loading %d vec3 position components", accessor->count);
  CASSERT_MSG(accessor->component_type == cgltf_component_type_r_32f,
              "Positions components are floats");
  CASSERT_MSG(accessor->type == cgltf_type_vec3, "Vertex positions should be a vec3");

  for (cgltf_size v = 0; v < accessor->count; ++v) {
    Vec3 pos;
    cgltf_accessor_read_float(accessor, v, &pos.x, 3);
    Vec3_darray_push(positions, pos);
  }
}

void load_normal_components(Vec3_darray *normals, cgltf_accessor *accessor) {
  TRACE("Loading %d vec3 normal components", accessor->count);
  CASSERT_MSG(accessor->component_type == cgltf_component_type_r_32f,
              "Normal vector components are floats");
  CASSERT_MSG(accessor->type == cgltf_type_vec3, "Vertex normals should be a vec3");

  for (cgltf_size v = 0; v < accessor->count; ++v) {
    Vec3 pos;
    cgltf_accessor_read_float(accessor, v, &pos.x, 3);
    Vec3_darray_push(normals, pos);
  }
}

void load_texcoord_components(Vec2_darray *texcoords, cgltf_accessor *accessor) {
  TRACE("Load texture coordinates from accessor");
  CASSERT(accessor->component_type == cgltf_component_type_r_32f);
  CASSERT_MSG(accessor->type == cgltf_type_vec2, "Texture coordinates should be a vec2");

  for (cgltf_size v = 0; v < accessor->count; ++v) {
    Vec2 tex;
    bool success = cgltf_accessor_read_float(accessor, v, &tex.x, 2);
    if (!success) {
      ERROR("Error loading tex coord");
    }
    Vec2_darray_push(texcoords, tex);
  }
}

void load_joint_index_components(Vec4u_darray *joint_indices, cgltf_accessor *accessor) {
  TRACE("Load joint indices from accessor");
  CASSERT(accessor->component_type == cgltf_component_type_r_16u);
  CASSERT_MSG(accessor->type == cgltf_type_vec4, "Joint indices should be a vec4");
  Vec4u tmp_joint_index;
  Vec4 joints_as_floats;
  for (cgltf_size v = 0; v < accessor->count; ++v) {
    cgltf_accessor_read_float(accessor, v, &joints_as_floats.x, 4);
    tmp_joint_index.x = (u32)joints_as_floats.x;
    tmp_joint_index.y = (u32)joints_as_floats.y;
    tmp_joint_index.z = (u32)joints_as_floats.z;
    tmp_joint_index.w = (u32)joints_as_floats.w;
    printf("Joints affecting vertex %d :  %d %d %d %d\n", v, tmp_joint_index.x, tmp_joint_index.y,
           tmp_joint_index.z, tmp_joint_index.w);
    Vec4u_darray_push(joint_indices, tmp_joint_index);
  }
}

bool model_load_gltf_str(const char *file_string, const char *filepath, Str8 relative_path,
                         Model *out_model, bool invert_textures_y) {
  TRACE("Load GLTF from string");

  // Setup temps
  Vec3_darray *tmp_positions = Vec3_darray_new(1000);
  Vec3_darray *tmp_normals = Vec3_darray_new(1000);
  Vec2_darray *tmp_uvs = Vec2_darray_new(1000);
  Vec4u_darray *tmp_joint_indices = Vec4u_darray_new(1000);
  Vec4_darray *tmp_weights = Vec4_darray_new(1000);
  Material_darray *tmp_materials = Material_darray_new(1);
  Mesh_darray *tmp_meshes = Mesh_darray_new(1);
  u32_darray *tmp_material_indexes = u32_darray_new(1);

  // FIXME
  // joint_darray *tmp_joints = joint_darray_new(256);
  // vertex_bone_data_darray *tmp_vertex_bone_data = vertex_bone_data_darray_new(1000);

  cgltf_options options = { 0 };
  cgltf_data *data = NULL;
  cgltf_result result = cgltf_parse_file(&options, filepath, &data);
  if (result != cgltf_result_success) {
    WARN("gltf load failed");
    // TODO: cleanup arrays(allocate all from arena ?)
    return false;
  }

  cgltf_load_buffers(&options, data, filepath);
  DEBUG("loaded buffers");

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
  size_t num_materials = GLTF_LoadMaterials(data, relative_path, tmp_materials);

  // --- Meshes
  size_t num_meshes = data->meshes_count;
  TRACE("Num meshes %d", num_meshes);
  for (size_t m = 0; m < num_meshes; m++) {
    printf("Primitive count %d\n", data->meshes[m].primitives_count);
    for (size_t prim_i = 0; prim_i < data->meshes[m].primitives_count; prim_i++) {
      DEBUG("Primitive %d\n", prim_i);

      cgltf_primitive primitive = data->meshes[m].primitives[prim_i];
      DEBUG("Found %d attributes", primitive.attributes_count);

      for (cgltf_size a = 0; a < primitive.attributes_count; a++) {
        cgltf_attribute attribute = primitive.attributes[a];
        if (attribute.type == cgltf_attribute_type_position) {
          cgltf_accessor *accessor = attribute.data;
          load_position_components(tmp_positions, accessor);
        } else if (attribute.type == cgltf_attribute_type_normal) {
          cgltf_accessor *accessor = attribute.data;
          load_normal_components(tmp_normals, accessor);
        } else if (attribute.type == cgltf_attribute_type_texcoord) {
          cgltf_accessor *accessor = attribute.data;
          load_texcoord_components(tmp_uvs, accessor);
        } else if (attribute.type == cgltf_attribute_type_joints) {
          TRACE("Load joint indices from accessor");
          cgltf_accessor *accessor = attribute.data;
          load_joint_index_components(tmp_joint_indices, accessor);
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
      i32 mat_idx = -1;
      if (primitive.material != NULL) {
        DEBUG("Primitive Material %s", primitive.material->name);
        // FIXME!
        for (u32 i = 0; i < Material_darray_len(tmp_materials); i++) {
          printf("%s vs %s \n", primitive.material->name, tmp_materials->data[i].name);
          if (strcmp(primitive.material->name, tmp_materials->data[i].name) == 0) {
            INFO("Found material");
            mat_idx = i;
            u32_darray_push(tmp_material_indexes, mat_idx);
            break;
          }
        }
      }

      TRACE("Vertex data has been loaded");

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
      printf("Positions %d Normals %d UVs %d\n", tmp_positions->len, tmp_normals->len,
             tmp_uvs->len);
      assert(tmp_positions->len == tmp_normals->len);
      assert(tmp_normals->len == tmp_uvs->len);
      for (u32 v_i = 0; v_i < tmp_positions->len; v_i++) {
        Vertex v = { .static_3d = {
                         .position = tmp_positions->data[v_i],
                         .normal = tmp_normals->data[v_i],
                         .tex_coords = tmp_uvs->data[v_i],
                     } };
        Vertex_darray_push(geo_vertices, v);
      }

      // Store indices
      cgltf_accessor *indices = primitive.indices;
      if (primitive.indices > 0) {
        WARN("indices! %d", indices->count);
        has_indices = true;

        // store indices
        for (cgltf_size i = 0; i < indices->count; ++i) {
          cgltf_uint ei;
          cgltf_accessor_read_uint(indices, i, &ei, 1);
          u32_darray_push(geo_indices, ei);
        }

        // fetch and store vertices for each index
        // for (cgltf_size i = 0; i < indices->count; ++i) {
        //   Vertex vert;
        //   cgltf_uint index = mesh.indices[i];
        //   vert.position = tmp_positions->data[index];
        //   vert.normal = tmp_normals->data[index];
        //   vert.uv = tmp_uvs->data[index];
        //   vertex_darray_push(mesh.vertices, vert);

        // if (is_skinned) {
        //   vertex_bone_data vbd = tmp_vertex_bone_data->data[index];  // create a copy
        //   vertex_bone_data_darray_push(mesh.vertex_bone_data, vbd);
        // }
        // for each vertex do the bone data
        // }
        // } else {
        // has_indices = false;
        // return false;  // TODO: handle this
        // }

        Geometry *geometry = malloc(sizeof(Geometry));
        geometry->format = VERTEX_STATIC_3D;
        geometry->has_indices = true;
        geometry->vertices = geo_vertices;
        geometry->indices = geo_indices;
        geometry->index_count = geo_indices->len;

        Mesh m = Mesh_Create(geometry, false);
        Mesh_darray_push(tmp_meshes, m);

        Vec3_darray_clear(tmp_positions);
        Vec3_darray_clear(tmp_normals);
        Vec2_darray_clear(tmp_uvs);
        Vec4u_darray_clear(tmp_joint_indices);
        Vec4_darray_clear(tmp_weights);
      }
    }

    //   for (int i = 0; i < out_model->meshes->len; i++) {
    //     u32 mat_idx = out_model->meshes->data[i].material_index;
    //     printf("Mesh %d Mat index %d Mat name %s\n", i, mat_idx,
    //            out_model->materials->data[mat_idx].name);
    //   }

    // TODO: GLTF_LoadAnimations()
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
    //         // FIXME: CASSERT_MSG function "Expected animation sampler input component to be
    //         type f32
    //         // (keyframe times)");
    //         f32 *times = arena_alloc(arena, n_frames * sizeof(f32));
    //         sampler->animation.n_timestamps = n_frames;
    //         sampler->animation.timestamps = times;
    //         cgltf_accessor_unpack_floats(channel.sampler->input, times, n_frames);

    //         assert_path_type_matches_component_type(channel.target_path,
    //         channel.sampler->output);

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
  }

  num_meshes = tmp_meshes->len;

  // we now have an array of meshes, materials, and the material which each mesh should get
  out_model->meshes = malloc(num_meshes * sizeof(MeshHandle));
  out_model->mesh_count = num_meshes;
  out_model->materials = malloc(num_materials * sizeof(MaterialHandle));
  out_model->material_count = num_materials;

  MaterialHandle *mat_handles = calloc(num_materials, sizeof(MaterialHandle));
  for (u32 mat_i = 0; mat_i < num_materials; mat_i++) {
    mat_handles[mat_i] =
        Material_pool_insert(Render_GetMaterialPool(), &tmp_materials->data[mat_i]);
  }
  memcpy(out_model->materials, mat_handles, num_materials * sizeof(MaterialHandle));

  for (u32 mesh_i = 0; mesh_i < num_meshes; mesh_i++) {
    u32 mat_idx = tmp_material_indexes->data[mesh_i];
    tmp_meshes->data[mesh_i].material = mat_handles[mat_idx];
    MeshHandle mesh = Mesh_pool_insert(Render_GetMeshPool(), &tmp_meshes->data[mesh_i]);
    out_model->meshes[mesh_i] = mesh;
  }

  free(mat_handles);

  return true;
}

const char *bool_yes_no(bool pred) { return pred ? "Yes" : "No"; }

// Loads all materials
size_t GLTF_LoadMaterials(cgltf_data *data, Str8 relative_path, Material_darray *out_materials) {
  size_t num_materials = data->materials_count;
  TRACE("Num materials %d", num_materials);
  for (size_t m = 0; m < num_materials; m++) {
    cgltf_material gltf_material = data->materials[m];
    TRACE("Loading material '%s'", gltf_material.name);
    cgltf_pbr_metallic_roughness pbr = gltf_material.pbr_metallic_roughness;

    Material our_material = PBRMaterialDefault();  // focusing on PBR materials for now

    our_material.base_colour =
        vec3(pbr.base_color_factor[0], pbr.base_color_factor[1], pbr.base_color_factor[2]);
    our_material.metallic = pbr.metallic_factor;
    our_material.roughness = pbr.roughness_factor;

    // -- albedo / base colour
    cgltf_texture_view albedo_tex_view = pbr.base_color_texture;
    bool has_albedo_texture = albedo_tex_view.texture != NULL;
    TRACE("Has PBR base colour texture?    %s", bool_yes_no(has_albedo_texture));
    printf("Base colour factor: %f %f %f\n", pbr.base_color_factor[0], pbr.base_color_factor[1],
           pbr.base_color_factor[2]);
    if (has_albedo_texture) {
      char albedo_map_path[1024];
      snprintf(albedo_map_path, sizeof(albedo_map_path), "%s/%s", relative_path.buf,
               albedo_tex_view.texture->image->uri);
      our_material.albedo_map = TextureLoadFromFile(albedo_map_path);
    } else {
      our_material.albedo_map = Render_GetWhiteTexture();
      WARN("GLTF model has no albedo map");
      our_material.base_colour = vec3_create(pbr.base_color_factor[0],pbr.base_color_factor[1], pbr.base_color_factor[2]);
    }

    // -- metallic
    cgltf_texture_view metal_rough_tex_view = pbr.metallic_roughness_texture;
    // bool has_metal_
    printf("Metal factor: %f\n", pbr.metallic_factor);
    printf("Roughness factor: %f\n", pbr.roughness_factor);
    if (metal_rough_tex_view.texture != NULL) {
      char metal_rough_map_path[1024];
      snprintf(metal_rough_map_path, sizeof(metal_rough_map_path), "%s/%s", relative_path.buf,
               metal_rough_tex_view.texture->image->uri);
      our_material.metallic_roughness_map = TextureLoadFromFile(metal_rough_map_path);
    } else {
      WARN("GLTF model has no metal/roughness map");
      our_material.metallic = pbr.metallic_factor;
      our_material.roughness = pbr.roughness_factor;
    }

    cgltf_texture_view normal_tex_view = gltf_material.normal_texture;
    if (normal_tex_view.texture != NULL) {
      char normal_map_path[1024];
      snprintf(normal_map_path, sizeof(normal_map_path), "%s/%s", relative_path.buf,
               normal_tex_view.texture->image->uri);
      our_material.normal_map = TextureLoadFromFile(normal_map_path);
    } else {
      WARN("GLTF model has no normal map");
    }

    u32 string_length = strlen(gltf_material.name) + 1;
    assert(string_length < 64);
    strcpy(our_material.name, gltf_material.name);

    Material_darray_push(out_materials, our_material);
  }

  return out_materials->len;
}
