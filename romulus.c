//
//  romulus.c
//  romulus
//
//  Created by Jacob Gordon on 10/21/16.
//  Copyright © 2016 Jacob Gordon. All rights reserved.
//

#include <stdlib.h>
#include "matrixUtil.h"
#include "romulus.h"

static const char* vertex_shader0 =
"#version 330\n"
"in vec3  position ;"
"in vec2  texcoord ;"
"out vec2 out_texcoord ;"
"void main () {"
"gl_Position = vec4(position,1.0) ;"
"out_texcoord = texcoord ;"
"}";

static const char* fragment_shader0 =
"#version 330\n"
"in vec2      out_texcoord ;"
"out vec4     fragColor ;"
"uniform sampler2D romulus_texture_0 ;"
"void main () {"
"fragColor = (texture(romulus_texture_0, out_texcoord.st)) ;"
"}";

struct romulus_texture_s { romulus_raw_texture raw_texture ; unsigned int tid ; unsigned int tunit ; RKString texture_name ; } ;

struct romulus_material_s { RKList mesh_list ; romulus_shader shader ; romulus_attributes_binder attributes ; int active ; romulus_init_material init ; } ;

struct romulus_mesh_s { RKList entity_list ; int active ; romulus_attributes_constructor attributes_constructor ; romulus_attributes_binder attributes ;
    
int vao ; int ebo ; int vertex_count ; int index_count ; } ;

struct romulus_entity_s { romulus_mesh mesh ; RKList_node node ; int active ; RKMath_NewVector(pos, 3) ; RKMath_NewVector(rot, 3) ; float scale ; } ;

struct romulus_shader_s { romulus_attributes_binder attributes ; unsigned int pid ; } ;

struct romulus_camera_s { RKMath_NewVector(view_matrix, 16) ; RKMath_NewVector(pos, 3) ; float pitch ; float yaw ; float roll ; } ;

struct romulus_geometry_s { RKList material_list ; } ;

struct romulus_render_buffer_s { romulus_scene scene ; int width ; int height ; RKMath_NewVector(projection_matrix, 16) ;

unsigned int fbuf ; unsigned int fbuf_tex ; unsigned int fbuf_depth_tex ; } ;

struct romulus_scene_s { romulus_window display_window ; int width ; int height ; romulus_bool cull ; romulus_bool depth ; romulus_bool vsync ; } ;

struct romulus_render_stage_s { romulus_camera camera ; romulus_render_buffer render_buffer ; romulus_geometry geometry ; } ;


void romulus_bind_texture_unit( unsigned int tunit ) {
    
    if ( tunit > 15 ) return ;
    
    glActiveTexture(GL_TEXTURE0 + tunit) ;
}

static void romulus_resize_display( IDKWindow window, int width, int height ) {
    
    romulus_scene scene = IDK_GetPtrFromWindow(window) ;
    
    romulus_set_active_scene(scene) ;
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0) ;
    
    glViewport(0, 0, width, height) ;
    
    scene->width = width ;
    
    scene->height = height ;
    
    IDKLog("romulus, gl viewport width: ", 0, 0) ;
    
    IDKLogInt(width, 1, 0) ;
    
    IDKLog("romulus, gl viewport height: ", 0, 0) ;
    
    IDKLogInt(height, 1, 0) ;

}

void romulus_report_error( const char* report_name ) {
    
    switch (glGetError()) {
            
        case GL_INVALID_ENUM:
            
            IDKLog("romulus reports error ", 0, 1) ;
            
            IDKLog("GL_INVALID_ENUM from, ", 0, 1) ;
            
            IDKLog(report_name, 1, 1) ;
            
            break;
            
        case GL_INVALID_VALUE:
            
            IDKLog("romulus reports error ", 0, 1) ;
            
            IDKLog("GL_INVALID_VALUE from, ", 0, 1) ;
            
            IDKLog(report_name, 1, 1) ;
            
            break;
            
        case GL_INVALID_OPERATION:
            
            IDKLog("romulus reports error ", 0, 1) ;
            
            IDKLog("GL_INVALID_OPERATION from, ", 0, 1) ;
            
            IDKLog(report_name, 1, 1) ;
            
            break;
            
            
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            
            IDKLog("romulus reports error ", 0, 1) ;
            
            IDKLog("GL_INVALID_FRAMEBUFFER_OPERATION from, ", 0, 1) ;
            
            IDKLog(report_name, 1, 1) ;
            
            break;
            
        case GL_OUT_OF_MEMORY:
            
            IDKLog("romulus reports error ", 0, 1) ;
            
            IDKLog("GL_OUT_OF_MEMORY from, ", 0, 1) ;
            
            IDKLog(report_name, 1, 1) ;
            
            break;
            
        default:
            break;
    }
}

