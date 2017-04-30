/*
 Copyright (c) 2017 Jacob Gordon. All rights reserved.
 
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

#include <stdio.h>
#include <stdlib.h>
#include <romulus/romulus.h>

static const char* vertex_shader1 =
"#version 330\n"
"in vec3  position ;"
"in vec2  texcoord ;"
"out vec2 out_texcoord ;"
"uniform mat4 transform_matrix ;"
"uniform mat4 projection_matrix ;"
"uniform mat4 view_matrix ;"
"void main() {"
"gl_Position = projection_matrix * view_matrix * transform_matrix * vec4(position,1.0) ;"
"out_texcoord = texcoord ;"
"}";

static const char* fragment_shader1 =
"#version 330\n"
"in vec2      out_texcoord ;"
"out vec4     fragColor ;"
"uniform sampler2D romulus_texture_1 ;"
"void main() {"
"fragColor = (texture(romulus_texture_1, out_texcoord.st)) ;"
"}";

float cube_vertices[] = {
    // front
    -1.0, -1.0,  1.0,
    1.0, -1.0,  1.0,
    1.0,  1.0,  1.0,
    -1.0,  1.0,  1.0,
    // back
    -1.0, -1.0, -1.0,
    1.0, -1.0, -1.0,
    1.0,  1.0, -1.0,
    -1.0,  1.0, -1.0,
};

float cube_textcoords[] = {
				
				0,0,
				0,1,
				1,1,
				1,0,
				0,0,
				0,1,
				1,1,
				1,0,
				0,0,
				0,1,
				1,1,
				1,0,
				0,0,
				0,1,
				1,1,
				1,0,
				0,0,
				0,1,
				1,1,
				1,0,
				0,0,
				0,1,
				1,1,
				1,0
    
				
};

unsigned int cube_elements[] = {
    // front
    0, 1, 2,
    2, 3, 0,
    // top
    1, 5, 6,
    6, 2, 1,
    // back
    7, 6, 5,
    5, 4, 7,
    // bottom
    4, 0, 3,
    3, 7, 4,
    // left
    4, 5, 1,
    1, 0, 4,
    // right
    3, 2, 6,
    6, 7, 3,
};

void attributes_constructor( int vertex_count, RKArgs args ) {
    
    RKArgs_UseArgs(args) ;
    
    romulus_construct_attribute(0, RKArgs_GetNextArg(args, float*), romulus_float_type_id, 3, vertex_count, romulus_false) ;
    
    romulus_construct_attribute(1, RKArgs_GetNextArg(args, float*), romulus_float_type_id, 2, vertex_count, romulus_true) ;
}

void attributes_binder( romulus_shader shader ) {
    
    romulus_bind_attribute_location_to_shader("position", 0, shader) ;
    
    romulus_bind_attribute_location_to_shader("texcoord", 1, shader) ;
}

void material_init( romulus_shader shader, RKArgs material_args ) {
    
    RKArgs_UseArgs(material_args) ;
    
    romulus_texture the_texture = *RKArgs_GetNextArg(material_args, romulus_texture*) ;
    
    romulus_load_texture_to_shader(the_texture, shader) ;
}

void draw( romulus2d_rect rect, RKArgs args ) {
   
    char fps_string[100] ;
    
    RKArgs_UseArgs(args) ;
    
    romulus_app app = RKArgs_GetNextArg(args, romulus_app) ;
    
    romulus_window window = RKArgs_GetNextArg(args, romulus_window) ;
    
    IDKApp App = romulus_get_idk_app(app) ;
    
    IDKWindow idk_window = romulus_get_idk_window(window) ;
    
    IDK_GetFPSHPString(App, fps_string) ;
    
    romulus2d_draw_string(rect, fps_string, 0.2, 0.1, 0.9, 1, 1, 1, 1) ;
    
    romulus2d_draw_string(rect, "Hello World", 0.2, IDK_GetMouseX(idk_window), IDK_GetMouseY(idk_window), 1, 1, 1, 1) ;
}

void runframe( RKArgs args ) {
    
    romulus_window window = RKArgs_GetArgWithIndex(args, romulus_window, 1) ;
    
    romulus_texture* texture = RKArgs_GetArgWithIndex(args, romulus_texture*, 2) ;
    
    romulus_scene scene = RKArgs_GetArgWithIndex(args, romulus_scene, 3) ;
    
    romulus2d_rect draw_rect = RKArgs_GetArgWithIndex(args, romulus2d_rect, 4) ;
    
    romulus_camera camera = RKArgs_GetArgWithIndex(args, romulus_camera, 5) ;
    
    romulus_entity entity = RKArgs_GetArgWithIndex(args, romulus_entity, 11 ) ;
    
    romulus_render_stage stage = RKArgs_GetArgWithIndex(args, romulus_render_stage, 12) ;
    
    IDKWindow idk_window = romulus_get_idk_window(window) ;
    
    static RKMath_Vectorit(pos, 0, 0, 0) ;
    
    static RKMath_Vectorit(rot, 0, 0, 0) ;
    
    static float picth = 0 ;
    
    static float yaw = 0 ;
    
    static float roll = 0 ;
    
    if ( IDK_GetKey(idk_window,idk_up_key) ) pos[2] += 0.01 ;
    
    if ( IDK_GetKey(idk_window,idk_down_key) ) pos[2] -= 0.01 ;
    
    if ( IDK_GetKey(idk_window,idk_right_key) ) pos[0] += 0.01 ;
    
    if ( IDK_GetKey(idk_window,idk_left_key) ) pos[0] -= 0.01 ;
    
    if ( IDK_GetKey(idk_window,idk_r_key) ) pos[1] += 0.01 ;
    
    if ( IDK_GetKey(idk_window,idk_f_key) ) pos[1] -= 0.01 ;
    
    if ( IDK_GetKey(idk_window,idk_w_key) ) rot[0] += 1 ;
    
    if ( IDK_GetKey(idk_window,idk_s_key) ) rot[0] -= 1 ;
    
    if ( IDK_GetKey(idk_window,idk_a_key) ) rot[1] += 1 ;
    
    if ( IDK_GetKey(idk_window,idk_d_key) ) rot[1] -= 1 ;
    
    if ( IDK_GetKey(idk_window,idk_q_key) ) rot[2] += 1 ;
    
    if ( IDK_GetKey(idk_window,idk_e_key) ) rot[2] -= 1 ;
    
    romulus_camera_set_pos(camera, pos) ;
    
    romulus_camera_set_pitch(camera, picth) ;
    
    romulus_camera_set_yaw(camera, yaw) ;
    
    romulus_camera_set_roll(camera, roll) ;
    
    romulus_update_camera(camera) ;
    
    romulus_set_entity_rot(entity, rot) ;
    
    romulus2d_render_rect(draw_rect, args) ;
    
    *texture = romulus_new_texture(scene, "romulus_texture_1", 0, romulus2d_get_texture_from_rect(draw_rect), 0) ;
   
    romulus_render(stage) ;
    
    romulus_present(stage) ;
   
    romulus_destroy_texture(*texture) ;
}

void shutdownapp( RKArgs args ) {
    
    
}

int main(int argc, const char * argv[]) {
    
    RKMath_Vectorit(pos, 0, 0, -5) ;
    
    RKMath_Vectorit(rot, 0, 0, 0) ;
    
    RKMath_Vectorit(color, 0, 0, 0) ;
    
    RKMath_NewVector(projection_matrix, 16) ;
    
    romulus_texture* texture = RKMem_NewMemOfType(romulus_texture) ;
    
    romulus_app app = romulus_new_app(rkstr("romulus_test_app"), 0, romulus_true) ;
    
    romulus_window window = romulus_new_window(app, 1366, 768, "Hello World!!!!", romulus_true) ;
    
    romulus_scene scene = romulus_get_scene_from_window(window) ;
    
    romulus2d_rect draw_rect = romulus2d_new_rect(NULL, draw, color, 1024, 1024, romulus2d_rgba8_texture_format) ;
    
    romulus_camera camera = romulus_new_camera(pos, 0, 0, 0) ;
    
    romulus_mesh cube = romulus_new_mesh(scene, attributes_constructor, newargs( args(float*,cube_vertices,cube_textcoords) ), attributes_binder, 8, cube_elements, 36) ;
   
    romulus_shader shader = romulus_new_shader(scene, vertex_shader1, fragment_shader1, attributes_binder) ;
    
    romulus_new_perspective_matrix(projection_matrix, 1920, 1080, 60, 0.1f, 1000.0f) ;
    
    romulus_render_buffer render_buffer = romulus_new_render_buffer(scene, 1920, 1080, projection_matrix) ;
    
    romulus_geometry geometry = romulus_new_geometry() ;
    
    romulus_material material = romulus_new_material(shader, material_init, newargs( args(romulus_texture*, texture) )) ;
    
    romulus_entity entity = romulus_new_entity(cube, pos, rot, 1) ;
    
    romulus_add_mesh_to_material(cube, material) ;
    
    romulus_add_material_to_geometry(material, geometry) ;
    
    romulus_render_stage stage = romulus_new_render_stage(camera, render_buffer, geometry) ;
    
    romulus_run_loop(scene, runframe, newargs( args(romulus_app,app), args(romulus_window,window), args(romulus_texture*, texture),
                                              args(romulus_scene, scene), args(romulus2d_rect, draw_rect), args(romulus_camera, camera),
                                              args(romulus_mesh, cube), args(romulus_shader, shader), args(romulus_render_buffer, render_buffer),
                                              args(romulus_geometry, geometry), args(romulus_material, material),
                                              args(romulus_entity, entity), args(romulus_render_stage, stage) ), shutdownapp, newargs(  ) ) ;
    
    return 0 ;
}
