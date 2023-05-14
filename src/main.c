#define _USE_MATH_DEFINES
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "defines.h"
#include "player/player.h"
#include "world/wall.h"
#include "world/sector.h"

typedef struct Keys {
    int w, a, s, d;
    int strafeLeft, strafeRight;
    int move;
} Keys;
Keys keys;

Player player;

void keyCallback(GLFWwindow* window, int key, int scanCode, int action, int modifiers) {
    #define SET_ACTION_MAPPING(field, keyCode) case keyCode: \
        if (action == GLFW_PRESS) { keys.field = 1;}  \
        else if (action == GLFW_RELEASE) { keys.field = 0; } \
        break;

    switch (key) {
        SET_ACTION_MAPPING(w, GLFW_KEY_W)
        SET_ACTION_MAPPING(a, GLFW_KEY_A)
        SET_ACTION_MAPPING(s, GLFW_KEY_S)
        SET_ACTION_MAPPING(d, GLFW_KEY_D)
        SET_ACTION_MAPPING(move, GLFW_KEY_M)
        SET_ACTION_MAPPING(strafeLeft, GLFW_KEY_LEFT)
        SET_ACTION_MAPPING(strafeRight, GLFW_KEY_RIGHT)
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, true);
            break;
        default:
            break;
    }
}

GLFWwindow* initGL(const int width, const int height, const char* title) {
    glfwInit();
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(
        width,
        height,
        title,
        NULL,
        NULL
    );
    if (window == NULL) {
        glfwTerminate();
        exit(1);
    }

    glfwSetKeyCallback(window, keyCallback);
//    if (glfwRawMouseMotionSupported() == GLFW_TRUE) {
//        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
//#ifdef RAW_MOUSE_INPUT
//        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
//#endif
//    }

    glfwMakeContextCurrent(window);
    gladLoadGL();
    glViewport(0, 0, width, height);
    glOrtho(0, width, 0, height, Z_NEAR, Z_FAR);
    glPointSize(PIXEL_SCALE);
    return window;
}