romulus_window romulus_new_window( int win_width, int win_height, const char* win_title ) {
    
    return IDK_NewWindow(win_width, win_height, win_title, romulus_resize_display) ;
}

romulus_scene romulus_new_scene( romulus_window window, romulus_bool vsync ) {
    
    int x, y = 0 ;
    
    romulus_scene scene = RKMem_NewMemOfType(struct romulus_scene_s) ;
    
    scene->display_window = window ;
    
    IDK_SetPtrFromWindow(scene->display_window, scene) ;
    
    IDK_SetRasterResizeFunc(window, romulus_resize_display) ;
    
    IDK_GetRasterSize(scene->display_window, &x, &y) ;
    
    romulus_resize_display(scene->display_window, x, y) ;
    
    romulus_set_scene_vsync(scene, vsync) ;
    
    romulus_set_cull_back_face(scene, romulus_true) ;
    
    romulus_set_depth_test(scene, romulus_true) ;
    
    return scene ;
}

void romulus_destroy_scene( romulus_scene scene ) {
    
    IDK_DestroyWindow(scene->display_window) ;
    
    free(scene) ;
}

void romulus_set_active_scene( romulus_scene scene ) {
    
    IDK_SetWindowContextCurrent(scene->display_window) ;
}

void romulus_set_scene_vsync( romulus_scene scene, romulus_bool vsync ) {
    
    if  ( vsync ) {
        
        scene->vsync = romulus_true ;
        
        IDK_EnableVsync(scene->display_window) ;
        
    } else {
        
       scene->vsync = romulus_false ;
        
       IDK_DisableVsync(scene->display_window) ;
    }
}

romulus_bool romulus_is_scene_vsync( romulus_scene scene ) {
    
    return scene->vsync ;
}

void romulus_set_cull_back_face( romulus_scene scene, romulus_bool cull ) {
    
    scene->cull = cull ;
}

void romulus_set_depth_test( romulus_scene scene, romulus_bool depth ) {
    
    scene->depth = depth ;
}

romulus_window romulus_get_window_from_scene( romulus_scene scene ) {
    
    return scene->display_window ;
}

static int romulus_build_shaders( romulus_shader shader, const char* vertex_source, const char* fragment_source ) {
    
    int status = 0 ;
    
    int length_of_log = 0 ;
    
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER) ;
    
    glShaderSource(vertex_shader, 1, &vertex_source, NULL) ;
    
    glCompileShader(vertex_shader) ;
    
    glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &length_of_log) ;
    
    if ( length_of_log > 0 ) {
        
        GLchar *log = RKMem_CArray(length_of_log, GLchar) ;
        
        glGetShaderInfoLog(vertex_shader, length_of_log, &length_of_log, log) ;
        
        IDKLog("romulus, vertex shader compile log: ", 0, 1) ;
        
        IDKLog(log, 1, 1) ;
        
        free(log) ;
    }

    
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &status) ;
    
    if ( status == 0 ) {
        
        IDKLog("romulus, failed to compile vertex shader", 1, 1) ;
        
        return 0 ;
    }
    
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER) ;
    
    glShaderSource(fragment_shader, 1, &fragment_source, NULL) ;
    
    glCompileShader(fragment_shader) ;
    
    glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &length_of_log);
    
    if ( length_of_log > 0 ) {
        
        GLchar *log = RKMem_CArray(length_of_log, GLchar) ;
        
        glGetShaderInfoLog(fragment_shader, length_of_log, &length_of_log, log) ;
        
        IDKLog("romulus, frag shader compile log: ", 0, 1) ;
        
        IDKLog(log, 1, 1) ;
        
        free(log) ;
    }
    
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &status) ;
    
    if ( status == 0 ) {
        
        IDKLog("romulus, failed to compile fragment shader", 1, 1) ;
        
        return 0 ;
    }
    
    GLuint romulus_shader_program = glCreateProgram() ;

    shader->pid = romulus_shader_program ;
    
    shader->attributes(shader) ;
    
    glAttachShader(romulus_shader_program, vertex_shader) ;
    
    glAttachShader(romulus_shader_program, fragment_shader) ;
    
    glLinkProgram(romulus_shader_program) ;
    
    glGetProgramiv(romulus_shader_program, GL_INFO_LOG_LENGTH, &length_of_log);
    
    if ( length_of_log > 0 ) {
        
        GLchar *log = RKMem_CArray(length_of_log, GLchar) ;
        
        glGetProgramInfoLog(romulus_shader_program, length_of_log, &length_of_log, log) ;
        
        IDKLog("romulus, link program log: ", 0, 1) ;
        
        IDKLog(log, 1, 1) ;
        
        free(log) ;
    }
    
    glGetProgramiv(romulus_shader_program, GL_LINK_STATUS, &status) ;
    
    if ( status == 0 ) {
        
        IDKLog("romulus, failed to link program", 1, 1) ;
        
        return 0 ;
    }
    
    glValidateProgram(romulus_shader_program) ;
    
    glGetProgramiv(romulus_shader_program, GL_INFO_LOG_LENGTH, &length_of_log);
    
    if ( length_of_log > 0 ) {
        
        GLchar *log = RKMem_CArray(length_of_log, GLchar) ;
        
        glGetProgramInfoLog(romulus_shader_program, length_of_log, &length_of_log, log) ;
        
        IDKLog("romulus, validate program log: ", 0, 1) ;
        
        IDKLog(log, 1, 1) ;
        
        free(log) ;
    }
    
    glGetProgramiv(romulus_shader_program, GL_VALIDATE_STATUS, &status) ;
    
    if ( status == 0 ) {
        
        IDKLog("romulus, failed to validate program", 1, 1) ;
        
        return 0 ;
    }
    
    glUseProgram(romulus_shader_program) ;
    
    return romulus_shader_program ;
}

