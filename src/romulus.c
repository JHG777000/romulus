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

#include <stdlib.h>
#include <romulus/matrixUtil.h>
#include <romulus/romulus.h>

static const char* vertex_shader0 =
"#version 330\n"
"in vec3  position ;"
"in vec2  texcoord ;"
"out vec2 out_texcoord ;"
"void main() {"
"gl_Position = vec4(position,1.0) ;"
"out_texcoord = texcoord ;"
"}";

static const char* fragment_shader0 =
"#version 330\n"
"in vec2      out_texcoord ;"
"out vec4     fragColor ;"
"vec2 texcoord ;"
"uniform sampler2D romulus_texture_0 ;"
"void main() {"
//"texcoord = vec2(out_texcoord.s, 1.0 - out_texcoord.t) ;"
"fragColor = (texture(romulus_texture_0, out_texcoord.st)) ;"
"}";

struct romulus_app_s { IDKApp App ; romulus_bool logging ; } ;

struct romulus_window_s { IDKWindow idk_window ; romulus_app app ; romulus_scene scene ; } ;

struct romulus_scene_s { romulus_window display_window ; romulus_bool logging ; int width ; int height ; romulus_bool vsync ; } ;

struct romulus_texture_s { romulus2d_texture raw_texture ; unsigned int tid ; unsigned int tunit ; RKString texture_name ; } ;

struct romulus_material_s { RKList mesh_list ; RKArgs material_args ; romulus_shader shader ; romulus_attributes_binder attributes ; int active ; romulus_init_material init ; } ;

struct romulus_mesh_s { RKList entity_list ; int active ; romulus_attributes_constructor attributes_constructor ; romulus_attributes_binder attributes ;
    
int vao ; int ebo ; int vertex_count ; int index_count ; } ;

struct romulus_entity_s { romulus_mesh mesh ; RKList_node node ; int active ; RKMath_NewVector(pos, 3) ; RKMath_NewVector(rot, 3) ; float scale ; } ;

struct romulus_shader_s { romulus_attributes_binder attributes ; unsigned int pid ; } ;

struct romulus_camera_s { RKMath_NewVector(view_matrix, 16) ; RKMath_NewVector(pos, 3) ; float pitch ; float yaw ; float roll ; } ;

struct romulus_geometry_s { RKList material_list ; } ;

struct romulus_render_buffer_s { romulus_scene scene ; int width ; int height ; RKMath_NewVector(projection_matrix, 16) ;

unsigned int fbuf ; unsigned int fbuf_tex ; unsigned int fbuf_depth_tex ; } ;

struct romulus_render_stage_s { romulus_camera camera ; romulus_render_buffer render_buffer ; romulus_geometry geometry ;
    
romulus_bool cull ; romulus_bool depth ; romulus_bool alpha ; } ;

int romulus_check_extension( const char* extension ) {
    
    GLint n, i = 0 ;
    
    glGetIntegerv(GL_NUM_EXTENSIONS, &n) ;
    
    while ( i < n ) {
        
     if ( !strcmp(extension, (char*)glGetStringi(GL_EXTENSIONS, i)) ) return 1 ;
        
        i++ ;
    }
    
    return 0 ;
}

static void romulus_bind_texture_unit( unsigned int tunit ) {
    
    if ( tunit > 15 ) return ;
    
    glActiveTexture(GL_TEXTURE0 + tunit) ;
}

static void romulus_resize_display( IDKWindow idk_window, int width, int height ) {
    
    romulus_window window = IDK_GetPtrFromWindow(idk_window) ;
    
    IDKApp App = romulus_get_idk_app(window->app)  ;
    
    romulus_scene scene = window->scene ;
    
    romulus_set_active_scene(scene) ;
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0) ;
    
    glViewport(0, 0, width, height) ;
    
    scene->width = width ;
    
    scene->height = height ;
    
    if (!scene->logging) return ;
    
    IDKLog(App, "romulus, gl viewport width: ", 0, 0) ;
    
    IDKLogInt(App, width, 1, 0) ;
    
    IDKLog(App, "romulus, gl viewport height: ", 0, 0) ;
    
    IDKLogInt(App, height, 1, 0) ;

}

