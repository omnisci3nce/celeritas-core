/**
 * @file obj.c
 * @brief Wavefront OBJ loader.
 * @copyright Copyright (c) 2024
 */
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core.h"
#include "darray.h"
#include "file.h"
#include "log.h"
#include "maths.h"
#include "path.h"
#include "render.h"
#include "render_types.h"
#include "str.h"

struct face {
  u32 vertex_indices[3];
  u32 normal_indices[3];
  u32 uv_indices[3];
};
typedef struct face face;

KITC_DECL_TYPED_ARRAY(vec3)
KITC_DECL_TYPED_ARRAY(vec2)
KITC_DECL_TYPED_ARRAY(face)

// Forward declarations
void create_submesh(mesh_darray *meshes, vec3_darray *tmp_positions, vec3_darray *tmp_normals,
                    vec2_darray *tmp_uvs, face_darray *tmp_faces, material_darray *materials,
                    bool material_loaded, char current_material_name[256]);
bool load_material_lib(const char *path, str8 relative_path, material_darray *materials);
bool model_load_obj_str(const char *file_string, str8 relative_path, model *out_model,
                        bool invert_textures_y);

model_handle model_load_obj(core *core, const char *path, bool invert_textures_y) {
  TRACE("Loading model at Path %s\n", path);
  path_opt relative_path = path_parent(path);
  if (!relative_path.has_value) {
    WARN("Couldnt get a relative path for the path to use for loading materials & textures later");
  }
  printf("Relative path: %s\n", relative_path.path.buf);
  const char *file_string = string_from_file(path);

  // TODO: store the relative path without the name.obj at the end

  model model = { 0 };
  model.name = str8_cstr_view(path);
  model.meshes = mesh_darray_new(1);
  model.materials = material_darray_new(1);

  bool success = model_load_obj_str(file_string, relative_path.path, &model, invert_textures_y);

  if (!success) {
    FATAL("Couldnt load OBJ file at path %s", path);
    ERROR_EXIT("Load fails are considered crash-worthy right now. This will change later.\n");
  }

  u32 index = model_darray_len(core->models);
  model_darray_push(core->models, model);
  return (model_handle){ .raw = index };
}

