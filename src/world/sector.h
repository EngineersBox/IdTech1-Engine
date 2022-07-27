#pragma once

#ifndef ID_TECH_1_DOOM_SECTOR_H
#define ID_TECH_1_DOOM_SECTOR_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Sector {
    struct {
        int start;
        int end;
    } walls;
    struct {
        int z1;
        int z2;
    } heightBounds;
    struct {
        int x;
        int y;
    } centre;
    int dist;
} Sector;

#ifdef __cplusplus
};
#endif

#endif //ID_TECH_1_DOOM_SECTOR_H