void romulus_attributes_zero( romulus_shader shader ) {
    
}

romulus_shader romulus_new_shader( romulus_scene scene, const char* vertex_shader, const char* fragment_shader, romulus_attributes_binder attributes ) {
    
    romulus_set_active_scene(scene) ;
    
    romulus_shader shader = RKMem_NewMemOfType(struct romulus_shader_s) ;
    
    shader->attributes = attributes ;
    
    shader->pid = romulus_build_shaders(shader,  vertex_shader,  fragment_shader) ;
    
    return shader ;
}

static void romulus_end_of_sahders( int program ) {
    
    if ( program == 0 ) return ;
    
    int i = 0 ;
    
    GLsizei num_of_shaders = 0 ;
    
    glGetProgramiv(program, GL_ATTACHED_SHADERS, &num_of_shaders) ;
    
    GLuint* shaders = RKMem_CArray(num_of_shaders, GLuint) ;
    
    glGetAttachedShaders(program, num_of_shaders, &num_of_shaders, shaders) ;
    
    while (i < num_of_shaders) {
        
        glDeleteShader(shaders[i]) ;
        
        i++ ;
    }
    
    free(shaders) ;
    
    glDeleteProgram(program) ;
    
    glUseProgram(0) ;
}

void romulus_destroy_shader( romulus_shader shader ) {
    
    romulus_end_of_sahders( shader->pid ) ;
    
    free(shader) ;
}

void romulus_bind_attribute_location_to_shader( const char* attribute_name, unsigned int location, romulus_shader shader ) {
    
    glBindAttribLocation(shader->pid, location, attribute_name) ;
}

static GLenum romulus_get_gl_type(romulus_data_type data_type) {
    
    switch (data_type) {
            
        case romulus_byte_type_id:
            
            return GL_UNSIGNED_BYTE ;
            
            break;
            
        case romulus_sbyte_type_id:
            
            return GL_BYTE ;
            
            break;
            
        case romulus_short_type_id:
            
            return GL_SHORT ;
            
            break;
            
            
        case romulus_ushort_type_id:
            
            return GL_UNSIGNED_SHORT ;
            
            break;
            
        case romulus_int_type_id:
            
            return GL_INT ;
            
            break;

        case romulus_uint_type_id:
            
            return GL_UNSIGNED_INT ;
            
            break;
            
        case romulus_float_type_id:
            
            return GL_FLOAT ;
            
            break;
            
        case romulus_double_type_id:
            
            return GL_DOUBLE ;
            
            break;
            
        default:
            break;
    }
    
}

static GLsizei romulus_get_gl_type_size(GLenum type){
    
    switch (type) {
        case GL_BYTE:
            return sizeof(GLbyte);
        case GL_UNSIGNED_BYTE:
            return sizeof(GLubyte);
        case GL_SHORT:
            return sizeof(GLshort);
        case GL_UNSIGNED_SHORT:
            return sizeof(GLushort);
        case GL_INT:
            return sizeof(GLint);
        case GL_UNSIGNED_INT:
            return sizeof(GLuint);
        case GL_FLOAT:
            return sizeof(GLfloat);
        case GL_DOUBLE:
            return sizeof(GLdouble);
    }
    return 0;
    
}