bool model_load_obj_str(const char *file_string, str8 relative_path, model *out_model,
                        bool invert_textures_y) {
  TRACE("Load OBJ from string");

  // Setup temps
  vec3_darray *tmp_positions = vec3_darray_new(1000);
  vec3_darray *tmp_normals = vec3_darray_new(1000);
  vec2_darray *tmp_uvs = vec2_darray_new(1000);
  face_darray *tmp_faces = face_darray_new(1000);
  // TODO: In the future I'd like these temporary arrays to be allocated from an arena provided
  // by the function one level up, model_load_obj. That way we can just `return false;` anywhere in
  // this code to indicate an error, and be sure that all that memory will be cleaned up without
  // having to call vec3_darray_free in every single error case before returning.

  // Other state
  bool object_set = false;
  bool material_loaded = false;
  char current_material_name[64];

  char *pch;
  char *rest = file_string;
  pch = strtok_r((char *)file_string, "\n", &rest);

  int line_num = 0;
  char last_char_type = 'a';

  while (pch != NULL) {
    line_num++;
    char line_header[128];
    int offset = 0;

    // skip whitespace
    char *p = pch;

    skip_space(pch);

    if (*p == '\0') {
      /* the string is empty */
    } else {
      // read the first word of the line
      int res = sscanf(pch, "%s %n", line_header, &offset);
      /* printf("header: %s, offset : %d res: %d\n",line_header, offset, res); */
      if (res != 1) {
        break;
      }

      if (strcmp(line_header, "o") == 0 || strcmp(line_header, "g") == 0) {
        // if we're currently parsing one
        if (!object_set) {
          object_set = true;
        } else {
          create_submesh(out_model->meshes, tmp_positions, tmp_normals, tmp_uvs, tmp_faces,
                         out_model->materials, material_loaded, current_material_name);
          object_set = false;
        }
      } else if (strcmp(line_header, "v") == 0) {
        // special logic: if we went from faces back to vertices trigger a mesh output.
        // PS: I hate OBJ
        if (last_char_type == 'f') {
          create_submesh(out_model->meshes, tmp_positions, tmp_normals, tmp_uvs, tmp_faces,
                         out_model->materials, material_loaded, current_material_name);
          object_set = false;
        }

        last_char_type = 'v';
        vec3 vertex;
        sscanf(pch + offset, "%f %f %f", &vertex.x, &vertex.y, &vertex.z);

        vec3_darray_push(tmp_positions, vertex);
      } else if (strcmp(line_header, "vt") == 0) {
        last_char_type = 't';
        vec2 uv;
        char copy[1024];
        memcpy(copy, pch + offset, strlen(pch + offset) + 1);
        char *p = pch + offset;
        while (isspace((unsigned char)*p)) ++p;

        // I can't remember what is going on here
        memset(copy, 0, 1024);
        memcpy(copy, pch + offset, strlen(pch + offset) + 1);
        int res = sscanf(copy, "%f %f", &uv.x, &uv.y);
        memset(copy, 0, 1024);
        memcpy(copy, pch + offset, strlen(pch + offset) + 1);
        if (res != 1) {
          // da frick? some .obj files have 3 uvs instead of 2
          f32 dummy;
          int res2 = sscanf(copy, "%f %f %f", &uv.x, &uv.y, &dummy);
        }

        if (invert_textures_y) {
          uv.y = -uv.y;  // flip Y axis to be consistent with how other PNGs are being handled
          // `texture_load` will flip it again
        }
        vec2_darray_push(tmp_uvs, uv);
      } else if (strcmp(line_header, "vn") == 0) {
        last_char_type = 'n';
        vec3 normal;
        sscanf(pch + offset, "%f %f %f", &normal.x, &normal.y, &normal.z);
        vec3_darray_push(tmp_normals, normal);
      } else if (strcmp(line_header, "f") == 0) {
        last_char_type = 'f';
        struct face f;
        sscanf(pch + offset, "%d/%d/%d %d/%d/%d %d/%d/%d", &f.vertex_indices[0], &f.uv_indices[0],
               &f.normal_indices[0], &f.vertex_indices[1], &f.uv_indices[1], &f.normal_indices[1],
               &f.vertex_indices[2], &f.uv_indices[2], &f.normal_indices[2]);
        // printf("f %d/%d/%d %d/%d/%d %d/%d/%d\n", f.vertex_indices[0], f.uv_indices[0],
        // f.normal_indices[0],
        //   f.vertex_indices[1], f.uv_indices[1], f.normal_indices[1],
        //   f.vertex_indices[2], f.uv_indices[2], f.normal_indices[2]);
        face_darray_push(tmp_faces, f);
      } else if (strcmp(line_header, "mtllib") == 0) {
        char filename[1024];
        sscanf(pch + offset, "%s", filename);
        char mtllib_path[1024];
        snprintf(mtllib_path, sizeof(mtllib_path), "%s/%s", relative_path.buf, filename);
        if (!load_material_lib(mtllib_path, relative_path, out_model->materials)) {
          ERROR("couldnt load material lib");
          return false;
        }
      } else if (strcmp(line_header, "usemtl") == 0) {
        material_loaded = true;
        sscanf(pch + offset, "%s", current_material_name);
      }
    }

    pch = strtok_r(NULL, "\n", &rest);
  }

  // last mesh or if one wasnt created with 'o' directive
  if (face_darray_len(tmp_faces) > 0) {
    TRACE("Last leftover mesh");
    create_submesh(out_model->meshes, tmp_positions, tmp_normals, tmp_uvs, tmp_faces,
                   out_model->materials, material_loaded, current_material_name);
  }

  // Free data
  free((char *)file_string);
  vec3_darray_free(tmp_positions);
  vec3_darray_free(tmp_normals);
  vec2_darray_free(tmp_uvs);
  face_darray_free(tmp_faces);
  TRACE("Freed temporary OBJ loading data");

  if (mesh_darray_len(out_model->meshes) > 256) {
    printf("num meshes: %ld\n", mesh_darray_len(out_model->meshes));
  }

  // TODO: bounding box calculation for each mesh
  // TODO: bounding box calculation for model

  return true;
}

/**
 * @brief Takes the current positions, normals, uvs arrays and constructs the vertex array
 *        from those indices.
 */
void create_submesh(mesh_darray *meshes, vec3_darray *tmp_positions, vec3_darray *tmp_normals,
                    vec2_darray *tmp_uvs, face_darray *tmp_faces, material_darray *materials,
                    bool material_loaded, char current_material_name[256]) {
  size_t num_verts = face_darray_len(tmp_faces) * 3;
  vertex_darray *out_vertices = vertex_darray_new(num_verts);

  face_darray_iter face_iter = face_darray_iter_new(tmp_faces);
  struct face *f;

  while ((f = face_darray_iter_next(&face_iter))) {
    for (int j = 0; j < 3; j++) {
      vertex vert = { 0 };
      vert.position = tmp_positions->data[f->vertex_indices[j] - 1];
      if (vec3_darray_len(tmp_normals) == 0) {
        vert.normal = vec3_create(0.0, 0.0, 0.0);
      } else {
        vert.normal = tmp_normals->data[f->normal_indices[j] - 1];
      }
      vert.uv = tmp_uvs->data[f->uv_indices[j] - 1];
      vertex_darray_push(out_vertices, vert);
    }
  }

  DEBUG("Loaded submesh\n  vertices: %zu\n  uvs: %zu\n  normals: %zu\n  faces: %zu",
        vec3_darray_len(tmp_positions), vec2_darray_len(tmp_uvs), vec3_darray_len(tmp_normals),
        face_darray_len(tmp_faces));

  // Clear current object faces
  face_darray_clear(tmp_faces);

  mesh m = { .vertices = out_vertices };
  if (material_loaded) {
    // linear scan to find material
    bool found = false;
    DEBUG("Num of materials : %ld", material_darray_len(materials));
    material_darray_iter mat_iter = material_darray_iter_new(materials);
    blinn_phong_material *cur_material;
    while ((cur_material = material_darray_iter_next(&mat_iter))) {
      if (strcmp(cur_material->name, current_material_name) == 0) {
        DEBUG("Found match");
        m.material_index = mat_iter.current_idx - 1;
        found = true;
        break;
      }
    }

    if (!found) {
      // TODO: default material
      m.material_index = 0;
      DEBUG("Set default material");
    }
  }
  mesh_darray_push(meshes, m);
}

