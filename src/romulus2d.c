//
//  romulus2d.c
//  romulus
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
"uniform sampler2D romulus_texture_0 ;"
"void main() {"
"fragColor = (texture(romulus_texture_0, out_texcoord.st)) ;"
"}";

struct romulus2d_texture_s { romulus2d_texture_format format ; int width ; int height ; void* data ; } ;

struct romulus2d_rect_s { romulus2d_rect_render_func render_func ; romulus_mesh quad ; romulus2d_texture raster_surface ;
    
RKMath_NewVector(background_color, 4) ; romulus2d_texture background ; romulus_texture texture ; } ;

struct romulus2d_scene_s { romulus_scene scene3d ; romulus_render_stage stage ; } ;

static void romulus2d_material_init( romulus_shader shader, RKArgs material_args ) {
    
    RKArgs_UseArgs(material_args) ;
    
    romulus_texture the_texture = *RKArgs_GetNextArg(material_args, romulus_texture*) ;
    
    romulus_load_texture_to_shader(the_texture, shader) ;
}

static void romulus2d_attributes_binder( romulus_shader shader ) {
    
    romulus_bind_attribute_location_to_shader("position", 0, shader) ;
    
    romulus_bind_attribute_location_to_shader("texcoord", 1, shader) ;
}

static void romulus2d_attributes_constructor( int vertex_count, RKArgs args ) {
    
    RKArgs_UseArgs(args) ;
    
    romulus_construct_attribute(0, RKArgs_GetNextArg(args, GLfloat*), romulus_float_type_id, 3, vertex_count, romulus_false) ;
    
    romulus_construct_attribute(1, RKArgs_GetNextArg(args, GLfloat*), romulus_float_type_id, 2, vertex_count, romulus_true) ;
}
/*
static romulus_render_stage new_render_stage_for_romulus2d( romulus_scene scene ) {
    
    RKMath_Vectorit(pos, 0, 0, 0) ;
    
    romulus_camera camera = romulus_new_camera(pos, 0, 0, 0) ;
    
    romulus_shader shader = romulus_new_shader(scene, vertex_shader0, fragment_shader0, romulus_attributes_base) ;
    
    romulus_new_material(shader, <#romulus_init_material init#>) ;
    
    return romulus_new_render_stage(<#romulus_camera camera#>, <#romulus_render_buffer render_buffer#>, <#romulus_geometry geometry#>)
}
*/
romulus2d_scene romulus2d_new_scene( romulus_scene scene ) {
    
    romulus2d_scene scene2d = RKMem_NewMemOfType(struct romulus2d_scene_s) ;
    
    scene2d->scene3d = scene ;
    
    scene2d->stage = NULL ;
    
    return scene2d ;
}

static inline float x_to_float( romulus2d_texture rs, unsigned int x ) {
    
    return (float)x / (rs->width) ;
}

static inline float y_to_float( romulus2d_texture rs, unsigned int y ) {
    
    return (float)y / (rs->height) ;
}

static inline unsigned int x_to_int( romulus2d_texture rs, float x ) {
    
    return (unsigned int)(x * (rs->width)) ;
}

static inline unsigned int y_to_int( romulus2d_texture rs, float y ) {
    
    return (unsigned int)(y * (rs->height)) ;
}

static inline void set_pixel( void* data, int rk, float red, float green, float blue, float alpha, romulus2d_texture_format format ) {
    
    switch ( format ) {
            
        case romulus2d_rgba8_texture_format:
            
            ((romulus2d_raw_texture_byte_data)data)[rk] = ((unsigned char)(red*255)) ;
            
            ((romulus2d_raw_texture_byte_data)data)[rk + 1] = ((unsigned char)(green*255)) ;
            
            ((romulus2d_raw_texture_byte_data)data)[rk + 2] = ((unsigned char)(blue*255)) ;
            
            ((romulus2d_raw_texture_byte_data)data)[rk + 3] = ((unsigned char)(alpha*255)) ;
            
            break ;
            
        case romulus2d_rgba32_texture_format:
            
            ((romulus2d_raw_texture_float_data)data)[rk] = red ;
            
            ((romulus2d_raw_texture_float_data)data)[rk + 1] = green ;
            
            ((romulus2d_raw_texture_float_data)data)[rk + 2] = blue ;
            
            ((romulus2d_raw_texture_float_data)data)[rk + 3] = alpha ;
            
            break ;
            
        case romulus2d_depth_texture_format:
            
            ((romulus2d_raw_texture_float_data)data)[rk] = RKMath_Max(RKMath_Max(red, green), RKMath_Max(blue, alpha)) ;
            
            break ;
            
        default:
            break;
    }
    
}

static inline void get_pixel( void* data, int rk, float* red, float* green, float* blue, float* alpha, romulus2d_texture_format format ) {
    
    switch ( format ) {
            
        case romulus2d_rgba8_texture_format:
            
            *red = ((float)((romulus2d_raw_texture_byte_data)data)[rk]) / 255.0f ;
            
            *green = ((float)((romulus2d_raw_texture_byte_data)data)[rk + 1]) / 255.0f ;
            
            *blue = ((float)((romulus2d_raw_texture_byte_data)data)[rk + 2]) / 255.0f ;
            
            *alpha = ((float)((romulus2d_raw_texture_byte_data)data)[rk + 3]) / 255.0f  ;
            
        break ;
            
        case romulus2d_rgba32_texture_format:
            
            *red =  ((romulus2d_raw_texture_float_data)data)[rk] ;
            
            *green =  ((romulus2d_raw_texture_float_data)data)[rk + 1] ;
            
            *blue =  ((romulus2d_raw_texture_float_data)data)[rk + 2] ;
            
            *alpha =  ((romulus2d_raw_texture_float_data)data)[rk + 3] ;
            
        break ;
            
        case romulus2d_depth_texture_format:
            
            *red =  ((romulus2d_raw_texture_float_data)data)[rk] ;
            
            *green =  ((romulus2d_raw_texture_float_data)data)[rk] ;
            
            *blue =  ((romulus2d_raw_texture_float_data)data)[rk] ;
            
            *alpha =  ((romulus2d_raw_texture_float_data)data)[rk] ;
            
        break ;
            
        default:
            break;
    }
    
}

romulus2d_texture romulus2d_new_texture( RKMVector background_color, int width, int height, romulus2d_texture_format format ) {
    
    romulus2d_texture texture = RKMem_NewMemOfType(struct romulus2d_texture_s) ;
    
    int f_size = 0 ;
    
    texture->width = width ;
    
    texture->height = height ;
    
    texture->format = format ;
    
    f_size = romulus2d_get_texture_format_size(texture) ;
    
    texture->data = malloc( width * height * f_size ) ;
    
    romulus2d_set_texture_to_color(texture, background_color) ;
    
    return texture ;
}

int romulus2d_texture_get_width( romulus2d_texture texture ) {
    
    return texture->width ;
}

int romulus2d_texture_get_height( romulus2d_texture texture ) {
    
    return texture->height ;
}

void* romulus2d_get_raw_texture_data( romulus2d_texture texture ) {
    
    return texture->data ;
}

