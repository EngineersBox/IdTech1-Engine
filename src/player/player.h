#pragma once

#ifndef ID_TECH_1_DOOM_PLAYER_H
#define ID_TECH_1_DOOM_PLAYER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Player {
    struct {
        int x;
        int y;
        int z;
    } pos;
    int angle;
    int look;
} Player;

#ifdef __cplusplus
};
#endif

#endif //ID_TECH_1_DOOM_PLAYER_H