void romulus_report_error( romulus_scene scene, const char* report_name ) {
    
    IDKApp App = romulus_get_idk_app(scene->display_window->app)  ;
    
    if ( !scene->logging ) return ;
    
    switch (glGetError()) {
            
        case GL_INVALID_ENUM:
            
            IDKLog(App, "romulus reports error ", 0, 1) ;
            
            IDKLog(App, "GL_INVALID_ENUM from, ", 0, 1) ;
            
            IDKLog(App, report_name, 1, 1) ;
            
            break;
            
        case GL_INVALID_VALUE:
            
            IDKLog(App, "romulus reports error ", 0, 1) ;
            
            IDKLog(App, "GL_INVALID_VALUE from, ", 0, 1) ;
            
            IDKLog(App, report_name, 1, 1) ;
            
            break;
            
        case GL_INVALID_OPERATION:
            
            IDKLog(App, "romulus reports error ", 0, 1) ;
            
            IDKLog(App, "GL_INVALID_OPERATION from, ", 0, 1) ;
            
            IDKLog(App, report_name, 1, 1) ;
            
            break;
            
            
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            
            IDKLog(App, "romulus reports error ", 0, 1) ;
            
            IDKLog(App, "GL_INVALID_FRAMEBUFFER_OPERATION from, ", 0, 1) ;
            
            IDKLog(App, report_name, 1, 1) ;
            
            break;
            
        case GL_OUT_OF_MEMORY:
            
             IDKLog(App, "romulus reports error ", 0, 1) ;
            
             IDKLog(App, "GL_OUT_OF_MEMORY from, ", 0, 1) ;
            
             IDKLog(App, report_name, 1, 1) ;
            
            break;
            
        default:
            break;
    }
}

romulus_app romulus_new_app( RKString app_name, float version, romulus_bool logging ) {
    
    romulus_app app = RKMem_NewMemOfType(struct romulus_app_s) ;
    
    app->logging = logging ;
    
    app->App = IDK_NewApp(app_name, version, NULL) ;
    
    return app ;
}

void romulus_destroy_app( romulus_app app ) {
    
    IDK_DestroyApp(app->App) ;
    
    free(app) ;
}

IDKApp romulus_get_idk_app( romulus_app app ) {
    
    return app->App ;
}

static romulus_scene romulus_new_scene( romulus_window window, romulus_bool vsync, romulus_bool logging ) ;

romulus_window romulus_new_window( romulus_app app, int win_width, int win_height, const char* win_title, romulus_bool vsync, romulus_bool scene_logging ) {
    
    romulus_window window = RKMem_NewMemOfType(struct romulus_window_s) ;
    
    window->app = app ;
    
    window->idk_window = IDK_NewWindow(app->App, win_width, win_height, win_title, romulus_resize_display) ;
    
    IDK_SetPtrFromWindow(window->idk_window, window) ;
    
    romulus_new_scene(window, vsync, scene_logging) ;
    
    return window ;
}

static void romulus_destroy_window( romulus_window window ) {
    
    IDK_DestroyWindow(window->idk_window) ;
    
    free(window) ;
}

void romulus_enter_fullscreen( romulus_window window ) {
    
    IDK_EnterFullscreen(window->idk_window) ;
}

void romulus_exit_fullscreen( romulus_window window ) {
    
    IDK_ExitFullscreen(window->idk_window) ;
}

romulus_app romulus_get_app_from_window( romulus_window window ) {
    
    return window->app ;
}

romulus_scene romulus_get_scene_from_window( romulus_window window ) {
    
    return window->scene ;
}

IDKWindow romulus_get_idk_window( romulus_window window ) {
    
    return window->idk_window ;
}

