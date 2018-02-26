/*
 Copyright (c) 2016-2018 Jacob Gordon. All rights reserved.
 
 Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 
 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 
 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#ifndef romulus_h
#define romulus_h

#include <IDK/IDK.h>

#include <romulus/romulus2d.h>

typedef struct romulus_window_s* romulus_window ;

typedef struct romulus_app_s* romulus_app ;

typedef struct romulus_scene_s* romulus_scene ;

typedef struct romulus_render_stage_s* romulus_render_stage ;

typedef struct romulus_render_buffer_s* romulus_render_buffer ;

typedef struct romulus_shader_s* romulus_shader ;

typedef struct romulus_geometry_s* romulus_geometry ;

typedef struct romulus_material_s* romulus_material ;

typedef struct romulus_mesh_s* romulus_mesh ;

typedef struct romulus_entity_s* romulus_entity ;

typedef struct romulus_camera_s* romulus_camera ;

typedef struct romulus_texture_s* romulus_texture ;

typedef struct romulus_texture_array_s* romulus_texture_array ;

typedef enum { romulus_byte_type_id, romulus_sbyte_type_id,
    
romulus_short_type_id, romulus_ushort_type_id,
    
romulus_int_type_id, romulus_uint_type_id,
    
romulus_float_type_id, romulus_double_type_id } romulus_data_type ;

typedef enum { romulus_true = 1, romulus_false = 0 } romulus_bool ;

typedef void (*romulus_attributes_binder)(romulus_shader shader) ;

typedef void (*romulus_attributes_constructor)(int vertex_count, RKArgs args) ;

typedef void (*romulus_init_material)(romulus_shader shader, RKArgs material_args) ;

typedef void (*romulus_update_uniforms)(romulus_shader shader, RKArgs uniform_args) ;

typedef IDKWindowRunLoopFuncType romulus_run_loop_func_type ;

typedef IDKWindowQuitRunLoopFuncType romulus_run_quit_loop_func_type ;

void romulus_report_error( romulus_scene scene, const char* report_name ) ;

romulus_app romulus_new_app( RKString app_name, float version, romulus_bool logging ) ;

void romulus_destroy_app( romulus_app app ) ;

IDKApp romulus_get_idk_app( romulus_app app ) ;

romulus_window romulus_new_window( romulus_app app, int win_width, int win_height, const char* win_title, romulus_bool vsync, romulus_bool scene_logging ) ;

void romulus_enter_fullscreen( romulus_window window ) ;

void romulus_exit_fullscreen( romulus_window window ) ;

romulus_app romulus_get_app_from_window( romulus_window window ) ;

romulus_scene romulus_get_scene_from_window( romulus_window window ) ;

IDKWindow romulus_get_idk_window( romulus_window window ) ;

void romulus_destroy_scene( romulus_scene scene ) ;

void romulus_set_scene_vsync( romulus_scene scene, romulus_bool vsync ) ;

romulus_bool romulus_is_scene_vsync( romulus_scene scene ) ;

romulus_window romulus_get_window_from_scene( romulus_scene scene ) ;

void romulus_set_active_scene( romulus_scene scene ) ;

void romulus_enable_scene_logging( romulus_scene scene, romulus_bool logging ) ;

int romulus_check_extension( const char* extension ) ;

int romulus_get_max_texture_units( romulus_scene scene ) ;

romulus_shader romulus_new_shader( romulus_scene scene, const char* vertex_shader, const char* fragment_shader, romulus_attributes_binder attributes ) ;

void romulus_destroy_shader( romulus_shader shader ) ;

void romulus_bind_attribute_location_to_shader( const char* attribute_name, unsigned int location, romulus_shader shader )  ;

romulus_texture romulus_new_texture( romulus_scene scene, const char* texture_name, unsigned int texture_unit, romulus2d_texture raw_texture, float quality ) ;

void romulus_destroy_texture( romulus_texture texture ) ;

void romulus_load_texture_to_shader( romulus_texture texture, romulus_shader shader ) ;

int romulus_get_texture_width( romulus_texture texture ) ;

int romulus_get_texture_height( romulus_texture texture ) ;

romulus2d_texture_format romulus_get_texture_format( romulus_texture texture ) ;

romulus_texture_array romulus_new_texture_array( romulus_scene scene, romulus2d_texture_format format, const char* texture_name,
                                                unsigned int texture_unit, int width, int height, int max_num_of_textures ) ;

void romulus_destroy_texture_array( romulus_texture_array texture_array ) ;

void romulus_load_texture_array_to_shader( romulus_texture_array texture_array, romulus_shader shader ) ;

void romulus_add_texture_to_texture_array( romulus2d_texture texture, romulus_texture_array texture_array ) ;

void romulus_set_texture_array_quality( romulus_texture_array texture_array, float quality ) ;

int romulus_get_texture_array_width( romulus_texture_array texture_array ) ;

int romulus_get_texture_array_height( romulus_texture_array texture_array ) ;

int romulus_get_texture_array_max_num_of_textures( romulus_texture_array texture_array ) ;

int romulus_get_texture_array_num_of_textures( romulus_texture_array texture_array ) ;

romulus2d_texture_format romulus_get_texture_array_format( romulus_texture_array texture_array ) ;

romulus_material romulus_new_material( romulus_shader shader, romulus_init_material init, RKArgs material_args ) ;

void romulus_destroy_material( romulus_material material ) ;

int romulus_is_material_active( romulus_material material ) ;

void romulus_set_material_active( romulus_material material, int is_active ) ;

void romulus_add_mesh_to_material( romulus_mesh mesh, romulus_material material )  ;

void romulus_new_perspective_matrix( RKMVector matrix, int width, int height, float fov, float nearZ, float farZ ) ;

void romulus_new_ographic_matrix( RKMVector matrix, float left, float right, float bottom, float top, float nearZ, float farZ ) ;

void romulus_new_transform_matrix( RKMVector matrix, RKMVector pos, RKMVector rot, float scale ) ;

void romulus_new_view_matrix( RKMVector matrix, RKMVector pos, float pitch, float yaw, float roll ) ;

romulus_camera romulus_new_camera( RKMVector pos, float pitch, float yaw, float roll ) ;

void romulus_destroy_camera( romulus_camera camera ) ;

void romulus_camera_get_pos( romulus_camera camera, RKMVector pos ) ;

void romulus_camera_set_pos( romulus_camera camera, RKMVector pos ) ;

void romulus_camera_set_pitch( romulus_camera camera, float pitch ) ;

void romulus_camera_set_yaw( romulus_camera camera, float yaw ) ;

void romulus_camera_set_roll( romulus_camera camera, float roll ) ;

void romulus_update_camera( romulus_camera camera ) ;

romulus_mesh romulus_new_mesh( romulus_scene scene, romulus_attributes_constructor attributes_constructor, RKArgs attribute_args, romulus_attributes_binder attributes,
                              
int vertex_count, unsigned int* indexes, int index_count ) ;

void romulus_destroy_mesh( romulus_mesh mesh ) ;

void romulus_construct_attribute( unsigned int index, void* data, romulus_data_type data_type, int num_of_data_per_element, int num_of_elements, romulus_bool is_normalized ) ;

void romulus_enable_attribute_with_mesh( romulus_mesh mesh, unsigned int index ) ;

void romulus_disable_attribute_with_mesh( romulus_mesh mesh, unsigned int index ) ;

void romulus_load_float_to_shader( float value, RKString value_name, romulus_shader shader ) ;

void romulus_load_transform_matrix_to_shader( RKMVector matrix, romulus_shader shader ) ;

void romulus_load_projection_matrix_to_shader( RKMVector matrix, romulus_shader shader ) ;

void romulus_load_view_matrix_to_shader( RKMVector matrix, romulus_shader shader ) ;

romulus_entity romulus_new_entity( romulus_mesh mesh, RKMVector pos, RKMVector rot, float scale, romulus_update_uniforms update_uniforms, RKArgs uniforms_args ) ;

void romulus_destroy_entity( romulus_entity entity ) ;

void romulus_swap_entity_order( romulus_entity entity_a, romulus_entity entity_b ) ;

void romulus_set_entity_pos( romulus_entity entity, RKMVector pos ) ;

void romulus_get_entity_pos( romulus_entity entity, RKMVector pos ) ;

void romulus_set_entity_rot( romulus_entity entity, RKMVector rot ) ;

void romulus_get_entity_rot( romulus_entity entity, RKMVector rot ) ;

void romulus_set_entity_scale( romulus_entity entity, float scale ) ;

float romulus_get_entity_scale( romulus_entity entity ) ;

void romulus_add_to_entity_pos( romulus_entity entity, RKMVector vec ) ;

void romulus_add_to_entity_rot( romulus_entity entity, RKMVector vec ) ;

romulus_geometry romulus_new_geometry( void ) ;

void romulus_destroy_geometry( romulus_geometry geometry ) ;

void romulus_add_material_to_geometry( romulus_material material, romulus_geometry geometry ) ;

romulus_render_buffer romulus_new_render_buffer( romulus_scene scene, int width, int height ) ;

void romulus_destroy_render_buffer( romulus_render_buffer render_buffer ) ;

void romulus_load_render_buffer_as_texture_to_shader( romulus_render_buffer render_buffer, const char* texture_name, unsigned int tunit, romulus_shader shader ) ;

int romulus_get_render_buffer_width( romulus_render_buffer render_buffer ) ;

int romulus_get_render_buffer_height( romulus_render_buffer render_buffer ) ;

romulus_render_stage romulus_new_render_stage( romulus_camera camera, RKMVector projection_matrix, romulus_render_buffer render_buffer, romulus_geometry geometry ) ;

void romulus_destroy_render_stage( romulus_render_stage stage ) ;

romulus_camera romulus_get_camera_from_stage( romulus_render_stage stage ) ;

romulus_render_buffer romulus_get_render_buffer_from_stage( romulus_render_stage stage ) ;

romulus_geometry romulus_get_geometry_from_stage( romulus_render_stage stage ) ;

void romulus_set_cull_back_face( romulus_render_stage stage, romulus_bool cull ) ;

void romulus_set_depth_test( romulus_render_stage stage, romulus_bool depth ) ;

void romulus_set_depth_alpha( romulus_render_stage stage, romulus_bool alpha ) ;

void romulus_set_render_stage_projection_matrix( romulus_render_stage stage, RKMVector projection_matrix ) ;

void romulus_render( romulus_render_stage stage ) ;

void romulus_present( romulus_render_buffer render_buffer ) ;

void romulus_run_loop( romulus_scene scene, romulus_run_loop_func_type run_loop_func, RKArgs run_args, romulus_run_quit_loop_func_type run_quit_loop_func, RKArgs quit_args ) ;

#endif /* romulus_h */
