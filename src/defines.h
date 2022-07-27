#pragma once


#ifndef ID_TECH_1_DOOM_DEFINES_H
#define ID_TECH_1_DOOM_DEFINES_H

#define WINDOW_WIDTH      160                       // Raw screen height
#define WINDOW_HEIGHT     120                       // Raw screen width
#define RESOLUTION        1                         // 0=160x120 1=360x240 4=640x480
#define SW                WINDOW_WIDTH*RESOLUTION   // screen width
#define SH                WINDOW_HEIGHT*RESOLUTION  // screen height
#define SW2               (SW/2)                    // half of screen width
#define SH2               (SH/2)                    // half of screen height
#define PIXEL_SCALE       4/RESOLUTION              // OpenGL pixel scale
#define GLSW              (SW*PIXEL_SCALE)          // OpenGL window width
#define GLSH              (SH*PIXEL_SCALE)          // OpenGL window height
#define Z_NEAR            0                         // Near clipping plane distance
#define Z_FAR             1000                      // Far clipping plane distance
#define BACKGROUND_COLOUR 0.07f, 0.13f, 0.17f, 1.0f // Clear colour
#define FOV 200                                     // FOV scaling, 200 ~ 90deg
#define LOOK_SCALE 32.0                             // Near-far distance scaling
#define SECTOR_COUNT 4                              // Sectors to load
#define WALL_COUNT 16                               // Walls to load (multiple of SECTOR_COUNT)

#define RAW_MOUSE_INPUT

#endif //ID_TECH_1_DOOM_DEFINES_H