int romulus2d_get_texture_format_size( romulus2d_texture texture ) {
    
    switch ( texture->format ) {
            
        case romulus2d_rgba8_texture_format:
        case romulus2d_rgba32_texture_format:
            
            return 4 ;
            
            break;
            
        case romulus2d_depth_texture_format:
            
            return 1 ;
            
            break ;
            
        default:
            break;
    }
}

romulus2d_texture_format romulus2d_get_texture_format( romulus2d_texture texture ) {
    
    return texture->format ;
}

void romulus2d_set_texture_to_color( romulus2d_texture texture, RKMVector color ) {
    
    int f_size = romulus2d_get_texture_format_size(texture) ;
    
    unsigned int size = texture->width * texture->height * f_size ;
    
    int rk = 0 ;
    
    float red = color[RKM_R] ;
    
    float green = color[RKM_G] ;
    
    float blue = color[RKM_B] ;
    
    float alpha = color[RKM_A] ;
    
    while ( rk < size ) {
        
        set_pixel(texture->data, rk, red, green, blue, alpha, texture->format) ;
        
        rk += f_size ;
    }
    
}

void romulus2d_texture_copy( romulus2d_texture dest, romulus2d_texture src ) {
    
    int f_size_src = romulus2d_get_texture_format_size(src) ;
    
    int f_size_dest = romulus2d_get_texture_format_size(dest) ;
    
    unsigned int src_size = src->width * src->height * sizeof(unsigned char) * f_size_src ;
    
    unsigned int dest_size = dest->width * dest->height * sizeof(unsigned char) * f_size_dest ;
    
    if ( src_size != dest_size ) return ;
    
    memcpy(dest->data, src->data, src_size) ;
}

romulus2d_rect romulus2d_new_rect( romulus2d_scene scene, romulus2d_rect_render_func render_func, RKMVector background_color, int width, int height,
                                  romulus2d_texture_format format ) {
    
    if ( scene != NULL ) return NULL ;
    
    romulus2d_rect rect = RKMem_NewMemOfType(struct romulus2d_rect_s) ;
    
    rect->quad = NULL ;
    
    rect->texture = NULL ;
    
    RKMath_Equal(rect->background_color, background_color, 4) ;
    
    rect->render_func = render_func ;
    
    rect->raster_surface = romulus2d_new_texture(rect->background_color, width, height, format) ;
    
    rect->background = romulus2d_new_texture(rect->background_color, width, height, format) ;
    
    return rect ;
}

void romulus2d_reset_rect( romulus2d_rect rect ) {
    
    romulus2d_texture_copy(rect->raster_surface, rect->background) ;
}

void romulus2d_render_rect( romulus2d_rect rect, RKArgs args ) {
    
    romulus2d_reset_rect(rect) ;
    
    if ( rect->render_func != NULL ) rect->render_func(rect, args) ;
}

romulus2d_texture romulus2d_get_texture_from_rect( romulus2d_rect rect ) {
    
    return rect->raster_surface ;
}

void romulus2d_set_pixel( romulus2d_rect rect, unsigned int x, unsigned int y, float red, float green, float blue, float alpha ) {
    
    romulus2d_texture rs = rect->raster_surface ;
    
    if ( (x >= rs->width) || (y >= rs->height) ) return ;
    
    int format_size = romulus2d_get_texture_format_size(rs)  ;
    
    unsigned int rk = ( x * format_size ) + ( y * rs->width * format_size ) ;
        
    set_pixel(rs->data, rk, red, green, blue, alpha, rs->format) ;
}

void romulus2d_get_pixel( romulus2d_rect rect, unsigned int x, unsigned int y, float* red, float* green, float* blue, float* alpha ) {
    
    romulus2d_texture rs = rect->raster_surface ;
    
    if ( (x >= rs->width) || (y >= rs->height) ) return ;
    
    int format_size = romulus2d_get_texture_format_size(rs) ;
    
    unsigned int rk = ( x * format_size ) + ( y * rs->width * format_size ) ;
    
    get_pixel(rs->data, rk, red, green, blue, alpha, rs->format) ;
}


/*
 * line3d was dervied from DigitalLine.c published as "Digital Line Drawing"
 * by Paul Heckbert from "Graphics Gems", Academic Press, 1990
 *
 * 3D modifications by Bob Pendleton. The original source code was in the public
 * domain, the author of the 3D version places his modifications in the
 * public domain as well.
 *
 * line3d uses Bresenham's algorithm to generate the 3 dimensional points on a
 * line from (x1, y1, z1) to (x2, y2, z2)
 *
 */

static void line3d(romulus2d_rect rect, int x1, int y1, int x2, int y2, int z1, int z2, float red, float green, float blue, float alpha) {
    
    int xd, yd, zd;
    int x, y, z;
    int ax, ay, az;
    int sx, sy, sz;
    int dx, dy, dz;
    
    dx = x2 - x1;
    dy = y2 - y1;
    dz = z2 - z1;
    
    ax = RKMath_Abs_m(dx) << 1;
    ay = RKMath_Abs_m(dy) << 1;
    az = RKMath_Abs_m(dz) << 1;
    
    sx = RKMath_Zsgn(dx);
    sy = RKMath_Zsgn(dy);
    sz = RKMath_Zsgn(dz);
    
    x = x1;
    y = y1;
    z = z1;
    
    if (ax >= RKMath_Max(ay, az))            /* x dominant */
    {
        yd = ay - (ax >> 1);
        zd = az - (ax >> 1);
        for (;;)
        {
            romulus2d_set_pixel(rect, x, y, red, green, blue, alpha) ;
            if (x == x2)
            {
                break;
            }
            
            if (yd >= 0)
            {
                y += sy;
                yd -= ax;
            }
            
            if (zd >= 0)
            {
                z += sz;
                zd -= ax;
            }
            
            x += sx;
            yd += ay;
            zd += az;
        }
    }
    else if (ay >= RKMath_Max(ax, az))            /* y dominant */
    {
        xd = ax - (ay >> 1);
        zd = az - (ay >> 1);
        for (;;)
        {
            romulus2d_set_pixel(rect, x, y, red, green, blue, alpha) ;
            if (y == y2)
            {
                break;
            }
            
            if (xd >= 0)
            {
                x += sx;
                xd -= ay;
            }
            
            if (zd >= 0)
            {
                z += sz;
                zd -= ay;
            }
            
            y += sy;
            xd += ax;
            zd += az;
        }
    }
    else if (az >= RKMath_Max(ax, ay))            /* z dominant */
    {
        xd = ax - (az >> 1);
        yd = ay - (az >> 1);
        for (;;)
        {
            romulus2d_set_pixel(rect, x, y, red, green, blue, alpha) ;
            if (z == z2)
            {
                break;
            }
            
            if (xd >= 0)
            {
                x += sx;
                xd -= az;
            }
            
            if (yd >= 0)
            {
                y += sy;
                yd -= az;
            }
            
            z += sz;
            xd += ax;
            yd += ay;
        }
    }
    
}