static romulus_scene romulus_new_scene( romulus_window window, romulus_bool vsync, romulus_bool logging ) {
    
    int x, y = 0;
    
    romulus_scene scene = RKMem_NewMemOfType(struct romulus_scene_s) ;
    
    scene->display_window = window ;
    
    scene->logging = logging ;
    
    if ( scene->display_window->app->logging  ) romulus_enable_scene_logging(scene, logging) ;
    
    IDK_SetRasterResizeFunc(window->idk_window, romulus_resize_display) ;
    
    IDK_GetRasterSize(window->idk_window, &x, &y) ;
    
    window->scene = scene ;
    
    romulus_set_scene_vsync(scene, vsync) ;
    
    if ( scene->logging ) {
        
        IDKLog(window->app->App,"romulus rendering libary, version ", 0, 0) ;
        
        IDKLogFloat(window->app->App,0,1,0) ;
        
        IDKLog(window->app->App,"romulus built on top of: ", 0, 0) ;
        
        IDK_LogVersion(window->app->App) ;
        
        IDKLog(window->app->App,"romulus, opengl renderer: ", 0, 0) ;
    
        IDKLog(window->app->App,(const char*)glGetString(GL_RENDERER), 1, 0) ;
    
        IDKLog(window->app->App,"romulus, opengl version: ", 0, 0) ;
    
        IDKLog(window->app->App,(const char*)glGetString(GL_VERSION), 1, 0) ;
    }
    
     romulus_resize_display(window->idk_window, x, y) ;
    
    return scene ;
}

void romulus_destroy_scene( romulus_scene scene ) {
    
    romulus_destroy_window(scene->display_window) ;
    
    free(scene) ;
}

void romulus_set_scene_vsync( romulus_scene scene, romulus_bool vsync ) {
    
    if  ( vsync ) {
        
        scene->vsync = romulus_true ;
        
        IDK_EnableVsync(scene->display_window->idk_window) ;
        
    } else {
        
       scene->vsync = romulus_false ;
        
        IDK_DisableVsync(scene->display_window->idk_window) ;
    }
}

romulus_bool romulus_is_scene_vsync( romulus_scene scene ) {
    
    return scene->vsync ;
}

romulus_window romulus_get_window_from_scene( romulus_scene scene ) {
    
    return scene->display_window ;
}

void romulus_set_active_scene( romulus_scene scene ) {
    
    IDK_SetWindowContextCurrent(scene->display_window->idk_window) ;
}

void romulus_enable_scene_logging( romulus_scene scene, romulus_bool logging ) {
    
     scene->logging = ( scene->display_window->app->logging && logging ) ? romulus_true : romulus_false ;
}

