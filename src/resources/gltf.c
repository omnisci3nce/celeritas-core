// TODO: Port code from old repo

/*
struct face {
  cgltf_uint indices[3];
};

// TODO: Brainstorm how I can make this simpler and break it up into more testable pieces

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