static void line3d_with_thickness(romulus2d_rect rect, int x1, int y1, int x2, int y2, int z1, int z2, float thickness, float red, float green, float blue, float alpha) {
    
    romulus2d_texture rs = rect->raster_surface ;
    
    int xd, yd, zd;
    int x, y, z;
    int ax, ay, az;
    int sx, sy, sz;
    int dx, dy, dz;
    
    dx = x2 - x1;
    dy = y2 - y1;
    dz = z2 - z1;
    
    ax = RKMath_Abs_m(dx) << 1;
    ay = RKMath_Abs_m(dy) << 1;
    az = RKMath_Abs_m(dz) << 1;
    
    sx = RKMath_Zsgn(dx);
    sy = RKMath_Zsgn(dy);
    sz = RKMath_Zsgn(dz);
    
    x = x1;
    y = y1;
    z = z1;
    
    if (ax >= RKMath_Max(ay, az))            /* x dominant */
    {
        yd = ay - (ax >> 1);
        zd = az - (ax >> 1);
        for (;;)
        {
            romulus2d_draw_rectangle(rect, thickness, thickness, x_to_float(rs, x), y_to_float(rs, y), red, green, blue, alpha) ;
            if (x == x2)
            {
                break;
            }
            
            if (yd >= 0)
            {
                y += sy;
                yd -= ax;
            }
            
            if (zd >= 0)
            {
                z += sz;
                zd -= ax;
            }
            
            x += sx;
            yd += ay;
            zd += az;
        }
    }
    else if (ay >= RKMath_Max(ax, az))            /* y dominant */
    {
        xd = ax - (ay >> 1);
        zd = az - (ay >> 1);
        for (;;)
        {
            romulus2d_draw_rectangle(rect, thickness, thickness, x_to_float(rs, x), y_to_float(rs, y), red, green, blue, alpha) ;
            if (y == y2)
            {
                break;
            }
            
            if (xd >= 0)
            {
                x += sx;
                xd -= ay;
            }
            
            if (zd >= 0)
            {
                z += sz;
                zd -= ay;
            }
            
            y += sy;
            xd += ax;
            zd += az;
        }
    }
    else if (az >= RKMath_Max(ax, ay))            /* z dominant */
    {
        xd = ax - (az >> 1);
        yd = ay - (az >> 1);
        for (;;)
        {
            romulus2d_draw_rectangle(rect, thickness, thickness, x_to_float(rs, x), y_to_float(rs, y), red, green, blue, alpha) ;
            if (z == z2)
            {
                break;
            }
            
            if (xd >= 0)
            {
                x += sx;
                xd -= az;
            }
            
            if (yd >= 0)
            {
                y += sy;
                yd -= az;
            }
            
            z += sz;
            xd += ax;
            yd += ay;
        }
    }
    
}

void romulus2d_draw_line( romulus2d_rect rect, float x1, float y1, float x2, float y2, float red, float green, float blue, float alpha ) {
    
    romulus2d_texture rs = rect->raster_surface ;
    
    RKMath_Clamp(&x1, 0, 1, 1) ;
    
    RKMath_Clamp(&x2, 0, 1, 1) ;
    
    RKMath_Clamp(&y1, 0, 1, 1) ;
    
    RKMath_Clamp(&y2, 0, 1, 1) ;
    
    line3d(rect, x_to_int(rs, x1), y_to_int(rs, y1), x_to_int(rs, x2), y_to_int(rs, y2), 0, 0, red, green, blue, alpha) ;
}

void romulus2d_draw_line_with_thickness( romulus2d_rect rect, float thickness, float x1, float y1, float x2, float y2, float red, float green, float blue, float alpha ) {
    
    romulus2d_texture rs = rect->raster_surface ;
    
    RKMath_Clamp(&x1, 0, 1, 1) ;
    
    RKMath_Clamp(&x2, 0, 1, 1) ;
    
    RKMath_Clamp(&y1, 0, 1, 1) ;
    
    RKMath_Clamp(&y2, 0, 1, 1) ;
    
    RKMath_Clamp(&thickness, 0, 1, 1) ;
    
    thickness *= 0.02 ;
    
    line3d_with_thickness(rect, x_to_int(rs, x1), y_to_int(rs, y1), x_to_int(rs, x2), y_to_int(rs, y2), 0, 0, thickness, red, green, blue, alpha) ;
}

void romulus2d_draw_path( romulus2d_rect rect, float* path_data, int num_of_points, float red, float green, float blue, float alpha ) {
    
    int i = 0 ;
    
    while ( i+3 < num_of_points ) {
        
        romulus2d_draw_line(rect, path_data[i], path_data[i+1], path_data[i+2], path_data[i+3], red, green, blue, alpha) ;
        
        i+=2 ;
    }
}

void romulus2d_draw_path_with_thickness( romulus2d_rect rect, float thickness, float* path_data, int num_of_points, float red, float green, float blue, float alpha ) {
    
    int i = 0 ;
    
    while ( i+3 < num_of_points ) {
        
        romulus2d_draw_line_with_thickness(rect, thickness, path_data[i], path_data[i+1], path_data[i+2], path_data[i+3], red, green, blue, alpha) ;
        
        i+=2 ;
    }
}

void romulus2d_draw_rectangle( romulus2d_rect rect, float size_x, float size_y, float x, float y, float red, float green, float blue, float alpha ) {
    
    int i = 0 ;
    
    int j = 0 ;
 
    RKMath_Clamp(&x, 0, 1, 1) ;
    
    RKMath_Clamp(&y, 0, 1, 1) ;
    
    RKMath_Clamp(&size_x, 0, 1, 1) ;
    
    RKMath_Clamp(&size_y, 0, 1, 1) ;
    
    romulus2d_texture rs = rect->raster_surface ;
    
    unsigned int size_x0 = x_to_int(rs, size_x) ;
    
    unsigned int size_y0 = y_to_int(rs, size_y) ;
    
    unsigned int x0 = x_to_int(rs, x) ;
    
    unsigned int y0 = y_to_int(rs, y) ;
    
    while (i < size_x0) {
        
        j = 0 ;
        
        while (j < size_y0) {
            
            romulus2d_set_pixel(rect, (i + x0), (j + y0), red, green, blue, alpha) ;
            
            j++ ;
        }
        
        i++ ;
    }

}

static void rectangle( romulus2d_rect rect, int size_x, int size_y, int x, int y, float red, float green, float blue, float alpha ) {
    
    if ( size_x > rect->raster_surface->width ) return ;
    
    if ( size_y > rect->raster_surface->height ) return ;
    
    int i = 0 ;
    
    int j = 0 ;
    
    while (i < size_x) {
        
        j = 0 ;
        
        while (j < size_y) {
            
            romulus2d_set_pixel(rect, (i + x), (j + y), red, green, blue, alpha) ;
            
            j++ ;
        }
        
        i++ ;
    }
    
}

static void dotline( romulus2d_rect rect, int line[], int x, int y, int size, float red, float green, float blue, float alpha ) {
    
    int i = 0 ;
    
    int spaces = 0 ;
    
    if (line != NULL) {
        
        while (line[i] != -1) {
            
            if (line[i] == 1) {
                
                rectangle(rect, size, size, x + (spaces * size), y, red, green, blue, alpha) ;
            }
            
            i++ ;
            
            spaces++ ;
        }
    }
}

static void draw_char( romulus2d_rect rect, char c, int size, int x, int y, float red, float green, float blue, float alpha  ) ;