static int romulus_build_shaders( romulus_app app, romulus_shader shader, const char* vertex_source, const char* fragment_source ) {
    
    int status = 0 ;
    
    int length_of_log = 0 ;
    
    IDKApp App = app->App ;
    
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER) ;
    
    glShaderSource(vertex_shader, 1, &vertex_source, NULL) ;
    
    glCompileShader(vertex_shader) ;
    
    glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &length_of_log) ;
    
    if ( length_of_log > 0 ) {
        
        GLchar *log = RKMem_CArray(length_of_log, GLchar) ;
        
        glGetShaderInfoLog(vertex_shader, length_of_log, &length_of_log, log) ;
        
        IDKLog(App, "romulus, vertex shader compile log: ", 0, 1) ;
        
        IDKLog(App, log, 1, 1) ;
        
        free(log) ;
    }

    
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &status) ;
    
    if ( status == 0 ) {
        
        IDKLog(App, "romulus, failed to compile vertex shader", 1, 1) ;
        
        return 0 ;
    }
    
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER) ;
    
    glShaderSource(fragment_shader, 1, &fragment_source, NULL) ;
    
    glCompileShader(fragment_shader) ;
    
    glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &length_of_log);
    
    if ( length_of_log > 0 ) {
        
        GLchar *log = RKMem_CArray(length_of_log, GLchar) ;
        
        glGetShaderInfoLog(fragment_shader, length_of_log, &length_of_log, log) ;
        
        IDKLog(App, "romulus, frag shader compile log: ", 0, 1) ;
        
        IDKLog(App, log, 1, 1) ;
        
        free(log) ;
    }
    
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &status) ;
    
    if ( status == 0 ) {
        
        IDKLog(App, "romulus, failed to compile fragment shader", 1, 1) ;
        
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
        
        IDKLog(App, "romulus, link program log: ", 0, 1) ;
        
        IDKLog(App, log, 1, 1) ;
        
        free(log) ;
    }
    
    glGetProgramiv(romulus_shader_program, GL_LINK_STATUS, &status) ;
    
    if ( status == 0 ) {
        
        IDKLog(App, "romulus, failed to link program", 1, 1) ;
        
        return 0 ;
    }
    
    glValidateProgram(romulus_shader_program) ;
    
    glGetProgramiv(romulus_shader_program, GL_INFO_LOG_LENGTH, &length_of_log);
    
    if ( length_of_log > 0 ) {
        
        GLchar *log = RKMem_CArray(length_of_log, GLchar) ;
        
        glGetProgramInfoLog(romulus_shader_program, length_of_log, &length_of_log, log) ;
        
        IDKLog(App, "romulus, validate program log: ", 0, 1) ;
        
        IDKLog(App, log, 1, 1) ;
        
        free(log) ;
    }
    
    glGetProgramiv(romulus_shader_program, GL_VALIDATE_STATUS, &status) ;
    
    if ( status == 0 ) {
        
        IDKLog(App, "romulus, failed to validate program", 1, 1) ;
        
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
    
    shader->pid = romulus_build_shaders(scene->display_window->app, shader,  vertex_shader,  fragment_shader) ;
    
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

static GLuint romulus_load_texture_to_opengl(void* rawdata, int x, int y, GLenum iformat, GLenum format, GLenum type, float quality) {
    
    GLuint romulus_tex_name = 0 ;
    
    int anisotropic = 0 ;
    
    float max = 0 ;
    
    float a = 0 ;
    
    RKMath_Clamp(&quality, 0, 3, 1) ;
    
    if ( quality > 2 ) {
        
        quality -= 2 ;
        
        anisotropic = 1 ;
        
        a = quality ;
    }
    
    glGenTextures(1, &romulus_tex_name) ;
    
    glBindTexture(GL_TEXTURE_2D, romulus_tex_name) ;
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE) ;
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE) ;
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR) ;
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR) ;
    
    glTexImage2D(GL_TEXTURE_2D, 0, iformat, x, y, 0, format, type, rawdata) ;
    
    if ( quality > 0 ) {
    
      if ( !anisotropic ) {
        
        if ( quality <= 1 ) quality *= -1 ;
        
        if ( quality > 1 ) quality -= 1 ;
          
      } else {
          
        quality = 0 ;
          
      }
        
      glGenerateMipmap(GL_TEXTURE_2D) ;
    
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR) ;
    
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, quality) ;
        
        if ( (anisotropic) && (romulus_check_extension("GL_EXT_texture_filter_anisotropic")) ) {
            
          #ifndef GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
             #define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x1100 //Don't Care
          #endif
            
          #ifndef GL_TEXTURE_MAX_ANISOTROPY_EXT
            #define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x1100 //Don't Care
          #endif
            
          glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max) ;
            
          glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, (a * max)) ;
            
        }
    }
    
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

