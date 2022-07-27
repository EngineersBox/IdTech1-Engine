#pragma once

#ifndef ID_TECH_1_DOOM_WALL_H
#define ID_TECH_1_DOOM_WALL_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Wall {
    struct {
        int x;
        int y;
    } pos1, pos2;
    int colour;
} Wall;

#ifdef __cplusplus
};
#endif

#endif //ID_TECH_1_DOOM_WALL_H