void romulus_construct_attribute( unsigned int index, void* data, romulus_data_type data_type, int num_of_data_per_element, int num_of_elements, romulus_bool is_normalized ) {
    
    GLuint vbo = 0 ;
    
    GLsizei stride = romulus_get_gl_type_size(romulus_get_gl_type(data_type)) ;
    
    glGenBuffers(1, &vbo) ;
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    
    glBufferData(GL_ARRAY_BUFFER, num_of_elements * num_of_data_per_element * stride, data, GL_STATIC_DRAW) ;
    
    glEnableVertexAttribArray(index) ;
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo) ;
    
    glVertexAttribPointer(index, num_of_data_per_element, romulus_get_gl_type(data_type), is_normalized, num_of_data_per_element*stride, NULL) ;
}


static int romulus_make_vao( int* getebo, romulus_attributes_constructor attributes_constructor, RKArgs attribute_args, int vertex_count, unsigned int* indexes, int index_count ) {
    
    GLuint vao = 0 ;
    glGenVertexArrays(1, &vao) ;
    glBindVertexArray(vao) ;
    
    attributes_constructor(vertex_count,attribute_args) ;
    
    if ( indexes != NULL ) {
    
      GLuint ebo ;
    
      glGenBuffers(1, &ebo) ;
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo) ;
    
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * sizeof(unsigned int), indexes, GL_STATIC_DRAW) ;
    
      *getebo = ebo ;
    
    }
    
    return vao ;
}


static void romulus_destroy_vao( int vao ) {
    
    GLuint i = 0 ;
    GLuint buf = 0 ;
    
    glBindVertexArray(vao) ;
    
    while ( i < 2 ) {
        
        glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, (GLint*)&buf) ;
        
        if (buf) glDeleteBuffers(1, &buf) ;
        
        i++ ;
    }
    
    
    glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, (GLint*)&buf) ;
    
    glDeleteBuffers(1, &buf) ;
    
    glDeleteVertexArrays(1, (GLuint*)&vao) ;
    
}

romulus_mesh romulus_new_mesh( romulus_scene scene, romulus_attributes_constructor attributes_constructor, RKArgs attribute_args, romulus_attributes_binder attributes,
                              
int vertex_count, unsigned int* indexes, int index_count ) {
    
    romulus_set_active_scene(scene) ;
    
    romulus_mesh mesh = RKMem_NewMemOfType(struct romulus_mesh_s) ;
    
    mesh->active = 1 ;
    
    mesh->attributes = attributes ;
    
    mesh->entity_list = RKList_NewList() ;
    
    if ( indexes != NULL ) {
        
        mesh->vao = romulus_make_vao(&mesh->ebo, attributes_constructor, attribute_args, vertex_count, indexes, index_count) ;
        
        mesh->vertex_count = -1 ;
        
        mesh->index_count = index_count ;
        
    } else {
        
        mesh->vao = romulus_make_vao(NULL, attributes_constructor, attribute_args, vertex_count, NULL, 0) ;
        
        mesh->vertex_count = vertex_count ;
        
        mesh->index_count = -1 ;
    }
    
    return mesh ;
    
}

static void romulus_destroy_entity_from_mesh( void* entity ) {
    
    free(entity) ;
}

void romulus_destroy_mesh( romulus_mesh mesh ) {
    
    glBindVertexArray(mesh->vao) ;
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo) ;
    
    romulus_destroy_vao(mesh->vao) ;
    
    RKList_IterateListWith(romulus_destroy_entity_from_mesh, mesh->entity_list) ;
    
    free(mesh) ;
}

void romulus_enable_attribute( unsigned int index ) {
    
    glEnableVertexAttribArray(index) ;
}

void romulus_enable_attribute_with_mesh( romulus_mesh mesh, unsigned int index ) {
    
    glBindVertexArray(mesh->vao) ;
    
    glEnableVertexAttribArray(index) ;
}

void romulus_disable_attribute_with_mesh( romulus_mesh mesh, unsigned int index ) {
    
    glBindVertexArray(mesh->vao) ;
    
    glDisableVertexAttribArray(index) ;
}

void romulus_load_transform_matrix_to_shader( RKMVector matrix, romulus_shader shader ) {
    
    glUseProgram(shader->pid) ;
    
    glUniformMatrix4fv(glGetUniformLocation(shader->pid, "transform_matrix"), 1, 0, matrix) ;
}

void romulus_load_projection_matrix_to_shader( RKMVector matrix, romulus_shader shader ) {
    
    glUseProgram(shader->pid) ;
    
    glUniformMatrix4fv(glGetUniformLocation(shader->pid, "projection_matrix"), 1, 0, matrix) ;
}

void romulus_load_view_matrix_to_shader( RKMVector matrix, romulus_shader shader ) {
    
    glUseProgram(shader->pid) ;
    
    glUniformMatrix4fv(glGetUniformLocation(shader->pid, "view_matrix"), 1, 0, matrix) ;
}

