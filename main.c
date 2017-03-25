
#include <stdio.h>
#include <stdlib.h>
#include "romulus.h"

static const char* vertex_shader1 =
"#version 330\n"
"in vec3  position ;"
"in vec2  texcoord ;"
"out vec2 out_texcoord ;"
"uniform mat4 transform_matrix ;"
"uniform mat4 projection_matrix ;"
"uniform mat4 view_matrix ;"
"void main () {"
"gl_Position = projection_matrix * view_matrix * transform_matrix * vec4(position,1.0) ;"
"out_texcoord = texcoord ;"
"}";

static const char* fragment_shader1 =
"#version 330\n"
"in vec2      out_texcoord ;"
"out vec4     fragColor ;"
"uniform sampler2D romulus_texture_1 ;"
"void main () {"
"fragColor = (texture(romulus_texture_1, out_texcoord.st)) ;"//vec4(out_texcoord,0,1)
"}";

romulus_scene scene = NULL ;

romulus_mesh cube = NULL ;

romulus_material material = NULL ;

romulus_shader shader = NULL ;

romulus_texture texture = NULL ;

romulus_entity entity = NULL ;

romulus_camera camera = NULL ;

romulus_geometry geometry = NULL ;

romulus_render_buffer render_buffer = NULL ;

romulus_render_stage stage = NULL ;

IDKDrawArea area = NULL ;

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

void material_init( romulus_shader shader ) {
    
    romulus_load_texture_to_shader(texture, shader) ;
}

void draw( IDKDrawArea area) {
    
    IDK_String(area, "HELLO WORLD!!!!", 0.2, IDK_GetMouseX(), IDK_GetMouseY(), 0, 1.0f, 0) ;
    
    IDK_DisplayFrameRateHP(area, 0.2, 0.1, 0.9, 1.0f, 1.0f, 1.0f) ;
}

void runframe( void ) {
    
    static RKMath_Vectorit(pos, 0, 0, 0) ;
    
    static RKMath_Vectorit(rot, 0, 0, 0) ;
    
    static float picth = 0 ;
    
    static float yaw = 0 ;
    
    static float roll = 0 ;
    
    int x, y = 0 ;
    
    if ( IDK_GetKey(idk_up_key) ) pos[2] += 0.01 ;
    
    if ( IDK_GetKey(idk_down_key) ) pos[2] -= 0.01 ;
    
    if ( IDK_GetKey(idk_right_key) ) pos[0] += 0.01 ;
    
    if ( IDK_GetKey(idk_left_key) ) pos[0] -= 0.01 ;
    
    if ( IDK_GetKey(idk_r_key) ) pos[1] += 0.1 ;
    
    if ( IDK_GetKey(idk_f_key) ) pos[1] -= 0.1 ;
    
    if ( IDK_GetKey(idk_w_key) ) rot[0] += 1 ;
    
    if ( IDK_GetKey(idk_s_key) ) rot[0] -= 1 ;
    
    if ( IDK_GetKey(idk_a_key) ) rot[1] += 1 ;
    
    if ( IDK_GetKey(idk_d_key) ) rot[1] -= 1 ;
    
    if ( IDK_GetKey(idk_q_key) ) rot[2] += 1 ;
    
    if ( IDK_GetKey(idk_e_key) ) rot[2] -= 1 ;
    
    romulus_camera_set_pos(camera, pos) ;
    
    romulus_camera_set_pitch(camera, picth) ;
    
    romulus_camera_set_yaw(camera, yaw) ;
    
    romulus_camera_set_roll(camera, roll) ;
    
    romulus_update_camera(camera) ;
    
    romulus_set_entity_rot(entity, rot) ;
    
    IDK_Draw(area, &x, &y) ;
        
    texture = romulus_new_texture(scene, "romulus_texture_1", 0, IDK_GetPixelScene(area)) ;
   
    romulus_render(stage) ;
  
    romulus_present(stage) ;
   
    romulus_destroy_texture(texture) ;
}

void shutdownapp( void ) {
    
    
}

int main(int argc, const char * argv[]) {
    
    RKMath_Vectorit(pos, 0, 0, -5) ;
    
    RKMath_Vectorit(rot, 0, 0, 0) ;
    
    RKMath_NewVector(projection_matrix, 16) ;
    
    romulus_window window = romulus_new_window(1024, 1024, "Hello World!!!!") ;
    
    scene = romulus_new_scene(window, romulus_true) ;
    
    area = IDK_NewDrawArea(draw, NULL, 1024, 1024, 0, 0, 0) ;
    
    camera = romulus_new_camera(pos, 0, 0, 0) ;
    
    cube = romulus_new_mesh(scene, attributes_constructor, newargs( args(float*,cube_vertices,cube_textcoords) ), attributes_binder, 8, cube_elements, 36) ;
   
    shader = romulus_new_shader(scene, vertex_shader1, fragment_shader1, attributes_binder) ;
    
    romulus_new_perspective_matrix(projection_matrix, 1920, 1080, 60, 0.1f, 1000.0f) ;
    
    render_buffer = romulus_new_render_buffer(scene, 1920, 1080, projection_matrix) ;
    
    geometry = romulus_new_geometry() ;
    
    material = romulus_new_material(shader, material_init) ;
    
    entity = romulus_new_entity(cube, pos, rot, 1) ;
    
    romulus_add_mesh_to_material(cube, material) ;
    
    romulus_add_material_to_geometry(material, geometry) ;
    
    stage = romulus_new_render_stage(camera, render_buffer, geometry) ;
    
    romulus_run_loop(scene, runframe, shutdownapp) ;
    
    return 0 ;
}
