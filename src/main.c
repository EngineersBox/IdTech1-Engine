#define _USE_MATH_DEFINES
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "player/player.h"
#include "world/wall.h"
#include "world/sector.h"

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
#define SECTOR_COUNT 4
#define WALL_COUNT 16

#define RAW_MOUSE_INPUT

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
    if(c==0){ rgb[0]=255; rgb[1]=255; rgb[2]=  0;} //Yellow
    if(c==1){ rgb[0]=160; rgb[1]=160; rgb[2]=  0;} //Yellow darker
    if(c==2){ rgb[0]=  0; rgb[1]=255; rgb[2]=  0;} //Green
    if(c==3){ rgb[0]=  0; rgb[1]=160; rgb[2]=  0;} //Green darker
    if(c==4){ rgb[0]=  0; rgb[1]=255; rgb[2]=255;} //Cyan
    if(c==5){ rgb[0]=  0; rgb[1]=160; rgb[2]=160;} //Cyan darker
    if(c==6){ rgb[0]=160; rgb[1]=100; rgb[2]=  0;} //brown
    if(c==7){ rgb[0]=110; rgb[1]= 50; rgb[2]=  0;} //brown darker
    if(c==8){ rgb[0]=  0; rgb[1]= 60; rgb[2]=130;} //background
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
    int dx = angleLUT.sin[player.angle] * ROTATION_SCALE;
    int dy = angleLUT.cos[player.angle] * ROTATION_SCALE;
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

#define FOV 200
#define LOOK_SCALE 32.0
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

void drawWall(int x1, int x2, int bottom1, int bottom2, int top1, int top2, int colour) {
    int dyb = bottom2 - bottom1; // Y distance bottom
    int dyt = top2 - top1; // Y distance top
    int dx = x2 - x1; // X distance
    if (dx == 0) {
        dx = 1;
    }
    int xs = x1;

    // Clip x
    if (x1 < CLIP_BOUND) {
        x1 = CLIP_BOUND; // Left
    }
    if (x2 < CLIP_BOUND) {
        x2 = CLIP_BOUND; // Left
    }
    if (x1 > SW - CLIP_BOUND) {
        x1 = SW - CLIP_BOUND; // Right
    }
    if (x2 > SW - CLIP_BOUND) {
        x2 = SW - CLIP_BOUND; // Right
    }

    for (int x = x1; x < x2; x++) {
        int y1 = dyb * (x - xs + 0.5) / dx + bottom1;
        int y2 = dyt * (x - xs + 0.5) / dx + top1;

        // Clip y
        if (y1 < CLIP_BOUND) {
            y1 = CLIP_BOUND; // Bottom
        }
        if (y2 < CLIP_BOUND) {
            y2 = CLIP_BOUND; // Bottom
        }
        if (y1 > SH - CLIP_BOUND) {
            y1 = SH - CLIP_BOUND; // Top
        }
        if (y2 > SH - CLIP_BOUND) {
            y2 = SH - CLIP_BOUND; // Top
        }

        for (int y = y1; y < y2; y++) {
            pixel(x, y, colour);
        }
    }
}

int dist(int x1, int y1, int x2, int y2) {
    return (int) sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

void draw() {
    int wallX[4];
    int wallY[4];
    int wallZ[4];
    float CS = angleLUT.cos[player.angle];
    float SN = angleLUT.sin[player.angle];

    for (int sector = 0; sector < SECTOR_COUNT; sector++) {
        for (int wall = 0; wall < SECTOR_COUNT - sector - 1; wall++) {
            if (sectors[wall].dist < sectors[wall + 1].dist) {
                Sector st = sectors[wall];
                sectors[wall] = sectors[wall + 1];
                sectors[wall + 1] = st;
            }
        }
    }

    for (int sector = 0; sector < SECTOR_COUNT; sector++) {
        sectors[sector].dist = 0;
        for (int wall = sectors[sector].walls.start; wall < sectors[sector].walls.end; wall++) {
            int x1 = walls[wall].pos1.x - player.pos.x;
            int y1 = walls[wall].pos1.y - player.pos.y;
            int x2 = walls[wall].pos2.x - player.pos.x;
            int y2 = walls[wall].pos2.y - player.pos.y;

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
                    wallZ[1], wallY[1], wallZ[1]
                );
                // Top
                clipBehindPlayer(
                    &wallX[2], &wallY[2], &wallZ[2],
                    wallZ[3], wallY[3], wallZ[3]
                );
            }
            if (wallY[1] < 1) {
                // Bottom
                clipBehindPlayer(
                    &wallX[1], &wallY[1], &wallZ[1],
                    wallZ[0], wallY[0], wallZ[0]
                );
                // Top
                clipBehindPlayer(
                    &wallX[3], &wallY[3], &wallZ[3],
                    wallZ[2], wallY[2], wallZ[2]
                );
            }

            // Screen positions
            wallX[0] = wallX[0] * FOV / wallY[0] + SW2;
            wallY[0] = wallZ[0] * FOV / wallY[0] + SH2;
            wallX[1] = wallX[1] * FOV / wallY[1] + SW2;
            wallY[1] = wallZ[1] * FOV / wallY[1] + SH2;
            wallX[2] = wallX[2] * FOV / wallY[2] + SW2;
            wallY[2] = wallZ[2] * FOV / wallY[2] + SH2;
            wallX[3] = wallX[3] * FOV / wallY[3] + SW2;
            wallY[3] = wallZ[3] * FOV / wallY[3] + SH2;
            drawWall(wallX[0], wallX[1], wallY[0], wallY[1], wallY[2], wallY[3], walls[wall].colour);
        }
        // Average sector distance
        sectors[sector].dist /= (sectors[sector].walls.end - sectors[sector].walls.start);
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

int loadSectors[] = {
    // Wall start, wall end, z1 height, z2 height
    0, 4, 0, 40,
    4, 8, 0, 40,
    8, 12, 0, 40,
    12, 16, 0, 40
};

int loadWalls[] = {
    // x1, y1, x2, y2, colour
    0, 0, 32, 0, 0,
    32, 0, 32, 32, 1,
    32, 32, 0, 32, 0,
    0, 32, 0, 0, 1,

    64, 0, 96, 0, 2,
    96, 0, 96, 32, 3,
    96, 32, 64, 32, 2,
    64, 32, 64, 0, 3,

    64, 64, 96, 64, 4,
    96, 64, 96, 96, 5,
    96, 96, 64, 96, 4,
    64, 96, 64, 64, 5,

    0, 64, 32, 64, 6,
    32, 64, 32, 96, 7,
    32, 96, 0, 96, 6,
    0, 96, 0, 64, 7
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
        v1 += 4;
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