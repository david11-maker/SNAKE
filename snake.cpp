#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <deque>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <string>

using namespace std;

const int WIDTH = 800, HEIGHT = 800;
const int CELL_CONST_SIZE = 20;
const int COLS = WIDTH / CELL_CONST_SIZE;
const int ROWS = HEIGHT / CELL_CONST_SIZE;

deque<pair<int, int>> snake = {{10, 10}};
int dirX = 1, dirY = 0;

int appleX = rand() % COLS;
int appleY = rand() % ROWS;

bool paused = false;
bool gameOver = false;
int score = 0;
GLFWwindow* window;

void drawCell(int x, int y, float r, float g, float b, int CELL_SIZE) {
    float fx = (x * CELL_SIZE) / (float)WIDTH * 2 - 1;
    float fy = (y * CELL_SIZE) / (float)HEIGHT * 2 - 1;
    float sizeX = CELL_SIZE / (float)WIDTH * 2;
    float sizeY = CELL_SIZE / (float)HEIGHT * 2;

    glColor3f(r, g, b);
    glBegin(GL_QUADS);
    glVertex2f(fx, fy);
    glVertex2f(fx + sizeX, fy);
    glVertex2f(fx + sizeX, fy + sizeY);
    glVertex2f(fx, fy + sizeY);
    glEnd();
}

void drawSnake() {
    int c;
    c=0;
    for (auto& [x, y] : snake) {
        if(c==0){drawCell(x, y, 0.0f, 0.8f, 0.0f, 20); drawCell(x+2, y+2, 0.0f, 0.0f, 0.0f, 2);}
        else drawCell(x, y, 0.0f, 1.0f, 0.0f, 20);
        c++;
    }
}

void drawApple() {
    drawCell(appleX, appleY, 1.0f, 0.0f, 0.0f, 20);
}

void updateWindowTitle() {
    string title = "Snake | Score: " + to_string(score);
    if (paused) title += " (Paused)";
    glfwSetWindowTitle(window, title.c_str());
}

void moveSnake() {
    auto head = snake.front();
    int nx = (head.first + dirX + COLS) % COLS;
    int ny = (head.second + dirY + ROWS) % ROWS;

    for (auto& seg : snake) {
        if (seg.first == nx && seg.second == ny) {
            gameOver = true;
            return;
        }
    }

    snake.push_front({nx, ny});

    if (nx == appleX && ny == appleY) {
        appleX = rand() % COLS;
        appleY = rand() % ROWS;
        score += 10;
    } else {
        snake.pop_back();
    }
}

void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS) return;

    if (!paused && !gameOver) {
        if (key == GLFW_KEY_UP && dirY != -1)    { dirX = 0; dirY = 1; }
        if (key == GLFW_KEY_DOWN && dirY != 1)   { dirX = 0; dirY = -1; }
        if (key == GLFW_KEY_LEFT && dirX != 1)   { dirX = -1; dirY = 0; }
        if (key == GLFW_KEY_RIGHT && dirX != -1) { dirX = 1; dirY = 0; }
    }

    if (key == GLFW_KEY_SPACE) {
        paused = !paused;
        updateWindowTitle();
    }

    if (key == GLFW_KEY_R && gameOver) {
        snake = {{10, 10}};
        dirX = 1; dirY = 0;
        score = 0;
        gameOver = false;
        paused = false;
        appleX = rand() % COLS;
        appleY = rand() % ROWS;
        updateWindowTitle();
    }
}

int main() {
    srand(static_cast<unsigned>(time(NULL)));

    if (!glfwInit()) {
        cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    window = glfwCreateWindow(WIDTH, HEIGHT, "Snake", NULL, NULL);
    if (!window) {
        cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    glewInit();

    glfwSetKeyCallback(window, key_callback);
    updateWindowTitle();

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        drawApple();
        drawSnake();

        glfwSwapBuffers(window);
        glfwPollEvents();

        if (!paused && !gameOver) {
            moveSnake();
        }

        updateWindowTitle();
        this_thread::sleep_for(chrono::milliseconds(120));
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