static GLuint romulus_load_texture_to_opengl(JHGInt* rawdata, int x, int y, GLenum format, GLenum type) {
    
    GLuint romulus_tex_name = 0 ;
    
    glGenTextures(1, &romulus_tex_name) ;
    
    glBindTexture(GL_TEXTURE_2D, romulus_tex_name) ;
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE) ;
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE) ;
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR) ;
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR) ;
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, x, y, 0, format, type, rawdata) ;
    
    return romulus_tex_name ;
}

static GLuint romulus_load_depth_texture_to_opengl( int x, int y ) {
    
    GLuint romulus_tex_name = 0 ;
    
    glGenTextures(1, &romulus_tex_name) ;
    
    glBindTexture(GL_TEXTURE_2D, romulus_tex_name) ;
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE) ;
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE) ;
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR) ;
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR) ;
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, x, y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0) ;
    
    return romulus_tex_name ;
}

static unsigned int romulus_load_raw_texture( romulus_raw_texture texture ) {
    
    int x, y = 0 ;
    
    return romulus_load_texture_to_opengl(JHG_DrawPixels(texture, &x, &y), x, y, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV) ;
}

romulus_texture romulus_new_texture( romulus_scene scene, const char* texture_name, unsigned int texture_unit, romulus_raw_texture raw_texture ) {
    
    romulus_set_active_scene(scene) ;
    
    romulus_texture texture = RKMem_NewMemOfType(struct romulus_texture_s) ;
    
    texture->raw_texture = raw_texture ;
    
    texture->texture_name = RKString_NewString(texture_name) ;
    
    texture->tid = romulus_load_raw_texture(raw_texture) ;
    
    texture->tunit = (texture_unit > 15) ? 15 : texture_unit ;
    
    return texture ;
}

void romulus_destroy_texture( romulus_texture texture ) {
    
    RKString_DestroyString(texture->texture_name) ;
    
    //JHGPixels_scenefree(texture->raw_texture) ;
    
    glDeleteTextures(1, &texture->tid) ;
    
    free(texture) ;
}

void romulus_load_texture_to_shader( romulus_texture texture, romulus_shader shader ) {
   
    romulus_bind_texture_unit(texture->tunit) ;
    
    glBindTexture(GL_TEXTURE_2D, texture->tid) ;
    
    glUniform1i(glGetUniformLocation(shader->pid, RKString_GetString(texture->texture_name) ), texture->tunit) ;
}

int romulus_get_texture_width( romulus_texture texture ) {
    
    return JHGPixels_GetX(texture->raw_texture) ;
}

int romulus_get_texture_height( romulus_texture texture ) {
    
    return JHGPixels_GetY(texture->raw_texture) ;
}

romulus_material romulus_new_material( romulus_shader shader, romulus_init_material init ) {
    
    romulus_material material = RKMem_NewMemOfType(struct romulus_material_s) ;
    
    material->active = 1 ;
    
    material->shader = shader ;
    
    material->attributes = shader->attributes ;
    
    material->init = init ;
    
    material->mesh_list = RKList_NewList() ;
    
    return material ;
 }

void romulus_destroy_material( romulus_material material ) {
    
    RKList_DeleteList(material->mesh_list) ;
    
    free(material) ;
}

int romulus_is_material_active( romulus_material material ) {
    
    return material->active ;
}

void romulus_set_material_active( romulus_material material, int is_active ) {
    
    material->active = (is_active) ? 1 : 0 ;
}

void romulus_add_mesh_to_material( romulus_mesh mesh, romulus_material material ) {
    
    if ( mesh->attributes != material->attributes ) return ;
    
    RKList_AddToList(material->mesh_list, mesh) ;
}

void romulus_new_perspective_matrix( RKMVector matrix, int width, int height, float fov, float nearZ, float farZ ) {
    
    float aspect = ((float)width)/((float)height) ;
    
    mtxLoadPerspective(matrix, fov, aspect, nearZ, farZ) ;
}

void romulus_new_ographic_matrix( RKMVector matrix, float left, float right, float bottom, float top, float nearZ, float farZ ) {
    
    mtxLoadOrthographic(matrix, left, right, bottom, top, nearZ, farZ) ;
}

void romulus_new_transform_matrix( RKMVector matrix, RKMVector pos, RKMVector rot, float scale ) {
    
    mtxLoadIdentity(matrix) ;
    
    mtxTranslateApply(matrix, pos[RKM_X], pos[RKM_Y], pos[RKM_Z]) ;
    
    mtxRotateApply(matrix, rot[RKM_X], 1, 0, 0) ;
    
    mtxRotateApply(matrix, rot[RKM_Y], 0, 1, 0) ;
    
    mtxRotateApply(matrix, rot[RKM_Z], 0, 0, 1) ;
    
    mtxScaleApply(matrix, scale, scale, scale) ;
}