bool load_material_lib(const char *path,str8 relative_path, material_darray *materials) {
  TRACE("BEGIN load material lib at %s", path);

  const char *file_string = string_from_file(path);
  if (file_string == NULL) {
    ERROR("couldnt load %s", path);
    return false;
  }

  char *pch;
  char *saveptr;
  pch = strtok_r((char *)file_string, "\n", &saveptr);

  material current_material = DEFAULT_MATERIAL;

  bool material_set = false;

  while (pch != NULL) {
    char line_header[128];
    int offset = 0;
    // read the first word of the line
    int res = sscanf(pch, "%s %n", line_header, &offset);
    if (res != 1) {
      break;
    }

    // When we see "newmtl", start a new material, or flush the previous one
    if (strcmp(line_header, "newmtl") == 0) {
      if (material_set) {
        // a material was being parsed, so flush that one and start a new one
        material_darray_push(materials, current_material);
        DEBUG("pushed material with name %s", current_material.name);
        WARN("Reset current material");
        current_material = DEFAULT_MATERIAL;
      } else {
        material_set = true;
      }
      // scan the new material name
      char material_name[64];
      sscanf(pch + offset, "%s", current_material.name);
      DEBUG("material name %s\n", current_material.name);
      // current_material.name = material_name;
    } else if (strcmp(line_header, "Ka") == 0) {
      // ambient
      sscanf(pch + offset, "%f %f %f", &current_material.ambient_colour.x,
             &current_material.ambient_colour.y, &current_material.ambient_colour.z);
    } else if (strcmp(line_header, "Kd") == 0) {
      // diffuse
      sscanf(pch + offset, "%f %f %f", &current_material.diffuse.x, &current_material.diffuse.y,
             &current_material.diffuse.z);
    } else if (strcmp(line_header, "Ks") == 0) {
      // specular
      sscanf(pch + offset, "%f %f %f", &current_material.specular.x, &current_material.specular.y,
             &current_material.specular.z);
    } else if (strcmp(line_header, "Ns") == 0) {
      // specular exponent
      sscanf(pch + offset, "%f", &current_material.spec_exponent);
    } else if (strcmp(line_header, "map_Kd") == 0) {
      char diffuse_map_filename[1024];
      sscanf(pch + offset, "%s", diffuse_map_filename);
      char diffuse_map_path[1024];
      snprintf(diffuse_map_path, sizeof(diffuse_map_path), "%s/%s", relative_path.buf, diffuse_map_filename);
      printf("load from %s\n", diffuse_map_path);

      // --------------
      texture diffuse_texture = texture_data_load(diffuse_map_path, true);
      current_material.diffuse_texture = diffuse_texture;
      strcpy(current_material.diffuse_tex_path, diffuse_map_path);
      texture_data_upload(&current_material.diffuse_texture);
      // --------------
    } else if (strcmp(line_header, "map_Ks") == 0) {
      // char specular_map_path[1024] = "assets/";
      // sscanf(pch + offset, "%s", specular_map_path + 7);
      char specular_map_filename[1024];
      sscanf(pch + offset, "%s", specular_map_filename);
      char specular_map_path[1024];
      snprintf(specular_map_path, sizeof(specular_map_path), "%s/%s", relative_path.buf, specular_map_filename);
      printf("load from %s\n", specular_map_path);
      // --------------
      texture specular_texture = texture_data_load(specular_map_path, true);
      current_material.specular_texture = specular_texture;
      strcpy(current_material.specular_tex_path, specular_map_path);
      texture_data_upload(&current_material.specular_texture);
      // --------------
    } else if (strcmp(line_header, "map_Bump") == 0) {
      // TODO
    }

    pch = strtok_r(NULL, "\n", &saveptr);
  }

  TRACE("end load material lib");

  // last mesh or if one wasnt created with 'o' directive
  // TRACE("Last leftover material");
  material_darray_push(materials, current_material);

  INFO("Loaded %ld materials", material_darray_len(materials));
  TRACE("END load material lib");
  return true;
}
