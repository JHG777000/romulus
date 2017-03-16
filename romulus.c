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

static const char* vertex_shader =
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

static const char* fragment_shader =
"#version 330\n"
"in vec2      in_texcoord ;"
"out vec4     fragColor ;"
"uniform sampler2D romulus_texture_1 ;"
"void main () {"
"fragColor = (texture(romulus_texture_1, in_texcoord.st)) ;"
"}";

struct romulus_texture_s { unsigned int tid ; } ;

struct romulus_material_s { RKStore mesh_store ; romulus_attributes attributes ; int active ; unsigned int tunit_count ;
    
RKStore texture_store ; romulus_init_material init ; romulus_deinit_material deinit ; } ;

struct romulus_mesh_s { RKList entity_list ; int active ; romulus_attributes attributes ; int vao ; int ebo ; int vertex_count ; int index_count ; } ;

struct romulus_entity_s { romulus_mesh mesh ; RKList_node node ; int active ; RKMath_NewVector(pos, 3) ; RKMath_NewVector(rot, 3) ; float scale ; } ;

struct romulus_shader_s { romulus_display display ; romulus_attributes attributes ; unsigned int pid ; } ;

struct romulus_camera_s { RKMath_NewVector(view_matrix, 16) ; RKMath_NewVector(pos, 3) ; float pitch ; float yaw ; float roll ; } ;

struct romulus_geometry_s { romulus_display display ; romulus_attributes attributes ; RKStore material_store ; } ;

struct romulus_render_buffer_s { romulus_display display ; int fbuf ; } ;

struct romulus_display_s { IDKWindow display_window ; int width ; int height ; RKMath_NewVector(projection_matrix, 16) ; float fov ;
    
float aspect ; float nearZ ; float farZ ; int vsync ; } ;

struct romulus_render_stage_s { romulus_camera camera ; romulus_render_buffer render_buffer ; romulus_shader shader ; romulus_geometry geometry ; } ;


void romulus_bind_texture_unit( unsigned int tunit ) {
    
    glActiveTexture(GL_TEXTURE0 + tunit) ;
}

static void romulus_resize_display( IDKWindow window, int width, int height ) {
    
    romulus_display display = IDK_GetPtrFromWindow(window) ;
    
    romulus_set_active_display(display) ;
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0) ;
    
    glViewport(0, 0, width, height) ;
    
    display->width = width ;
    
    display->height = height ;
    
    #ifdef DEBUG
    
    IDKLog("romulus, gl viewport width: ", 0, 0) ;
    
    IDKLogInt(width, 1, 0) ;
    
    IDKLog("romulus, gl viewport height: ", 0, 0) ;
    
    IDKLogInt(height, 1, 0) ;
    
    #endif

}

romulus_display romulus_new_display( int window_width, int window_height, const char* window_title, float fov, float nearZ, float farZ, int vsync ) {
    
    int x, y = 0 ;
    
    romulus_display display = RKMem_NewMemOfType(struct romulus_display_s) ;
    
    display->display_window = IDK_NewWindow(window_width, window_height, window_title, romulus_resize_display) ;
    
    IDK_SetPtrFromWindow(display->display_window, display) ;
    
    IDK_GetRasterSize(display->display_window, &x, &y) ;
    
    romulus_resize_display(display->display_window, x, y) ;
    
    romulus_set_display_vsync(display, vsync) ;
    
    display->fov = fov ;
    
    display->aspect = ((float)window_width)/((float)window_height) ;
    
    display->nearZ = nearZ ;
    
    display->farZ = farZ ;
    
    romulus_new_projection_matrix(display->projection_matrix, display->fov, display->aspect, display->nearZ, display->farZ) ;
    
    return display ;
}

void romulus_destroy_display( romulus_display display ) {
    
    IDK_DestroyWindow(display->display_window) ;
    
    free(display) ;
}

void romulus_set_active_display( romulus_display display ) {
    
    IDK_SetWindowContextCurrent(display->display_window) ;
}

void romulus_set_display_vsync( romulus_display display, int vsync ) {
    
    if  ( vsync ) {
        
        display->vsync = 1 ;
        
        IDK_EnableVsync(display->display_window) ;
        
    } else {
        
       display->vsync = 0 ;
        
       IDK_DisableVsync(display->display_window) ;
    }
}

int romulus_is_display_vsync( romulus_display display ) {
    
    return display->vsync ;
}

void romulus_builtin_attributes_one( unsigned int shader_id ) {
    
    glBindAttribLocation(shader_id, 0, "position") ;
    
    glBindAttribLocation(shader_id, 1, "texcoord") ;
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
    
    shader->attributes(romulus_shader_program) ;
    
    glAttachShader(romulus_shader_program, vertex_shader) ;
    
    glAttachShader(romulus_shader_program, fragment_shader) ;
    
    glLinkProgram(romulus_shader_program) ;
    
    glGetProgramiv(romulus_shader_program, GL_LINK_STATUS, &status) ;
    
    if ( status == 0 ) {
        
        IDKLog("romulus, failed to link program", 1, 1) ;
        
        return 0 ;
    }
    
    glValidateProgram(romulus_shader_program) ;
    
    glGetProgramiv(romulus_shader_program, GL_VALIDATE_STATUS, &status) ;
    
    if ( status == 0 ) {
        
        IDKLog("romulus, failed to validate program", 1, 1) ;
        
        return 0 ;
    }
    
    glUseProgram(romulus_shader_program) ;
    
    return romulus_shader_program ;
}