void clearBackground() {
    glClearColor(BACKGROUND_COLOUR);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void pixel(int x, int y, int c) {
    int rgb[3];
#define MAP_COLOUR(index, r, g, b) case index: rgb[0] = r; rgb[1] = g; rgb[2] = b; break;

    switch (c) {
        MAP_COLOUR(0, 255, 255, 0) //Yellow
        MAP_COLOUR(1, 160, 160, 0) //Yellow darker
        MAP_COLOUR(2, 0, 255, 0) //Green
        MAP_COLOUR(3, 0, 160, 0) //Green darker
        MAP_COLOUR(4, 0, 255, 255) //Cyan
        MAP_COLOUR(5, 0, 160, 160) //Cyan darker
        MAP_COLOUR(6, 160, 100, 0) //brown
        MAP_COLOUR(7, 110, 50, 0) //brown darker
        MAP_COLOUR(8, 0, 60, 130) //background
        default:
            rgb[0] = 0; rgb[1] = 0; rgb[2] = 0;
            break;
    }

    glColor3ub(rgb[0],rgb[1],rgb[2]);
    glBegin(GL_POINTS);
    glVertex2i(x * PIXEL_SCALE + 2, y * PIXEL_SCALE + 2);
    glEnd();
}

typedef struct AngleLUT {
    float sin[360];
    float cos[360];
} AngleLUT;
AngleLUT angleLUT;

#define ROTATION_SCALE 10.0

void movePlayer() {
    if (keys.a) {
        if (keys.move) {
            player.look--;
        } else {
            player.angle -= 4;
            if (player.angle < 0) {
                player.angle += 360;
            }
        }
    }
    if (keys.d) {
        if (keys.move) {
            player.look++;
        } else {
            player.angle += 4;
            if (player.angle > 359) {
                player.angle -= 360;
            }
        }
    }
    int dx = (int) (angleLUT.sin[player.angle] * ROTATION_SCALE);
    int dy = (int) (angleLUT.cos[player.angle] * ROTATION_SCALE);
    if (keys.w) {
        if (keys.move) {
            player.pos.z -= 4;
        } else {
            player.pos.x += dx;
            player.pos.y += dy;
        }
    }
    if (keys.s) {
        if (keys.move) {
            player.pos.z += 4;
        } else {
            player.pos.x -= dx;
            player.pos.y -= dy;
        }
    }
    if (keys.strafeRight) {
        player.pos.x += dy;
        player.pos.y -= dx;
    }
    if (keys.strafeLeft) {
        player.pos.x -= dy;
        player.pos.y += dx;
    }
}

Wall walls[30];
Sector sectors[30];

#define CLIP_BOUND 1

void clipBehindPlayer(int* x1, int* y1, int* z1, int x2, int y2, int z2) {
    float da = *y1;
    float db = y2;
    float d = da - db;
    if (d == 0) {
        d = 1;
    }
    float s = da / (da - db);
    *x1 = *x1 + s * (x2 - (*x1));
    *y1 = *y1 + s * (y2 - (*y1));
    if (*y1 == 0) {
        *y1 = 1;
    }
    *z1 = *z1 + s * (z2 - (*z1));
}

void drawWall(int x1, int x2, int bottom1, int bottom2, int top1, int top2, int colour, int sector) {
    int dyb = bottom2 - bottom1; // Y distance bottom
    int dyt = top2 - top1; // Y distance top
    int dx = x2 - x1; // X distance
    if (dx == 0) {
        dx = 1;
    }
    int xs = x1;

#define CLIP(var, upper) \
    if (var##1 < CLIP_BOUND) { \
        var##1 = CLIP_BOUND; \
    } \
    if (var##2 < CLIP_BOUND) { \
        var##2 = CLIP_BOUND; \
    } \
    if (var##1 > (upper) - CLIP_BOUND) { \
        var##1 = (upper) - CLIP_BOUND; \
    } \
    if (var##2 > (upper) - CLIP_BOUND) { \
        var##2 = (upper) - CLIP_BOUND; \
    }

    // Clip X
    CLIP(x, SW)

    for (int x = x1; x < x2; x++) {
        int y1 = dyb * (x - xs + 0.5) / dx + bottom1;
        int y2 = dyt * (x - xs + 0.5) / dx + top1;

        // Clip y
        CLIP(y, SH)

        // Surface
        if (sectors[sector].surface == 1) {
            sectors[sector].surfacePoints[x] = y1;
            continue;
        } else if (sectors[sector].surface == 2) {
            sectors[sector].surfacePoints[x] = y2;
            continue;
        } else if (sectors[sector].surface == -1) {
            for (int y = sectors[sector].surfacePoints[x]; y < y1; y++) {
                pixel(x, y, sectors[sector].colours.bottom);
            }
        } else if (sectors[sector].surface == -2) {
            for (int y = y2; y < sectors[sector].surfacePoints[x]; y++) {
                pixel(x, y, sectors[sector].colours.top);
            }
        }

        for (int y = y1; y < y2; y++) {
            pixel(x, y, colour);
        }
    }
}

int dist(int x1, int y1, int x2, int y2) {
    return (int) sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

int compareDist(Sector* s1, Sector* s2) {
    return s1->dist < s2->dist;
}

void draw() {
    int wallX[4];
    int wallY[4];
    int wallZ[4];
    float CS = angleLUT.cos[player.angle];
    float SN = angleLUT.sin[player.angle];

    qsort(sectors, SECTOR_COUNT, sizeof(Sector), (int (*)(const void *, const void *)) compareDist);

    for (int sector = 0; sector < SECTOR_COUNT; sector++) {
        sectors[sector].dist = 0;
        if (player.pos.z < sectors[sector].heightBounds.z1) {
            sectors[sector].surface = 1; // Bottom
        } else if (player.pos.z > sectors[sector].heightBounds.z2) {
            sectors[sector].surface = 2; // Top
        } else {
            sectors[sector].surface = 0; // None
        }

        for (int face = 0; face < 2; face++) {
            for (int wall = sectors[sector].walls.start; wall < sectors[sector].walls.end; wall++) {
                int x1 = walls[wall].pos1.x - player.pos.x;
                int y1 = walls[wall].pos1.y - player.pos.y;
                int x2 = walls[wall].pos2.x - player.pos.x;
                int y2 = walls[wall].pos2.y - player.pos.y;

                // Swap surface ordering, draw back first then front
                if (face == 0) {
                    int swp = x1;
                    x1 = x2;
                    x2 = swp;
                    swp = y1;
                    y1 = y2;
                    y2 = swp;
                }

                // World positions
                wallX[0] = x1 * CS - y1 * SN;
                wallX[1] = x2 * CS - y2 * SN;
                wallX[2] = wallX[0];
                wallX[3] = wallX[1];

                wallY[0] = y1 * CS + x1 * SN;
                wallY[1] = y2 * CS + x2 * SN;
                wallY[2] = wallY[0];
                wallY[3] = wallY[1];

                // Update distance
                sectors[sector].dist += dist(
                    0, 0,
                    (wallX[0] + wallX[1]) / 2,
                    (wallY[0] + wallY[1]) / 2
                );

                wallZ[0] = sectors[sector].heightBounds.z1 - player.pos.z + ((player.look * wallY[0]) / LOOK_SCALE);
                wallZ[1] = sectors[sector].heightBounds.z1 - player.pos.z + ((player.look * wallY[1]) / LOOK_SCALE);
                wallZ[2] = wallZ[0] + sectors[sector].heightBounds.z2;
                wallZ[3] = wallZ[1] + sectors[sector].heightBounds.z2;

                if (wallY[0] < 1 && wallY[1] < 1) {
                    continue;
                } else if (wallY[0] < 1) {
                    // Bottom
                    clipBehindPlayer(
                        &wallX[0], &wallY[0], &wallZ[0],
                        wallX[1], wallY[1], wallZ[1]
                    );
                    // Top
                    clipBehindPlayer(
                        &wallX[2], &wallY[2], &wallZ[2],
                        wallX[3], wallY[3], wallZ[3]
                    );
                }
                if (wallY[1] < 1) {
                    // Bottom
                    clipBehindPlayer(
                        &wallX[1], &wallY[1], &wallZ[1],
                        wallX[0], wallY[0], wallZ[0]
                    );
                    // Top
                    clipBehindPlayer(
                        &wallX[3], &wallY[3], &wallZ[3],
                        wallX[2], wallY[2], wallZ[2]
                    );
                }
                #define CALCULATE_SCREEN_POSITION(index) \
                    wallX[index] = wallX[index] * FOV / wallY[index] + SW2; \
                    wallY[index] = wallZ[index] * FOV / wallY[index] + SH2;

                CALCULATE_SCREEN_POSITION(0)
                CALCULATE_SCREEN_POSITION(1)
                CALCULATE_SCREEN_POSITION(2)
                CALCULATE_SCREEN_POSITION(3)
                drawWall(
                    wallX[0], wallX[1],
                    wallY[0], wallY[1], wallY[2], wallY[3],
                    walls[wall].colour,
                    sector
                );
            }
            // Average sector distance
            sectors[sector].dist /= (sectors[sector].walls.end - sectors[sector].walls.start);
            // Flip to draw next surface
            sectors[sector].surface *= -1;
        }
    }
}

typedef struct FrameCounter {
    double frame1;
    double frame2;
} FrameCounter;
FrameCounter frameCounter;

// Wait = 1000ms / n fps
#define FRAME_WAIT 50

void display(GLFWwindow* window) {
    if ((frameCounter.frame1 - frameCounter.frame2) >= FRAME_WAIT) {
        clearBackground();
        movePlayer();
        draw();

        frameCounter.frame2 = frameCounter.frame1;
        glfwSwapBuffers(window);
        glfwSetWindowSize(window, GLSW, GLSH);
    }

    frameCounter.frame1 = glfwGetTime() * 1000;
    glfwPollEvents();
}

//int loadSectors[] = {
//    // Wall start, wall end, z1 height, z2 height, bottom colour, top colour
//    0, 4, 0, 40, 2, 3,
//    4, 8, 0, 40, 4, 5,
//    8, 12, 0, 40, 6, 7,
//    12, 16, 0, 40, 0, 1
//};
//
//int loadWalls[] = {
//    // x1, y1, x2, y2, colour
//    0, 0, 32, 0, 0,
//    32, 0, 32, 32, 1,
//    32, 32, 0, 32, 0,
//    0, 32, 0, 0, 1,
//
//    64, 0, 96, 0, 2,
//    96, 0, 96, 32, 3,
//    96, 32, 64, 32, 2,
//    64, 32, 64, 0, 3,
//
//    64, 64, 96, 64, 4,
//    96, 64, 96, 96, 5,
//    96, 96, 64, 96, 4,
//    64, 96, 64, 64, 5,
//
//    0, 64, 32, 64, 6,
//    32, 64, 32, 96, 7,
//    32, 96, 0, 96, 6,
//    0, 96, 0, 64, 7
//};

int loadSectors[] = {
    0,8, 40,50, 9,9,
    8,16, 0,40, 6,6,
    16,24, 80,110, 0,0,
    24,28, 0,30, 6,6,
    28,32, 0,20, 6,6,
    32,36, 0,10, 6,6,
    36,40, 0,30, 5,5,
    40,44, 0,30, 5,5,
    44,48, 30,110, 0,0,
    48,52, 30,110, 0,0,
};

int loadWalls[] = {
    160,228, 168,228, 4,
    168,228, 176,236, 5,
    176,236, 176,244, 4,
    176,244, 168,252, 5,
    168,252, 160,252, 4,
    160,252, 152,244, 5,
    152,244, 152,236, 4,
    152,236, 160,228, 5,
    104,224, 152,184, 1,
    152,184, 176,184, 3,
    176,184, 224,224, 1,
    224,224, 224,256, 0,
    224,256, 192,288, 1,
    192,288, 136,288, 0,
    136,288, 104,256, 1,
    104,256, 104,224, 0,
    104,224, 152,184, 1,
    152,184, 176,184, 0,
    176,184, 224,224, 1,
    224,224, 224,256, 0,
    224,256, 192,288, 1,
    192,288, 136,288, 0,
    136,288, 104,256, 1,
    104,256, 104,224, 0,
    152,168, 176,168, 2,
    176,168, 176,184, 3,
    176,184, 152,184, 2,
    152,184, 152,168, 3,
    152,152, 176,152, 2,
    176,152, 176,168, 3,
    176,168, 152,168, 2,
    152,168, 152,152, 3,
    152,136, 176,136, 2,
    176,136, 176,152, 3,
    176,152, 152,152, 2,
    152,152, 152,136, 3,
    208,160, 208,136, 5,
    208,136, 232,136, 4,
    232,136, 232,160, 5,
    232,160, 208,160, 4,
    96,136, 120,136, 4,
    120,136, 120,160, 5,
    120,160, 96,160, 4,
    96,160, 96,136, 5,
    216,144, 224,144, 4,
    224,144, 224,152, 5,
    224,152, 216,152, 4,
    216,152, 216,144, 5,
    104,144, 112,144, 4,
    112,144, 112,152, 5,
    112,152, 104,152, 4,
    104,152, 104,144, 5,
};

void init() {
    // Pre-calculate sin/cos to reduce overhead
    for (int a = 0; a < 360; a++) {
         angleLUT.sin[a] = sin(a / 180.0 * M_PI);
         angleLUT.cos[a] = cos(a / 180.0 * M_PI);
    }
    player = (Player) {
        .pos = {
            .x = 70,
            .y = -110,
            .z = 20
        },
        .angle = 0,
        .look = 0
    };
    int v1 = 0;
    int v2 = 0;
    for (int sector = 0; sector < SECTOR_COUNT; sector++) {
        sectors[sector].walls.start = loadSectors[v1 + 0];
        sectors[sector].walls.end = loadSectors[v1 + 1];
        sectors[sector].heightBounds.z1 = loadSectors[v1 + 2];
        sectors[sector].heightBounds.z2 = loadSectors[v1 + 3] - loadSectors[v1 + 2];
        sectors[sector].colours.top = loadSectors[v1 + 4];
        sectors[sector].colours.bottom = loadSectors[v1 + 5];
        v1 += 6;
        for (int wall = sectors[sector].walls.start; wall < sectors[sector].walls.end; wall++) {
            walls[wall].pos1.x = loadWalls[v2 + 0];
            walls[wall].pos1.y = loadWalls[v2 + 1];
            walls[wall].pos2.x = loadWalls[v2 + 2];
            walls[wall].pos2.y = loadWalls[v2 + 3];
            walls[wall].colour = loadWalls[v2 + 4];
            v2 += 5;
        }
    }
}

int main(int argc, char* argv[]) {
    GLFWwindow* window = initGL(GLSW, GLSH, "DOOM");
    init();

    while (!glfwWindowShouldClose(window)) {
        display(window);
    }

    glfwDestroyWindow(window);

    return 0;
}