static unsigned int romulus_load_raw_texture( romulus2d_texture texture, float quality ) {
    
    int x = romulus2d_texture_get_width(texture) ;
    
    int y = romulus2d_texture_get_height(texture) ;
    
    switch ( romulus2d_get_texture_format(texture) ) {
        
        case romulus2d_rgba8_texture_format:
            
           return romulus_load_texture_to_opengl(romulus2d_get_raw_texture_data(texture), x, y, GL_RGBA8, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, quality) ;
            
        break;
            
        case romulus2d_rgba32_texture_format:
            
           return romulus_load_texture_to_opengl(romulus2d_get_raw_texture_data(texture), x, y, GL_RGBA32F, GL_RGBA, GL_FLOAT, quality) ;
            
        break;
            
        case romulus2d_depth_texture_format:
            
           return romulus_load_texture_to_opengl(romulus2d_get_raw_texture_data(texture), x, y, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT, 0) ;
            
        break;
            
        default:
            break;
    }
    
    return 0 ;
}

romulus_texture romulus_new_texture( romulus_scene scene, const char* texture_name, unsigned int texture_unit, romulus2d_texture raw_texture, float quality ) {
    
    romulus_set_active_scene(scene) ;
    
    romulus_texture texture = RKMem_NewMemOfType(struct romulus_texture_s) ;
    
    texture->raw_texture = raw_texture ;
    
    texture->texture_name = RKString_NewString(texture_name) ;
    
    texture->tid = romulus_load_raw_texture(raw_texture,quality) ;
    
    texture->tunit = (texture_unit > 15) ? 15 : texture_unit ;
    
    return texture ;
}

void romulus_destroy_texture( romulus_texture texture ) {
    
    RKString_DestroyString(texture->texture_name) ;
    
    glDeleteTextures(1, &texture->tid) ;
    
    free(texture) ;
}

void romulus_load_texture_to_shader( romulus_texture texture, romulus_shader shader ) {
   
    romulus_bind_texture_unit(texture->tunit) ;
    
    glBindTexture(GL_TEXTURE_2D, texture->tid) ;
    
    glUniform1i(glGetUniformLocation(shader->pid, RKString_GetString(texture->texture_name) ), texture->tunit) ;
}

int romulus_get_texture_width( romulus_texture texture ) {
    
    return romulus2d_texture_get_width(texture->raw_texture) ;
}

int romulus_get_texture_height( romulus_texture texture ) {
    
    return romulus2d_texture_get_height(texture->raw_texture) ;
}

romulus_material romulus_new_material( romulus_shader shader, romulus_init_material init, RKArgs material_args ) {
    
    romulus_material material = RKMem_NewMemOfType(struct romulus_material_s) ;
    
    material->active = 1 ;
    
    material->shader = shader ;
    
    material->attributes = shader->attributes ;
    
    material->init = init ;
    
    material->material_args = RKArgs_CloneArgs(material_args) ;
    
    material->mesh_list = RKList_NewList() ;
    
    return material ;
 }

void romulus_destroy_material( romulus_material material ) {
    
    RKArgs_DestroyClonedArgs(material->material_args) ;
    
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
    
    fbuf_tex = romulus_load_texture_to_opengl(0, render_buffer->width, render_buffer->height, GL_RGBA8, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, -1) ;
    
    fbuf_depth_tex = romulus_load_depth_texture_to_opengl(render_buffer->width, render_buffer->height) ;
    
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, fbuf_tex, 0) ;
    
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, fbuf_depth_tex, 0) ;
    
    GLenum drawbuffers[1] = {GL_COLOR_ATTACHMENT0} ;
   
    glDrawBuffers(1, drawbuffers) ;
     
    render_buffer->fbuf = fbuf ;
    
    render_buffer->fbuf_tex = fbuf_tex ;
    
    //clear the framebuffer?
    glBindFramebuffer(GL_FRAMEBUFFER, fbuf) ;
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f) ;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) ;
    glUseProgram(0) ;
    glDrawElements(GL_TRIANGLES, 0, GL_UNSIGNED_INT, 0) ;

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
    
    romulus_set_cull_back_face(stage, romulus_true) ;
    
    romulus_set_depth_test(stage, romulus_true) ;
    
    romulus_set_depth_alpha(stage, romulus_false) ;
    
    return stage ;
}