void romulus2d_draw_string( romulus2d_rect rect, const char* string, float size, float x, float y, float red, float green, float blue, float alpha ) {
    
    romulus2d_texture rs = rect->raster_surface ;
    
    int strilen  = 0 ;
    
    int i = 0 ;
    
    RKMath_Clamp(&size, 0, 1, 1) ;
    
    RKMath_Clamp(&x, 0, 1, 1) ;
    
    RKMath_Clamp(&y, 0, 1, 1) ;
    
    strilen = (int) strlen( string ) ;
    
    unsigned int size0 = RKMath_Max(x_to_int(rs, size*0.03), y_to_int(rs, size*0.03)) ;
    
    unsigned int x0 = x_to_int(rs, x) ;
    
    unsigned int y0 = y_to_int(rs, y) ;
    
    while ( i < strilen ) {
        
        draw_char(rect, string[i], size0, x0 + ( i * (size0 * 5) ), y0, red, green, blue, alpha) ;
        
        i++ ;
    }
}

static void draw_char( romulus2d_rect rect, char c, int size, int x, int y, float red, float green, float blue, float alpha  ) {
    
    
    //A
    int A1[6] = {0,0,1,0,0,-1} ;
    int A2[6] = {0,1,0,1,0,-1} ;
    int A3[6] = {0,1,1,1,0,-1} ;
    int A4[6] = {0,1,0,1,0,-1} ;
    int A5[6] = {0,1,0,1,0,-1} ;
    //B
    int B1[6] = {0,1,1,0,0,-1} ;
    int B2[6] = {0,1,0,1,0,-1} ;
    int B3[6] = {0,1,1,1,0,-1} ;
    int B4[6] = {0,1,0,1,0,-1} ;
    int B5[6] = {0,1,1,0,0,-1} ;
    //C
    int C1[6] = {0,0,1,1,0,-1} ;
    int C2[6] = {0,1,0,0,0,-1} ;
    int C3[6] = {0,1,0,0,0,-1} ;
    int C4[6] = {0,1,0,0,0,-1} ;
    int C5[6] = {0,0,1,1,0,-1} ;
    //D
    int D1[6] = {0,1,1,0,0,-1} ;
    int D2[6] = {0,1,0,1,0,-1} ;
    int D3[6] = {0,1,0,1,0,-1} ;
    int D4[6] = {0,1,0,1,0,-1} ;
    int D5[6] = {0,1,1,0,0,-1} ;
    //E
    int E1[6] = {0,1,1,1,0,-1} ;
    int E2[6] = {0,1,0,0,0,-1} ;
    int E3[6] = {0,1,1,0,0,-1} ;
    int E4[6] = {0,1,0,0,0,-1} ;
    int E5[6] = {0,1,1,1,0,-1} ;
    //F
    int F1[6] = {0,1,1,1,0,-1} ;
    int F2[6] = {0,1,0,0,0,-1} ;
    int F3[6] = {0,1,1,1,0,-1} ;
    int F4[6] = {0,1,0,0,0,-1} ;
    int F5[6] = {0,1,0,0,0,-1} ;
    //G
    int G1[6] = {0,1,1,1,0,-1} ;
    int G2[6] = {0,1,0,0,0,-1} ;
    int G3[6] = {0,1,0,1,0,-1} ;
    int G4[6] = {0,1,0,1,0,-1} ;
    int G5[6] = {0,1,1,1,0,-1} ;
    //H
    int H1[6] = {0,1,0,1,0,-1} ;
    int H2[6] = {0,1,0,1,0,-1} ;
    int H3[6] = {0,1,1,1,0,-1} ;
    int H4[6] = {0,1,0,1,0,-1} ;
    int H5[6] = {0,1,0,1,0,-1} ;
    //I
    int I1[6] = {0,1,1,1,0,-1} ;
    int I2[6] = {0,0,1,0,0,-1} ;
    int I3[6] = {0,0,1,0,0,-1} ;
    int I4[6] = {0,0,1,0,0,-1} ;
    int I5[6] = {0,1,1,1,0,-1} ;
    //J
    int J1[6] = {0,1,1,1,0,-1} ;
    int J2[6] = {0,0,1,0,0,-1} ;
    int J3[6] = {0,0,1,0,0,-1} ;
    int J4[6] = {0,0,1,0,0,-1} ;
    int J5[6] = {0,1,1,0,0,-1} ;
    //K
    int K1[6] = {0,1,0,1,0,-1} ;
    int K2[6] = {0,1,1,0,0,-1} ;
    int K3[6] = {0,1,1,0,0,-1} ;
    int K4[6] = {0,1,1,0,0,-1} ;
    int K5[6] = {0,1,0,1,0,-1} ;
    //L
    int L1[6] = {0,1,0,0,0,-1} ;
    int L2[6] = {0,1,0,0,0,-1} ;
    int L3[6] = {0,1,0,0,0,-1} ;
    int L4[6] = {0,1,0,0,0,-1} ;
    int L5[6] = {0,1,1,1,0,-1} ;
    //M
    int M1[6] = {1,1,0,1,1,-1} ;
    int M2[6] = {1,0,1,0,1,-1} ;
    int M3[6] = {1,0,1,0,1,-1} ;
    int M4[6] = {1,0,1,0,1,-1} ;
    int M5[6] = {1,0,0,0,1,-1} ;
    //N
    int N1[6] = {0,1,0,0,1,-1} ;
    int N2[6] = {0,1,1,0,1,-1} ;
    int N3[6] = {0,1,0,1,1,-1} ;
    int N4[6] = {0,1,0,0,1,-1} ;
    int N5[6] = {0,1,0,0,1,-1} ;
    //O
    int O1[6] = {0,1,1,1,0,-1} ;
    int O2[6] = {0,1,0,1,0,-1} ;
    int O3[6] = {0,1,0,1,0,-1} ;
    int O4[6] = {0,1,0,1,0,-1} ;
    int O5[6] = {0,1,1,1,0,-1} ;
    //P
    int P1[6] = {0,1,1,1,0,-1} ;
    int P2[6] = {0,1,0,1,0,-1} ;
    int P3[6] = {0,1,1,1,0,-1} ;
    int P4[6] = {0,1,0,0,0,-1} ;
    int P5[6] = {0,1,0,0,0,-1} ;
    //Q
    int Q1[6] = {0,1,1,1,0,-1} ;
    int Q2[6] = {0,1,0,1,0,-1} ;
    int Q3[6] = {0,1,0,1,0,-1} ;
    int Q4[6] = {0,1,1,1,0,-1} ;
    int Q5[6] = {0,0,0,1,0,-1} ;
    //R
    int R1[6] = {0,1,1,1,0,-1} ;
    int R2[6] = {0,1,0,1,0,-1} ;
    int R3[6] = {0,1,1,0,0,-1} ;
    int R4[6] = {0,1,0,1,0,-1} ;
    int R5[6] = {0,1,0,0,1,-1} ;
    //S
    int S1[6] = {0,0,1,1,0,-1} ;
    int S2[6] = {0,0,1,0,0,-1} ;
    int S3[6] = {0,0,1,0,0,-1} ;
    int S4[6] = {0,0,0,1,0,-1} ;
    int S5[6] = {0,0,1,1,0,-1} ;
    //T
    int T1[6] = {0,1,1,1,0,-1} ;
    int T2[6] = {0,0,1,0,0,-1} ;
    int T3[6] = {0,0,1,0,0,-1} ;
    int T4[6] = {0,0,1,0,0,-1} ;
    int T5[6] = {0,0,1,0,0,-1} ;
    //U
    int U1[6] = {0,1,0,1,0,-1} ;
    int U2[6] = {0,1,0,1,0,-1} ;
    int U3[6] = {0,1,0,1,0,-1} ;
    int U4[6] = {0,1,0,1,0,-1} ;
    int U5[6] = {0,1,1,1,0,-1} ;
    //V
    int V1[6] = {0,1,0,1,0,-1} ;
    int V2[6] = {0,1,0,1,0,-1} ;
    int V3[6] = {0,1,0,1,0,-1} ;
    int V4[6] = {0,1,0,1,0,-1} ;
    int V5[6] = {0,0,1,0,0,-1} ;
    //W
    int W1[6] = {1,0,0,0,1,-1} ;
    int W2[6] = {1,0,1,0,1,-1} ;
    int W3[6] = {1,0,1,0,1,-1} ;
    int W4[6] = {1,0,1,0,1,-1} ;
    int W5[6] = {1,1,0,1,1,-1} ;
    //X
    int X1[6] = {0,1,0,1,0,-1} ;
    int X2[6] = {0,1,0,1,0,-1} ;
    int X3[6] = {0,0,1,0,0,-1} ;
    int X4[6] = {0,1,0,1,0,-1} ;
    int X5[6] = {0,1,0,1,0,-1} ;
    //Y
    int Y1[6] = {0,1,0,1,0,-1} ;
    int Y2[6] = {0,1,0,1,0,-1} ;
    int Y3[6] = {0,1,0,1,0,-1} ;
    int Y4[6] = {0,0,1,0,0,-1} ;
    int Y5[6] = {0,0,1,0,0,-1} ;
    //Z
    int Z1[6] = {0,1,1,1,0,-1} ;
    int Z2[6] = {0,0,0,1,0,-1} ;
    int Z3[6] = {0,0,1,0,0,-1} ;
    int Z4[6] = {0,1,0,0,0,-1} ;
    int Z5[6] = {0,1,1,1,0,-1} ;
    //.
    int PE1[6] = {0,0,0,0,0,-1} ;
    int PE2[6] = {0,0,0,0,0,-1} ;
    int PE3[6] = {0,0,0,0,0,-1} ;
    int PE4[6] = {0,0,0,0,0,-1} ;
    int PE5[6] = {0,0,1,0,0,-1} ;
    //!
    int EP1[6] = {0,0,1,0,0,-1} ;
    int EP2[6] = {0,0,1,0,0,-1} ;
    int EP3[6] = {0,0,1,0,0,-1} ;
    int EP4[6] = {0,0,0,0,0,-1} ;
    int EP5[6] = {0,0,1,0,0,-1} ;
    //?
    int QM1[6] = {0,1,1,0,0,-1} ;
    int QM2[6] = {0,0,1,0,0,-1} ;
    int QM3[6] = {0,0,1,0,0,-1} ;
    int QM4[6] = {0,0,0,0,0,-1} ;
    int QM5[6] = {0,0,1,0,0,-1} ;
    //1
    int ONE1[6] = {0,1,1,0,0,-1} ;
    int ONE2[6] = {0,0,1,0,0,-1} ;
    int ONE3[6] = {0,0,1,0,0,-1} ;
    int ONE4[6] = {0,0,1,0,0,-1} ;
    int ONE5[6] = {0,1,1,1,0,-1} ;
    //2
    int TWO1[6] = {0,0,1,1,0,-1} ;
    int TWO2[6] = {0,1,0,0,1,-1} ;
    int TWO3[6] = {0,0,0,1,0,-1} ;
    int TWO4[6] = {0,0,1,0,0,-1} ;
    int TWO5[6] = {0,1,1,1,1,-1} ;
    //3
    int THREE1[6] = {0,0,1,1,0,-1} ;
    int THREE2[6] = {0,1,0,0,1,-1} ;
    int THREE3[6] = {0,0,0,1,1,-1} ;
    int THREE4[6] = {0,1,0,0,1,-1} ;
    int THREE5[6] = {0,0,1,1,0,-1} ;
    //4
    int FOUR1[6] = {0,1,0,1,0,-1} ;
    int FOUR2[6] = {0,1,0,1,0,-1} ;
    int FOUR3[6] = {0,1,1,1,0,-1} ;
    int FOUR4[6] = {0,0,0,1,0,-1} ;
    int FOUR5[6] = {0,0,0,1,0,-1} ;
    //5
    int FIVE1[6] = {0,1,1,1,0,-1} ;
    int FIVE2[6] = {0,1,0,0,0,-1} ;
    int FIVE3[6] = {0,1,1,1,0,-1} ;
    int FIVE4[6] = {0,0,0,1,0,-1} ;
    int FIVE5[6] = {0,1,1,1,0,-1} ;
    //6
    int SIX1[6] = {0,1,1,1,0,-1} ;
    int SIX2[6] = {0,1,0,0,0,-1} ;
    int SIX3[6] = {0,1,1,1,0,-1} ;
    int SIX4[6] = {0,1,0,1,0,-1} ;
    int SIX5[6] = {0,1,1,1,0,-1} ;
    //7
    int SEVEN1[6] = {0,1,1,1,1,-1} ;
    int SEVEN2[6] = {0,0,0,0,1,-1} ;
    int SEVEN3[6] = {0,0,0,1,0,-1} ;
    int SEVEN4[6] = {0,0,1,0,0,-1} ;
    int SEVEN5[6] = {0,1,0,0,0,-1} ;
    //8
    int EIGHT1[6] = {0,1,1,1,0,-1} ;
    int EIGHT2[6] = {0,1,0,1,0,-1} ;
    int EIGHT3[6] = {0,1,1,1,0,-1} ;
    int EIGHT4[6] = {0,1,0,1,0,-1} ;
    int EIGHT5[6] = {0,1,1,1,0,-1} ;
    //9
    int NINE1[6] = {0,1,1,1,0,-1} ;
    int NINE2[6] = {0,1,0,1,0,-1} ;
    int NINE3[6] = {0,1,1,1,0,-1} ;
    int NINE4[6] = {0,0,0,1,0,-1} ;
    int NINE5[6] = {0,1,1,1,0,-1} ;
    //0
    int ZERO1[6] = {0,0,1,1,0,-1} ;
    int ZERO2[6] = {0,1,0,0,1,-1} ;
    int ZERO3[6] = {0,1,0,0,1,-1} ;
    int ZERO4[6] = {0,1,0,0,1,-1} ;
    int ZERO5[6] = {0,0,1,1,0,-1} ;
    //&-cursor
    int CURSOR1[6] = {0,1,1,1,0,-1} ;
    int CURSOR2[6] = {0,1,1,1,0,-1} ;
    int CURSOR3[6] = {0,1,1,1,0,-1} ;
    int CURSOR4[6] = {0,1,1,1,0,-1} ;
    int CURSOR5[6] = {0,1,1,1,0,-1} ;
    
    switch (c) {
        case 'A':
        case 'a':
            dotline(rect, A1,x,(y + (4 * size)),size,red,green,blue,alpha) ;
            dotline(rect, A2,x,(y + (3 * size)),size,red,green,blue,alpha) ;
            dotline(rect, A3,x,(y + (2 * size)),size,red,green,blue,alpha) ;
            dotline(rect, A4,x,(y + (1 * size)),size,red,green,blue,alpha) ;
            dotline(rect, A5,x,(y + (0 * size)),size,red,green,blue,alpha) ;
            break;
        case 'B':
        case 'b':
            dotline(rect, B1,x,(y + (4 * size)),size,red,green,blue,alpha) ;
            dotline(rect, B2,x,(y + (3 * size)),size,red,green,blue,alpha) ;
            dotline(rect, B3,x,(y + (2 * size)),size,red,green,blue,alpha) ;
            dotline(rect, B4,x,(y + (1 * size)),size,red,green,blue,alpha) ;
            dotline(rect, B5,x,(y + (0 * size)),size,red,green,blue,alpha) ;
            break;
        case 'C':
        case 'c':
            dotline(rect, C1,x,(y + (4 * size)),size,red,green,blue,alpha) ;
            dotline(rect, C2,x,(y + (3 * size)),size,red,green,blue,alpha) ;
            dotline(rect, C3,x,(y + (2 * size)),size,red,green,blue,alpha) ;
            dotline(rect, C4,x,(y + (1 * size)),size,red,green,blue,alpha) ;
            dotline(rect, C5,x,(y + (0 * size)),size,red,green,blue,alpha) ;
            break;
        case 'D':
        case 'd':
            dotline(rect, D1,x,(y + (4 * size)),size,red,green,blue,alpha) ;
            dotline(rect, D2,x,(y + (3 * size)),size,red,green,blue,alpha) ;
            dotline(rect, D3,x,(y + (2 * size)),size,red,green,blue,alpha) ;
            dotline(rect, D4,x,(y + (1 * size)),size,red,green,blue,alpha) ;
            dotline(rect, D5,x,(y + (0 * size)),size,red,green,blue,alpha) ;
            break;
        case 'E':
        case 'e':
            dotline(rect, E1,x,(y + (4 * size)),size,red,green,blue,alpha) ;
            dotline(rect, E2,x,(y + (3 * size)),size,red,green,blue,alpha) ;
            dotline(rect, E3,x,(y + (2 * size)),size,red,green,blue,alpha) ;
            dotline(rect, E4,x,(y + (1 * size)),size,red,green,blue,alpha) ;
            dotline(rect, E5,x,(y + (0 * size)),size,red,green,blue,alpha) ;
            break;
        case 'F':
        case 'f':
            dotline(rect, F1,x,(y + (4 * size)),size,red,green,blue,alpha) ;
            dotline(rect, F2,x,(y + (3 * size)),size,red,green,blue,alpha) ;
            dotline(rect, F3,x,(y + (2 * size)),size,red,green,blue,alpha) ;
            dotline(rect, F4,x,(y + (1 * size)),size,red,green,blue,alpha) ;
            dotline(rect, F5,x,(y + (0 * size)),size,red,green,blue,alpha) ;
            break;
        case 'G':
        case 'g':
            dotline(rect, G1,x,(y + (4 * size)),size,red,green,blue,alpha) ;
            dotline(rect, G2,x,(y + (3 * size)),size,red,green,blue,alpha) ;
            dotline(rect, G3,x,(y + (2 * size)),size,red,green,blue,alpha) ;
            dotline(rect, G4,x,(y + (1 * size)),size,red,green,blue,alpha) ;
            dotline(rect, G5,x,(y + (0 * size)),size,red,green,blue,alpha) ;
            break;
        case 'H':
        case 'h':
            dotline(rect, H1,x,(y + (4 * size)),size,red,green,blue,alpha) ;
            dotline(rect, H2,x,(y + (3 * size)),size,red,green,blue,alpha) ;
            dotline(rect, H3,x,(y + (2 * size)),size,red,green,blue,alpha) ;
            dotline(rect, H4,x,(y + (1 * size)),size,red,green,blue,alpha) ;
            dotline(rect, H5,x,(y + (0 * size)),size,red,green,blue,alpha) ;
            break;
        case 'I':
        case 'i':
            dotline(rect, I1,x,(y + (4 * size)),size,red,green,blue,alpha) ;
            dotline(rect, I2,x,(y + (3 * size)),size,red,green,blue,alpha) ;
            dotline(rect, I3,x,(y + (2 * size)),size,red,green,blue,alpha) ;
            dotline(rect, I4,x,(y + (1 * size)),size,red,green,blue,alpha) ;
            dotline(rect, I5,x,(y + (0 * size)),size,red,green,blue,alpha) ;
            break;
        case 'J':
        case 'j':
            dotline(rect, J1,x,(y + (4 * size)),size,red,green,blue,alpha) ;
            dotline(rect, J2,x,(y + (3 * size)),size,red,green,blue,alpha) ;
            dotline(rect, J3,x,(y + (2 * size)),size,red,green,blue,alpha) ;
            dotline(rect, J4,x,(y + (1 * size)),size,red,green,blue,alpha) ;
            dotline(rect, J5,x,(y + (0 * size)),size,red,green,blue,alpha) ;
            break;
        case 'K':
        case 'k':
            dotline(rect, K1,x,(y + (4 * size)),size,red,green,blue,alpha) ;
            dotline(rect, K2,x,(y + (3 * size)),size,red,green,blue,alpha) ;
            dotline(rect, K3,x,(y + (2 * size)),size,red,green,blue,alpha) ;
            dotline(rect, K4,x,(y + (1 * size)),size,red,green,blue,alpha) ;
            dotline(rect, K5,x,(y + (0 * size)),size,red,green,blue,alpha) ;
            break;
        case 'L':
        case 'l':
            dotline(rect, L1,x,(y + (4 * size)),size,red,green,blue,alpha) ;
            dotline(rect, L2,x,(y + (3 * size)),size,red,green,blue,alpha) ;
            dotline(rect, L3,x,(y + (2 * size)),size,red,green,blue,alpha) ;
            dotline(rect, L4,x,(y + (1 * size)),size,red,green,blue,alpha) ;
            dotline(rect, L5,x,(y + (0 * size)),size,red,green,blue,alpha) ;
            break;
        case 'M':
        case 'm':
            dotline(rect, M1,x,(y + (4 * size)),size,red,green,blue,alpha) ;
            dotline(rect, M2,x,(y + (3 * size)),size,red,green,blue,alpha) ;
            dotline(rect, M3,x,(y + (2 * size)),size,red,green,blue,alpha) ;
            dotline(rect, M4,x,(y + (1 * size)),size,red,green,blue,alpha) ;
            dotline(rect, M5,x,(y + (0 * size)),size,red,green,blue,alpha) ;
            break;
        case 'N':
        case 'n':
            dotline(rect, N1,x,(y + (4 * size)),size,red,green,blue,alpha) ;
            dotline(rect, N2,x,(y + (3 * size)),size,red,green,blue,alpha) ;
            dotline(rect, N3,x,(y + (2 * size)),size,red,green,blue,alpha) ;
            dotline(rect, N4,x,(y + (1 * size)),size,red,green,blue,alpha) ;
            dotline(rect, N5,x,(y + (0 * size)),size,red,green,blue,alpha) ;
            break;
        case 'O':
        case 'o':
            dotline(rect, O1,x,(y + (4 * size)),size,red,green,blue,alpha) ;
            dotline(rect, O2,x,(y + (3 * size)),size,red,green,blue,alpha) ;
            dotline(rect, O3,x,(y + (2 * size)),size,red,green,blue,alpha) ;
            dotline(rect, O4,x,(y + (1 * size)),size,red,green,blue,alpha) ;
            dotline(rect, O5,x,(y + (0 * size)),size,red,green,blue,alpha) ;
            break;
        case 'P':
        case 'p':
            dotline(rect, P1,x,(y + (4 * size)),size,red,green,blue,alpha) ;
            dotline(rect, P2,x,(y + (3 * size)),size,red,green,blue,alpha) ;
            dotline(rect, P3,x,(y + (2 * size)),size,red,green,blue,alpha) ;
            dotline(rect, P4,x,(y + (1 * size)),size,red,green,blue,alpha) ;
            dotline(rect, P5,x,(y + (0 * size)),size,red,green,blue,alpha) ;
            break;
        case 'Q':
        case 'q':
            dotline(rect, Q1,x,(y + (4 * size)),size,red,green,blue,alpha) ;
            dotline(rect, Q2,x,(y + (3 * size)),size,red,green,blue,alpha) ;
            dotline(rect, Q3,x,(y + (2 * size)),size,red,green,blue,alpha) ;
            dotline(rect, Q4,x,(y + (1 * size)),size,red,green,blue,alpha) ;
            dotline(rect, Q5,x,(y + (0 * size)),size,red,green,blue,alpha) ;
            break;
        case 'R':
        case 'r':
            dotline(rect, R1,x,(y + (4 * size)),size,red,green,blue,alpha) ;
            dotline(rect, R2,x,(y + (3 * size)),size,red,green,blue,alpha) ;
            dotline(rect, R3,x,(y + (2 * size)),size,red,green,blue,alpha) ;
            dotline(rect, R4,x,(y + (1 * size)),size,red,green,blue,alpha) ;
            dotline(rect, R5,x,(y + (0 * size)),size,red,green,blue,alpha) ;
            break;
        case 'S':
        case 's':
            dotline(rect, S1,x,(y + (4 * size)),size,red,green,blue,alpha) ;
            dotline(rect, S2,x,(y + (3 * size)),size,red,green,blue,alpha) ;
            dotline(rect, S3,x,(y + (2 * size)),size,red,green,blue,alpha) ;
            dotline(rect, S4,x,(y + (1 * size)),size,red,green,blue,alpha) ;
            dotline(rect, S5,x,(y + (0 * size)),size,red,green,blue,alpha) ;
            break;
        case 'T':
        case 't':
            dotline(rect, T1,x,(y + (4 * size)),size,red,green,blue,alpha) ;
            dotline(rect, T2,x,(y + (3 * size)),size,red,green,blue,alpha) ;
            dotline(rect, T3,x,(y + (2 * size)),size,red,green,blue,alpha) ;
            dotline(rect, T4,x,(y + (1 * size)),size,red,green,blue,alpha) ;
            dotline(rect, T5,x,(y + (0 * size)),size,red,green,blue,alpha) ;
            break;
        case 'U':
        case 'u':
            dotline(rect, U1,x,(y + (4 * size)),size,red,green,blue,alpha) ;
            dotline(rect, U2,x,(y + (3 * size)),size,red,green,blue,alpha) ;
            dotline(rect, U3,x,(y + (2 * size)),size,red,green,blue,alpha) ;
            dotline(rect, U4,x,(y + (1 * size)),size,red,green,blue,alpha) ;
            dotline(rect, U5,x,(y + (0 * size)),size,red,green,blue,alpha) ;
            break;
        case 'V':
        case 'v':
            dotline(rect, V1,x,(y + (4 * size)),size,red,green,blue,alpha) ;
            dotline(rect, V2,x,(y + (3 * size)),size,red,green,blue,alpha) ;
            dotline(rect, V3,x,(y + (2 * size)),size,red,green,blue,alpha) ;
            dotline(rect, V4,x,(y + (1 * size)),size,red,green,blue,alpha) ;
            dotline(rect, V5,x,(y + (0 * size)),size,red,green,blue,alpha) ;
            break;
        case 'W':
        case 'w':
            dotline(rect, W1,x,(y + (4 * size)),size,red,green,blue,alpha) ;
            dotline(rect, W2,x,(y + (3 * size)),size,red,green,blue,alpha) ;
            dotline(rect, W3,x,(y + (2 * size)),size,red,green,blue,alpha) ;
            dotline(rect, W4,x,(y + (1 * size)),size,red,green,blue,alpha) ;
            dotline(rect, W5,x,(y + (0 * size)),size,red,green,blue,alpha) ;
            break;
        case 'X':
        case 'x':
            dotline(rect, X1,x,(y + (4 * size)),size,red,green,blue,alpha) ;
            dotline(rect, X2,x,(y + (3 * size)),size,red,green,blue,alpha) ;
            dotline(rect, X3,x,(y + (2 * size)),size,red,green,blue,alpha) ;
            dotline(rect, X4,x,(y + (1 * size)),size,red,green,blue,alpha) ;
            dotline(rect, X5,x,(y + (0 * size)),size,red,green,blue,alpha) ;
            break;
        case 'Y':
        case 'y':
            dotline(rect, Y1,x,(y + (4 * size)),size,red,green,blue,alpha) ;
            dotline(rect, Y2,x,(y + (3 * size)),size,red,green,blue,alpha) ;
            dotline(rect, Y3,x,(y + (2 * size)),size,red,green,blue,alpha) ;
            dotline(rect, Y4,x,(y + (1 * size)),size,red,green,blue,alpha) ;
            dotline(rect, Y5,x,(y + (0 * size)),size,red,green,blue,alpha) ;
            break;
        case 'Z':
        case 'z':
            dotline(rect, Z1,x,(y + (4 * size)),size,red,green,blue,alpha) ;
            dotline(rect, Z2,x,(y + (3 * size)),size,red,green,blue,alpha) ;
            dotline(rect, Z3,x,(y + (2 * size)),size,red,green,blue,alpha) ;
            dotline(rect, Z4,x,(y + (1 * size)),size,red,green,blue,alpha) ;
            dotline(rect, Z5,x,(y + (0 * size)),size,red,green,blue,alpha) ;
            break;
        case '.':
            dotline(rect, PE1,x,(y + (4 * size)),size,red,green,blue,alpha) ;
            dotline(rect, PE2,x,(y + (3 * size)),size,red,green,blue,alpha) ;
            dotline(rect, PE3,x,(y + (2 * size)),size,red,green,blue,alpha) ;
            dotline(rect, PE4,x,(y + (1 * size)),size,red,green,blue,alpha) ;
            dotline(rect, PE5,x,(y + (0 * size)),size,red,green,blue,alpha) ;
            break;
        case '!':
            dotline(rect, EP1,x,(y + (4 * size)),size,red,green,blue,alpha) ;
            dotline(rect, EP2,x,(y + (3 * size)),size,red,green,blue,alpha) ;
            dotline(rect, EP3,x,(y + (2 * size)),size,red,green,blue,alpha) ;
            dotline(rect, EP4,x,(y + (1 * size)),size,red,green,blue,alpha) ;
            dotline(rect, EP5,x,(y + (0 * size)),size,red,green,blue,alpha) ;
            break;
        case '?':
            dotline(rect, QM1,x,(y + (4 * size)),size,red,green,blue,alpha) ;
            dotline(rect, QM2,x,(y + (3 * size)),size,red,green,blue,alpha) ;
            dotline(rect, QM3,x,(y + (2 * size)),size,red,green,blue,alpha) ;
            dotline(rect, QM4,x,(y + (1 * size)),size,red,green,blue,alpha) ;
            dotline(rect, QM5,x,(y + (0 * size)),size,red,green,blue,alpha) ;
            break;
        case '1':
            dotline(rect, ONE1,x,(y + (4 * size)),size,red,green,blue,alpha) ;
            dotline(rect, ONE2,x,(y + (3 * size)),size,red,green,blue,alpha) ;
            dotline(rect, ONE3,x,(y + (2 * size)),size,red,green,blue,alpha) ;
            dotline(rect, ONE4,x,(y + (1 * size)),size,red,green,blue,alpha) ;
            dotline(rect, ONE5,x,(y + (0 * size)),size,red,green,blue,alpha) ;
            break;
        case '2':
            dotline(rect, TWO1,x,(y + (4 * size)),size,red,green,blue,alpha) ;
            dotline(rect, TWO2,x,(y + (3 * size)),size,red,green,blue,alpha) ;
            dotline(rect, TWO3,x,(y + (2 * size)),size,red,green,blue,alpha) ;
            dotline(rect, TWO4,x,(y + (1 * size)),size,red,green,blue,alpha) ;
            dotline(rect, TWO5,x,(y + (0 * size)),size,red,green,blue,alpha) ;
            break;
        case '3':
            dotline(rect, THREE1,x,(y + (4 * size)),size,red,green,blue,alpha) ;
            dotline(rect, THREE2,x,(y + (3 * size)),size,red,green,blue,alpha) ;
            dotline(rect, THREE3,x,(y + (2 * size)),size,red,green,blue,alpha) ;
            dotline(rect, THREE4,x,(y + (1 * size)),size,red,green,blue,alpha) ;
            dotline(rect, THREE5,x,(y + (0 * size)),size,red,green,blue,alpha) ;
            break;
        case '4':
            dotline(rect, FOUR1,x,(y + (4 * size)),size,red,green,blue,alpha) ;
            dotline(rect, FOUR2,x,(y + (3 * size)),size,red,green,blue,alpha) ;
            dotline(rect, FOUR3,x,(y + (2 * size)),size,red,green,blue,alpha) ;
            dotline(rect, FOUR4,x,(y + (1 * size)),size,red,green,blue,alpha) ;
            dotline(rect, FOUR5,x,(y + (0 * size)),size,red,green,blue,alpha) ;
            break;
        case '5':
            dotline(rect, FIVE1,x,(y + (4 * size)),size,red,green,blue,alpha) ;
            dotline(rect, FIVE2,x,(y + (3 * size)),size,red,green,blue,alpha) ;
            dotline(rect, FIVE3,x,(y + (2 * size)),size,red,green,blue,alpha) ;
            dotline(rect, FIVE4,x,(y + (1 * size)),size,red,green,blue,alpha) ;
            dotline(rect, FIVE5,x,(y + (0 * size)),size,red,green,blue,alpha) ;
            break;
        case '6':
            dotline(rect, SIX1,x,(y + (4 * size)),size,red,green,blue,alpha) ;
            dotline(rect, SIX2,x,(y + (3 * size)),size,red,green,blue,alpha) ;
            dotline(rect, SIX3,x,(y + (2 * size)),size,red,green,blue,alpha) ;
            dotline(rect, SIX4,x,(y + (1 * size)),size,red,green,blue,alpha) ;
            dotline(rect, SIX5,x,(y + (0 * size)),size,red,green,blue,alpha) ;
            break;
        case '7':
            dotline(rect, SEVEN1,x,(y + (4 * size)),size,red,green,blue,alpha) ;
            dotline(rect, SEVEN2,x,(y + (3 * size)),size,red,green,blue,alpha) ;
            dotline(rect, SEVEN3,x,(y + (2 * size)),size,red,green,blue,alpha) ;
            dotline(rect, SEVEN4,x,(y + (1 * size)),size,red,green,blue,alpha) ;
            dotline(rect, SEVEN5,x,(y + (0 * size)),size,red,green,blue,alpha) ;
            break;
        case '8':
            dotline(rect, EIGHT1,x,(y + (4 * size)),size,red,green,blue,alpha) ;
            dotline(rect, EIGHT2,x,(y + (3 * size)),size,red,green,blue,alpha) ;
            dotline(rect, EIGHT3,x,(y + (2 * size)),size,red,green,blue,alpha) ;
            dotline(rect, EIGHT4,x,(y + (1 * size)),size,red,green,blue,alpha) ;
            dotline(rect, EIGHT5,x,(y + (0 * size)),size,red,green,blue,alpha) ;
            break;
        case '9':
            dotline(rect, NINE1,x,(y + (4 * size)),size,red,green,blue,alpha) ;
            dotline(rect, NINE2,x,(y + (3 * size)),size,red,green,blue,alpha) ;
            dotline(rect, NINE3,x,(y + (2 * size)),size,red,green,blue,alpha) ;
            dotline(rect, NINE4,x,(y + (1 * size)),size,red,green,blue,alpha) ;
            dotline(rect, NINE5,x,(y + (0 * size)),size,red,green,blue,alpha) ;
            break;
        case '0':
            dotline(rect, ZERO1,x,(y + (4 * size)),size,red,green,blue,alpha) ;
            dotline(rect, ZERO2,x,(y + (3 * size)),size,red,green,blue,alpha) ;
            dotline(rect, ZERO3,x,(y + (2 * size)),size,red,green,blue,alpha) ;
            dotline(rect, ZERO4,x,(y + (1 * size)),size,red,green,blue,alpha) ;
            dotline(rect, ZERO5,x,(y + (0 * size)),size,red,green,blue,alpha) ;
            break;
        case '&':
            dotline(rect, CURSOR1,x,(y + (4 * size)),size,red,green,blue,alpha) ;
            dotline(rect, CURSOR2,x,(y + (3 * size)),size,red,green,blue,alpha) ;
            dotline(rect, CURSOR3,x,(y + (2 * size)),size,red,green,blue,alpha) ;
            dotline(rect, CURSOR4,x,(y + (1 * size)),size,red,green,blue,alpha) ;
            dotline(rect, CURSOR5,x,(y + (0 * size)),size,red,green,blue,alpha) ;
            break;
        default:
            break;
    }
    
}