void romulus_new_view_matrix( RKMVector matrix, RKMVector pos, float pitch, float yaw, float roll ) {
    
     mtxLoadIdentity(matrix) ;
    
     mtxRotateApply(matrix, pitch, 1, 0, 0) ;
    
     mtxRotateApply(matrix, yaw, 0, 1, 0) ;
    
     mtxRotateApply(matrix, roll, 0, 0, 1) ;
    
     RKMath_Neg(pos, 3) ;
    
     mtxTranslateApply(matrix, pos[RKM_X], pos[RKM_Y], pos[RKM_Z]) ;
}

romulus_camera romulus_new_camera( RKMVector pos, float pitch, float yaw, float roll ) {
    
    romulus_camera camera = RKMem_NewMemOfType(struct romulus_camera_s) ;
    
    camera->pitch = pitch ;
    
    camera->yaw = yaw ;
    
    camera->roll = roll ;
    
    RKMath_Equal(camera->pos, pos, 3) ;
    
    romulus_new_view_matrix(camera->view_matrix, camera->pos, camera->pitch, camera->yaw, camera->roll) ;
    
    return camera ;
}

void romulus_destroy_camera( romulus_camera camera ) {
    
    free(camera) ;
}

void romulus_camera_get_pos( romulus_camera camera, RKMVector pos ) {
    
    RKMath_Equal(pos, camera->pos, 3) ;
}

void romulus_camera_set_pos( romulus_camera camera, RKMVector pos ) {
    
    RKMath_Equal(camera->pos, pos, 3) ;
}

void romulus_camera_set_pitch( romulus_camera camera, float pitch ) {
    
    camera->pitch = pitch ;
}

void romulus_camera_set_yaw( romulus_camera camera, float yaw ) {
    
    camera->yaw = yaw ;
}

void romulus_camera_set_roll( romulus_camera camera, float roll ) {
    
    camera->roll = roll ;
}

void romulus_update_camera( romulus_camera camera ) {
    
    romulus_new_view_matrix(camera->view_matrix, camera->pos, camera->pitch, camera->yaw, camera->roll) ;
}

romulus_entity romulus_new_entity( romulus_mesh mesh, RKMVector pos, RKMVector rot, float scale ) {
    
    romulus_entity entity = RKMem_NewMemOfType(struct romulus_entity_s) ;
    
    entity->node = RKList_AddToList(mesh->entity_list, entity) ;
    
    RKMath_Equal(entity->pos, pos, 3) ;
    
    RKMath_Equal(entity->rot, rot, 3) ;
    
    entity->scale = scale ;
    
    entity->mesh = mesh ;
    
    entity->active = 1 ;
    
    return entity ;
}

void romulus_destroy_entity( romulus_entity entity ) {
    
    RKList_DeleteNode(entity->mesh->entity_list, entity->node) ;
    
    free(entity) ;
}

void romulus_set_entity_pos( romulus_entity entity, RKMVector pos ) {
    
    RKMath_Equal(entity->pos, pos, 3) ;
}

void romulus_get_entity_pos( romulus_entity entity, RKMVector pos ) {
    
    RKMath_Equal(pos, entity->pos, 3) ;
}

void romulus_set_entity_rot( romulus_entity entity, RKMVector rot ) {
    
    RKMath_Equal(entity->rot, rot, 3) ;
}

void romulus_get_entity_rot( romulus_entity entity, RKMVector rot ) {
    
    RKMath_Equal(rot, entity->rot, 3) ;
}

void romulus_set_entity_scale( romulus_entity entity, float scale ) {
    
    entity->scale = scale ;
}

float romulus_get_entity_scale( romulus_entity entity ) {
    
    return entity->scale ;
}

void romulus_add_to_entity_pos( romulus_entity entity, RKMVector vec ) {
    
    RKMath_Add(entity->pos, entity->pos, vec, 3) ;
}

void romulus_add_to_entity_rot( romulus_entity entity, RKMVector vec ) {
    
    RKMath_Add(entity->rot, entity->rot, vec, 3) ;
}

romulus_geometry romulus_new_geometry( void ) {
    
    romulus_geometry geometry = RKMem_NewMemOfType(struct romulus_geometry_s) ;
    
    geometry->material_list = RKList_NewList() ;
    
    return geometry ;
}

void romulus_destroy_geometry( romulus_geometry geometry ) {
    
    RKList_DeleteList(geometry->material_list) ;
    
    free(geometry->material_list) ;
}

