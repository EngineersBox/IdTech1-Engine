#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "player/player.h"

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

#define Z_NEAR 0
#define Z_FAR 1000

#define BACKGROUND_COLOUR 0.07f, 0.13f, 0.17f, 1.0f

void keyCallback(GLFWwindow* window, int key, int scanCode, int action, int modifiers) {
    switch (key) {
        case GLFW_KEY_W:
            // TODO: Move forward
            printf("Forward\n");
            break;
        case GLFW_KEY_A:
            // TODO: Move left
            printf("Left\n");
            break;
        case GLFW_KEY_S:
            // TODO: Move backward
            printf("Backward\n");
            break;
        case GLFW_KEY_D:
            // TODO: Move right
            printf("Right\n");
            break;
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
    return window;
}

void display() {
    glClearColor(BACKGROUND_COLOUR);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

int main(int argc, char* argv[]) {
    GLFWwindow* window = initGL(WINDOW_WIDTH, WINDOW_HEIGHT, "DOOM");

    while (!glfwWindowShouldClose(window)) {
        display();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    return 0;
}