/*
 Copyright (c) 2017-2018 Jacob Gordon. All rights reserved.
 
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


#ifndef romulus2d_h
#define romulus2d_h

typedef struct romulus2d_texture_s* romulus2d_texture ;

typedef struct romulus2d_rect_s* romulus2d_rect ;

typedef struct romulus2d_scene_s* romulus2d_scene ;

typedef enum { romulus2d_rgba8_texture_format, romulus2d_rgba32_texture_format, romulus2d_depth_texture_format } romulus2d_texture_format ;

typedef unsigned char* romulus2d_raw_texture_byte_data ;

typedef unsigned int* romulus2d_raw_texture_int_data ;

typedef float* romulus2d_raw_texture_float_data ;

typedef void (*romulus2d_rect_render_func)(romulus2d_rect rect, RKArgs args) ;

romulus2d_texture romulus2d_new_texture( RKMVector background_color, int width, int height, romulus2d_texture_format format ) ;

romulus2d_texture romulus2d_new_texture_from_png( const char* filepath ) ;

void romulus2d_destroy_texture( romulus2d_texture texture ) ;

int romulus2d_texture_get_width( romulus2d_texture texture ) ;

int romulus2d_texture_get_height( romulus2d_texture texture ) ;

void* romulus2d_get_raw_texture_data( romulus2d_texture texture ) ;

int romulus2d_get_texture_format_size( romulus2d_texture texture ) ;

romulus2d_texture_format romulus2d_get_texture_format( romulus2d_texture texture ) ;

void romulus2d_set_texture_to_color( romulus2d_texture texture, RKMVector color ) ;

void romulus2d_texture_copy( romulus2d_texture dest, romulus2d_texture src ) ;

romulus2d_rect romulus2d_new_rect( romulus2d_scene scene, romulus2d_rect_render_func render_func, RKMVector background_color, int width, int height,
                                  romulus2d_texture_format format ) ;

void romulus2d_reset_rect( romulus2d_rect rect ) ;

void romulus2d_render_rect( romulus2d_rect rect, RKArgs args ) ;

romulus2d_texture romulus2d_get_texture_from_rect( romulus2d_rect rect ) ;

void romulus2d_set_pixel( romulus2d_rect rect, unsigned int x, unsigned int y, float red, float green, float blue, float alpha ) ;

void romulus2d_get_pixel( romulus2d_rect rect, unsigned int x, unsigned int y, float* red, float* green, float* blue, float* alpha ) ;

void romulus2d_draw_line( romulus2d_rect rect, float x1, float y1, float x2, float y2, float red, float green, float blue, float alpha ) ;

void romulus2d_draw_line_with_thickness( romulus2d_rect rect, float thickness, float x1, float y1, float x2, float y2, float red, float green, float blue, float alpha ) ;

void romulus2d_draw_path( romulus2d_rect rect, float* path_data, int num_of_points, float red, float green, float blue, float alpha ) ;

void romulus2d_draw_path_with_thickness( romulus2d_rect rect, float thickness, float* path_data, int num_of_points, float red, float green, float blue, float alpha ) ;

void romulus2d_draw_rectangle( romulus2d_rect rect, float size_x, float size_y, float x, float y, float red, float green, float blue, float alpha ) ;

void romulus2d_draw_string( romulus2d_rect rect, const char* string, float size, float x, float y, float red, float green, float blue, float alpha ) ;

#endif /* romulus2d_h */