void romulus_add_material_to_geometry( romulus_material material, romulus_geometry geometry ) {
    
    RKList_AddToList(geometry->material_list, material) ;
}

static void romulus_new_framebuffer( romulus_render_buffer render_buffer ) {
    
    GLuint fbuf ;
    
    GLuint fbuf_tex ;
    
    GLuint fbuf_depth_tex ;
    
    glGenFramebuffers(1, &fbuf) ;
    
    glBindFramebuffer(GL_FRAMEBUFFER, fbuf) ;
    
    glViewport(0, 0, render_buffer->width, render_buffer->height) ;
    
    fbuf_tex = romulus_load_texture_to_opengl(0, render_buffer->width, render_buffer->height, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV) ;
    
    fbuf_depth_tex = romulus_load_depth_texture_to_opengl(render_buffer->width, render_buffer->height) ;
    
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, fbuf_tex, 0) ;
    
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, fbuf_depth_tex, 0) ;
    
    GLenum drawbuffers[1] = {GL_COLOR_ATTACHMENT0} ;
   
    glDrawBuffers(1, drawbuffers) ;
     
    render_buffer->fbuf = fbuf ;
    
    render_buffer->fbuf_tex = fbuf_tex ;
}

romulus_render_buffer romulus_new_render_buffer( romulus_scene scene, int width, int height, RKMVector projection_matrix ) {
    
    romulus_render_buffer render_buffer = RKMem_NewMemOfType(struct romulus_render_buffer_s) ;
    
    render_buffer->scene = scene ;
    
    render_buffer->width = width ;
    
    render_buffer->height = height ;
    
    romulus_new_framebuffer(render_buffer) ;
    
    RKMath_Equal(render_buffer->projection_matrix, projection_matrix, 16) ;
    
    return render_buffer ;
}

void romulus_destroy_render_buffer( romulus_render_buffer render_buffer ) {
    
    glDeleteTextures(1, &render_buffer->fbuf_tex) ;
    
    glDeleteFramebuffers(1, &render_buffer->fbuf) ;
    
    free(render_buffer) ;
}

void romulus_load_render_buffer_as_texture_to_shader( romulus_render_buffer render_buffer, const char* texture_name, unsigned int tunit, romulus_shader shader ) {
    
    if ( tunit > 15 ) return ;
    
    romulus_bind_texture_unit(tunit) ;
    
    glBindTexture(GL_TEXTURE_2D, render_buffer->fbuf_tex) ;
    
    glUniform1i(glGetUniformLocation(shader->pid, texture_name ), tunit) ;
}

int romulus_get_render_buffer_width( romulus_render_buffer render_buffer ) {
    
    return render_buffer->width ;
}

int romulus_get_render_buffer_height( romulus_render_buffer render_buffer ) {
    
    return render_buffer->height ;
}

void romulus_set_render_buffer_projection_matrix( romulus_render_buffer render_buffer, RKMVector projection_matrix ) {
    
    RKMath_Equal(render_buffer->projection_matrix, projection_matrix, 16) ;
}

romulus_render_stage romulus_new_render_stage( romulus_camera camera, romulus_render_buffer render_buffer, romulus_geometry geometry ) {
    
    romulus_render_stage stage = RKMem_NewMemOfType(struct romulus_render_stage_s) ;
    
    stage->camera = camera ;
    
    stage->render_buffer = render_buffer ;
    
    stage->geometry = geometry ;
    
    return stage ;
}

void romulus_destroy_render_stage( romulus_render_stage stage ) {
    
    free(stage) ;
}

