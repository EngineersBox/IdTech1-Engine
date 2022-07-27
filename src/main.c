#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "player/player.h"

#define WINDOW_WIDTH      160                       // Raw screen height
#define WINDOW_HEIGHT     120                       // Raw screen width
#define res               1                         // 0=160x120 1=360x240 4=640x480
#define SW                WINDOW_WIDTH*res          // screen width
#define SH                WINDOW_HEIGHT*res         // screen height
#define SW2               (SW/2)                    // half of screen width
#define SH2               (SH/2)                    // half of screen height
#define pixelScale        4/res                     // OpenGL pixel scale
#define GLSW              (SW*pixelScale)           // OpenGL window width
#define GLSH              (SH*pixelScale)           // OpenGL window height
#define Z_NEAR            0                         // Near clipping plane distance
#define Z_FAR             1000                      // Far clipping plane distance
#define BACKGROUND_COLOUR 0.07f, 0.13f, 0.17f, 1.0f // Clear colour

typedef struct Keys {
    int w, a, s, d;
    int strafeLeft, strafeRight;
    int move;
} Keys;
Keys keys;

void keyCallback(GLFWwindow* window, int key, int scanCode, int action, int modifiers) {
    #define SET_ACTION_MAPPING(field, keyCode) case keyCode: \
        keys.field = action == GLFW_PRESS; \
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

    glfwMakeContextCurrent(window);
    gladLoadGL();
    glViewport(0, 0, width, height);
    glOrtho(0, width, 0, height, Z_NEAR, Z_FAR);
    glPointSize(pixelScale);
    return window;
}

void clearBackground() {
    glClearColor(BACKGROUND_COLOUR);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void movePlayer() {

}

// Draw a pixel at x/y with rgb
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
    glVertex2i(x * pixelScale + 2, y * pixelScale + 2);
    glEnd();
}

int tick;
void draw() {
    int c = 0;
    for (int y = 0; y < SH2; y++) {
        for (int x = 0; x < SW2; x++) {
            pixel(x,y,c);
            c++;
            if (c > 8) {
                c = 0;
            }
        }
    }
    //frame rate
    tick++;
    if (tick > 20) {
        tick = 0;
    }
    pixel(SW2, SH2 + tick, 0);
}

typedef struct FrameCounter {
    double frame1;
    double frame2;
} FrameCounter;
FrameCounter frameCounter;

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

int main(int argc, char* argv[]) {
    GLFWwindow* window = initGL(GLSW, GLSH, "DOOM");

    Player player = {
        .pos = {
            .x = 70,
            .y = 0,
            .z = -70
        },
        .angle = 0,
        .look = 0
    };

    while (!glfwWindowShouldClose(window)) {
        display(window);
    }

    glfwDestroyWindow(window);

    return 0;
}