//
//  romulus.h
//  romulus
//
//  Created by Jacob Gordon on 10/21/16.
//  Copyright © 2016 Jacob Gordon. All rights reserved.
//

#ifndef romulus_h
#define romulus_h

#include "IDK.h"

typedef struct romulus_display_s* romulus_display ;

typedef struct romulus_render_stage_s* romulus_render_stage ;

typedef struct romulus_render_buffer_s* romulus_render_buffer ;

typedef struct romulus_shader_s* romulus_shader ;

typedef struct romulus_geometry_s* romulus_geometry ;

typedef struct romulus_material_s* romulus_material ;

typedef struct romulus_mesh_s* romulus_mesh ;

typedef struct romulus_entity_s* romulus_entity ;

typedef struct romulus_camera_s* romulus_camera ;

typedef void (*romulus_attributes)(unsigned int shader_id) ;

typedef void (*romulus_init_material)(romulus_shader shader) ;

typedef void (*romulus_deinit_material)(romulus_shader shader) ;

romulus_display romulus_new_display( int window_width, int window_height, const char* window_title, float fov, float nearZ, float farZ, int vsync ) ;

void romulus_destroy_display( romulus_display display ) ;

void romulus_bind_texture_unit( unsigned int tunit ) ;

void romulus_set_display_vsync( romulus_display display, int vsync ) ;

int romulus_is_display_vsync( romulus_display display ) ;

void romulus_set_active_display( romulus_display display ) ;

void romulus_builtin_attributes_one( unsigned int shader_id ) ;

romulus_shader romulus_new_shader( romulus_display display, romulus_attributes attributes ) ;

void romulus_load_transform_matrix_to_shader( RKMVector matrix, romulus_shader shader ) ;

void romulus_load_projection_matrix_to_shader( RKMVector matrix, romulus_shader shader ) ;

void romulus_load_view_matrix_to_shader( RKMVector matrix, romulus_shader shader ) ;

void romulus_new_projection_matrix( RKMVector matrix, float fov, float aspect, float nearZ, float farZ ) ;

void romulus_new_transform_matrix( RKMVector matrix, RKMVector pos, RKMVector rot, float scale ) ;

void romulus_new_view_matrix( RKMVector matrix, RKMVector pos, float pitch, float yaw, float roll ) ;

romulus_camera romulus_new_camera( RKMVector pos, float pitch, float yaw, float roll ) ;

void romulus_destroy_camera( romulus_camera camera ) ;

void romulus_camera_get_pos( romulus_camera camera, RKMVector pos ) ;

void romulus_camera_set_pos( romulus_camera camera, RKMVector pos ) ;

romulus_mesh romulus_new_mesh( romulus_attributes attributes, RKMVector vertexes, int v_size, RKMVector indexes, int i_size, RKMVector texture_coords ) ;

static void romulus_destroy_entity_from_mesh( void* entity ) ;

void romulus_destroy_mesh( romulus_mesh mesh ) ;

romulus_entity romulus_new_entity( romulus_mesh mesh, RKMVector pos, RKMVector rot, float scale ) ;

void romulus_destroy_entity( romulus_entity entity ) ;

void romulus_set_entity_pos( romulus_entity entity, RKMVector pos ) ;

void romulus_get_entity_pos( romulus_entity entity, RKMVector pos ) ;

void romulus_set_entity_rot( romulus_entity entity, RKMVector rot ) ;

void romulus_get_entity_rot( romulus_entity entity, RKMVector rot ) ;

void romulus_set_entity_scale( romulus_entity entity, float scale ) ;

float romulus_get_entity_scale( romulus_entity entity ) ;

void romulus_add_to_entity_pos( romulus_entity entity, RKMVector vec ) ;

void romulus_add_to_entity_rot( romulus_entity entity, RKMVector vec ) ;

void romulus_render( romulus_render_stage stage ) ;

#endif /* romulus_h */