romulus_shader romulus_new_shader( romulus_display display, romulus_attributes attributes ) {
    
    romulus_shader shader = RKMem_NewMemOfType(struct romulus_shader_s) ;
    
    shader->display = display ;
    
    shader->attributes = attributes ;
    
    shader->pid = romulus_build_shaders(shader, vertex_shader, fragment_shader) ;
    
    return shader ;
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
    }
    return 0;
    
}

static int romulus_make_vao( int* getebo, RKMVector vertexes, int v_size, RKMVector indexes, int i_size, RKMVector texture_coords ) {
    
    GLuint vao = 0 ;
    glGenVertexArrays(1, &vao) ;
    glBindVertexArray(vao) ;
    
    GLuint vbo = 0 ;
    glGenBuffers(1, &vbo) ;
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, v_size * 3 * sizeof(float), vertexes, GL_STATIC_DRAW) ;
    glEnableVertexAttribArray(0) ;
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo) ;
    
    GLsizei stride = romulus_get_gl_type_size(GL_FLOAT) ;
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*stride, NULL) ;
    
    GLuint tbo ;
    glGenBuffers(1, &tbo) ;
    glBindBuffer(GL_ARRAY_BUFFER, tbo) ;
    glBufferData(GL_ARRAY_BUFFER, v_size * 2 * sizeof(float), texture_coords, GL_STATIC_DRAW) ;
    glEnableVertexAttribArray(1) ;
    
    glBindBuffer(GL_ARRAY_BUFFER, tbo);
    
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_TRUE, 2*stride, NULL) ;
    
    if ( indexes != NULL ) {
    
      GLuint ebo ;
    
      glGenBuffers(1, &ebo) ;
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo) ;
    
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, i_size * sizeof(float), indexes, GL_STATIC_DRAW) ;
    
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

romulus_mesh romulus_new_mesh( romulus_attributes attributes, RKMVector vertexes, int v_size, RKMVector indexes, int i_size, RKMVector texture_coords ) {
    
    romulus_mesh mesh = RKMem_NewMemOfType(struct romulus_mesh_s) ;
    
    mesh->active = 1 ;
    
    mesh->attributes = attributes ;
    
    mesh->entity_list = RKList_NewList() ;
    
    if ( indexes != NULL ) {
        
        mesh->vao = romulus_make_vao(&mesh->ebo, vertexes, v_size, indexes, i_size, texture_coords) ;
        
        mesh->vertex_count = -1 ;
        
        mesh->index_count = i_size ;
        
    } else {
        
        mesh->vao = romulus_make_vao(NULL, vertexes, v_size, NULL, 0, texture_coords) ;
        
        mesh->vertex_count = v_size ;
        
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

void romulus_new_projection_matrix( RKMVector matrix, float fov, float aspect, float nearZ, float farZ ) {
    
    mtxLoadPerspective(matrix, fov, aspect, nearZ, farZ) ;
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

void romulus_render( romulus_render_stage stage ) {
    
    romulus_set_active_display(stage->render_buffer->display) ;
    
    glBindFramebuffer(GL_FRAMEBUFFER, stage->render_buffer->fbuf) ;
    
    glEnable(GL_DEPTH_TEST) ;
    
    glEnable(GL_CULL_FACE) ;
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) ;
    
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f) ;
    
    glUseProgram(stage->shader->pid) ;
    
    RKMath_NewVector(transform_matrix, 16) ;
    
    romulus_load_projection_matrix_to_shader(stage->render_buffer->display->projection_matrix, stage->shader) ;
    
    romulus_load_view_matrix_to_shader(stage->camera->view_matrix, stage->shader) ;
    
    RKList material_list = RKStore_GetList(stage->geometry->material_store) ;
    
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
         
            material->init(stage->shader) ;
            
            mesh_node = RKList_GetFirstNode( RKStore_GetList(material->mesh_store) ) ;
            
            while (mesh_node != NULL) {
                
                mesh = RKList_GetData(mesh_node) ;
                
                if ( mesh->active ) {
                    
                    entity_node = RKList_GetFirstNode( mesh->entity_list ) ;
                    
                    while (entity_node != NULL) {
                     
                        entity = RKList_GetData(entity_node) ;
                        
                        romulus_new_transform_matrix(transform_matrix, entity->pos, entity->rot, entity->scale) ;
                        
                        romulus_load_transform_matrix_to_shader(transform_matrix, stage->shader) ;
                        
                        if ( mesh->vertex_count != -1 ) {
                            
                            glBindVertexArray(mesh->vao) ;
                            
                            glDrawArrays(GL_TRIANGLES, 0, mesh->vertex_count) ;
                            
                        } else if ( mesh->index_count != -1 ) {
                            
                            glBindVertexArray(mesh->vao) ;
                            
                            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo) ;
                            
                            glDrawElements(GL_TRIANGLES, mesh->index_count, GL_FLOAT, 0) ;
                        }
                        
                        entity_node = RKList_GetNextNode(entity_node) ;
                    }
                }
                
                mesh_node = RKList_GetNextNode(mesh_node) ;
            }
            
            material->deinit(stage->shader) ;
        }
        
        material_node = RKList_GetNextNode(material_node) ;
    }
    
    glUseProgram(0) ;
}