void romulus_destroy_render_stage( romulus_render_stage stage ) {
    
    free(stage) ;
}

romulus_camera romulus_get_camera_from_stage( romulus_render_stage stage ) {

    return stage->camera ;
}

romulus_render_buffer romulus_get_render_buffer_from_stage( romulus_render_stage stage ) {
    
    return stage->render_buffer ;
}

romulus_geometry romulus_get_geometry_from_stage( romulus_render_stage stage ) {
    
    return stage->geometry ;
}

void romulus_set_cull_back_face( romulus_render_stage stage, romulus_bool cull ) {
    
    stage->cull = cull ;
}

void romulus_set_depth_test( romulus_render_stage stage, romulus_bool depth ) {
    
    stage->depth = depth ;
}

void romulus_set_depth_alpha( romulus_render_stage stage, romulus_bool alpha ) {
    
    stage->alpha = alpha ;
}

void romulus_render( romulus_render_stage stage ) {
    
    romulus_set_active_scene(stage->render_buffer->scene) ;
    
    glBindFramebuffer(GL_FRAMEBUFFER, stage->render_buffer->fbuf) ;
    
    if (stage->depth)  {
        
        glEnable(GL_DEPTH_TEST) ;
        
    } else {
        
        glDisable(GL_DEPTH_TEST) ;
    }
    
    if (stage->cull)  {
        
        glEnable(GL_CULL_FACE) ;
        
    } else {
        
        glDisable(GL_CULL_FACE) ;
    }
    
    if (stage->alpha)  {
        
        glEnable(GL_BLEND) ;
        
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)  ;
        
    } else {
        
        glDisable(GL_BLEND) ;
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
            
            material->init(material->shader,material->material_args) ;
            
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
    
    glUseProgram(0) ;
    
    romulus_report_error( stage->render_buffer->scene, "romulus_render0") ;
    
    //clear the framebuffer?
    glBindFramebuffer(GL_FRAMEBUFFER, 0) ;
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f) ;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) ;
    glUseProgram(0) ;
    glDrawElements(GL_TRIANGLES, 0, GL_UNSIGNED_INT, 0) ;
    
    romulus_report_error( stage->render_buffer->scene, "romulus_render1") ;
}

static void romulus_present_attributes_binder( romulus_shader shader ) {
    
    romulus_bind_attribute_location_to_shader("position", 0, shader) ;
    
    romulus_bind_attribute_location_to_shader("texcoord", 1, shader) ;
}

static void romulus_present_attributes_constructor( int vertex_count, RKArgs args ) {
    
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
        
        mesh = romulus_new_mesh(stage->render_buffer->scene, romulus_present_attributes_constructor, newargs( args(GLfloat*,quad_pos_array,quad_texcoord_array) ),
                                                                                                                       romulus_present_attributes_binder, 4, quad_elementArray_array, 6) ;
        shader = romulus_new_shader(stage->render_buffer->scene, vertex_shader0, fragment_shader0, romulus_present_attributes_binder) ;
        
        init++ ;
    }
    
    glUseProgram(shader->pid) ;
    
    glViewport(0, 0, stage->render_buffer->scene->width, stage->render_buffer->scene->height) ;
    
    romulus_load_render_buffer_as_texture_to_shader(stage->render_buffer, "romulus_texture_0", 0, shader) ;
    
    glBindVertexArray(mesh->vao) ;
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo) ;
    
    glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_INT, 0) ;
    
    romulus_report_error( stage->render_buffer->scene, "romulus_present") ;
}

void romulus_run_loop( romulus_scene scene, romulus_run_loop_func_type run_loop_func, RKArgs run_args, romulus_run_quit_loop_func_type run_quit_loop_func, RKArgs quit_args ) {
    
    IDK_WindowRunLoop(scene->display_window->idk_window, run_loop_func, run_args, run_quit_loop_func, quit_args) ;
}