void romulus_render( romulus_render_stage stage ) {
    
    romulus_set_active_scene(stage->render_buffer->scene) ;
    
    glBindFramebuffer(GL_FRAMEBUFFER, stage->render_buffer->fbuf) ;
    
    if (stage->render_buffer->scene->depth)  {
        
        glEnable(GL_DEPTH_TEST) ;
        
    } else {
        
        glDisable(GL_DEPTH_TEST) ;
    }
    
    if (stage->render_buffer->scene->cull)  {
        
        glEnable(GL_CULL_FACE) ;
        
    } else {
        
        glDisable(GL_CULL_FACE) ;
    }
    
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f) ;
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) ;
    
    glUseProgram(0) ;
    
    glViewport(0, 0, stage->render_buffer->width, stage->render_buffer->height) ;
    
    RKMath_NewVector(transform_matrix, 16) ;
    
    RKList material_list = stage->geometry->material_list ;
    
    RKList_node material_node = NULL ;
    
    RKList_node mesh_node = NULL ;
    
    RKList_node entity_node = NULL ;
    
    romulus_material material = NULL ;
    
    romulus_mesh mesh = NULL ;
    
    romulus_entity entity = NULL ;
    
    material_node = RKList_GetFirstNode(material_list) ;
    
    while (material_node != NULL) {
        
        material = RKList_GetData(material_node) ;
        
        if ( material->active ) {
         
            glUseProgram(material->shader->pid) ;
            
            romulus_load_projection_matrix_to_shader(stage->render_buffer->projection_matrix, material->shader) ;
            
            romulus_load_view_matrix_to_shader(stage->camera->view_matrix, material->shader) ;
            
            material->init(material->shader) ;
            
            mesh_node = RKList_GetFirstNode( material->mesh_list) ;
            
            while (mesh_node != NULL) {
                
                mesh = RKList_GetData(mesh_node) ;
                
                if ( mesh->active ) {
                    
                    entity_node = RKList_GetFirstNode( mesh->entity_list ) ;
                    
                    while (entity_node != NULL) {
                     
                        entity = RKList_GetData(entity_node) ;
                        
                        romulus_new_transform_matrix(transform_matrix, entity->pos, entity->rot, entity->scale) ;
                        
                        romulus_load_transform_matrix_to_shader(transform_matrix, material->shader) ;
                        
                        if ( mesh->vertex_count != -1 ) {
                            
                            glBindVertexArray(mesh->vao) ;
                            
                            glDrawArrays(GL_TRIANGLES, 0, mesh->vertex_count) ;
                            
                        } else if ( mesh->index_count != -1 ) {
                            
                            glBindVertexArray(mesh->vao) ;
                            
                            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo) ;
                            
                            glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_INT, 0) ;
                        }
                        
                        entity_node = RKList_GetNextNode(entity_node) ;
                    }
                }
                
                mesh_node = RKList_GetNextNode(mesh_node) ;
            }
        }
        
        material_node = RKList_GetNextNode(material_node) ;
    }
    
    romulus_report_error("romulus_render") ;
    
    glUseProgram(0) ;
}

static void romulus_attributes_base( romulus_shader shader ) {
    
    romulus_bind_attribute_location_to_shader("position", 0, shader) ;
    
    romulus_bind_attribute_location_to_shader("texcoord", 1, shader) ;
}

static void romulus_attributes_constructor_base( int vertex_count, RKArgs args ) {
    
    RKArgs_UseArgs(args) ;
    
    romulus_construct_attribute(0, RKArgs_GetNextArg(args, GLfloat*), romulus_float_type_id, 3, vertex_count, romulus_false) ;
    
    romulus_construct_attribute(1, RKArgs_GetNextArg(args, GLfloat*), romulus_float_type_id, 2, vertex_count, romulus_true) ;
}

void romulus_present( romulus_render_stage stage ) {
    
    static int init = 0 ;
    
    static GLfloat quad_pos_array[] = {
        -1.0f, 1.0f, 0.0f,
        1.0f,  1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        -1.0f,-1.0f, 0.0f
    };
    
    static GLfloat quad_texcoord_array[] = {
        0.0f,  1.0f,
        1.0f,  1.0f,
        1.0f,  0.0f,
        0.0f,  0.0f
    };
    
    static GLuint quad_elementArray_array[] = {
        0, 2, 1,
        0, 3, 2
    };
    
    static romulus_mesh mesh = NULL ;
    
    static romulus_shader shader = NULL ;
    
    romulus_set_active_scene(stage->render_buffer->scene) ;
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0) ;
    
    glEnable(GL_DEPTH_TEST) ;
    
    glEnable(GL_CULL_FACE) ;
    
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f) ;
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) ;
    
    if ( !init ) {
        
        mesh = romulus_new_mesh(stage->render_buffer->scene, romulus_attributes_constructor_base, newargs( args(GLfloat*,quad_pos_array,quad_texcoord_array) ),
                                                                                                                       romulus_attributes_base, 4, quad_elementArray_array, 6) ;
        shader = romulus_new_shader(stage->render_buffer->scene, vertex_shader0, fragment_shader0, romulus_attributes_base) ;
        
        init++ ;
    }
    
    glUseProgram(shader->pid) ;
    
    glViewport(0, 0, stage->render_buffer->scene->width, stage->render_buffer->scene->height) ;
    
    romulus_load_render_buffer_as_texture_to_shader(stage->render_buffer, "romulus_texture_0", 0, shader) ;
    
    glBindVertexArray(mesh->vao) ;
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo) ;
    
    glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_INT, 0) ;

    romulus_report_error("romulus_present") ;
}

void romulus_run_loop( romulus_scene scene, romulus_run_loop_func_type run_loop_func, romulus_run_quit_loop_func_type run_quit_loop_func ) {
    
    IDK_WindowRunLoop(scene->display_window, run_loop_func, run_quit_loop_func) ;
}