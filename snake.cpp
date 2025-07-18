#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <deque>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <string>
#include <fstream>
#include <sstream>
#define mix(a,b,t) ((b)*(t)+(a)*(1-(t)))

using namespace std;

// --------------------------------------------------------------- FIXED SHADER HANDLING ---------------------------------------------------------------
GLfloat Vert[] = {
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f
};
unsigned int indices[] {
    0, 1, 2,
    0, 2, 3
};

GLuint loadShader(const char* filepath, GLenum shaderType) {
    std::ifstream shaderFile(filepath);
    if (!shaderFile.is_open()) {
        std::cerr << "ERROR: Could not open shader file: " << filepath << std::endl;
        return 0;
    }

    std::stringstream shaderStream;
    shaderStream << shaderFile.rdbuf();
    std::string shaderCode = shaderStream.str();
    const char* shaderSource = shaderCode.c_str();

    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &shaderSource, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader Compilation Failed (" << filepath << "):\n" << infoLog << std::endl;
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

GLuint createShaderProgram(const char* vertexPath, const char* fragmentPath) {
    GLuint vertexShader = loadShader(vertexPath, GL_VERTEX_SHADER);
    if (!vertexShader) return 0;

    GLuint fragmentShader = loadShader(fragmentPath, GL_FRAGMENT_SHADER);
    if (!fragmentShader) {
        glDeleteShader(vertexShader);
        return 0;
    }

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader Program Linking Failed:\n" << infoLog << std::endl;
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteProgram(shaderProgram);
        return 0;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

GLuint VBO, VAO, EBO;
GLuint mainShader;
void setup() {
    glDisable(GL_DEPTH_TEST);
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vert), Vert, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    mainShader = createShaderProgram("main.vert", "main.frag");
    if (!mainShader) {
        std::cerr << "Failed to create shader program. Using fallback rendering." << std::endl;
    }
}

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

// Fixed color handling (use float colors directly)
void drawCell(int x, int y, float r, float g, float b, int CELL_SIZE, int c, bool issnake = true) {
    float fx = (x * CELL_SIZE) / (float)WIDTH * 2 - 1;
    float fy = (y * CELL_SIZE) / (float)HEIGHT * 2 - 1;
    float sizeX = CELL_SIZE / (float)WIDTH * 2;
    float sizeY = CELL_SIZE / (float)HEIGHT * 2;

    Vert[0] = fx;            Vert[1] = fy;
    Vert[2] = fx + sizeX;    Vert[3] = fy;
    Vert[4] = fx + sizeX;    Vert[5] = fy + sizeY;
    Vert[6] = fx;            Vert[7] = fy + sizeY;

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vert), Vert);

    if (mainShader) {
        glUseProgram(mainShader);
        GLuint colorAdr = glGetUniformLocation(mainShader, "mainColor");
        if (colorAdr != -1) glUniform3f(colorAdr, r, g, b);

        GLuint posAdr = glGetUniformLocation(mainShader, "cellPos");
        if (posAdr != -1) glUniform2i(posAdr, x, y);

        GLuint snakeLenAdr = glGetUniformLocation(mainShader, "snakeLength");
        if (snakeLenAdr != -1) glUniform1i(snakeLenAdr, snake.size());

        GLuint snakeAdr = glGetUniformLocation(mainShader, "isSnake");
        if (snakeAdr != -1) glUniform1i(snakeAdr, issnake);

        GLuint currAdr = glGetUniformLocation(mainShader, "currentIndex");
        if (currAdr != -1) glUniform1i(currAdr, c);
    } else {
        // Fallback rendering if shaders fail
        glUseProgram(0);
        glColor3f(r, g, b);
    }

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

// Apple placement collision check
bool isOnSnake(int x, int y) {
    for (const auto& seg : snake) {
        if (seg.first == x && seg.second == y) {
            return true;
        }
    }
    return false;
}

void placeApple() {
    do {
        appleX = rand() % COLS;
        appleY = rand() % ROWS;
    } while (isOnSnake(appleX, appleY));
}

void drawSnake() {
    int c = 0;
    for (auto& [x, y] : snake) {
        const float l = (snake.size()>1) ? (float(c)/(snake.size()-1)) : 0;
        const float dl = 1-((1-l)*(1-l));

        if (c == 0) {
            drawCell(x, y, 0.0f, 0.8f, 0.0f, 20, c); // Head
        } else {
            float green = mix(1.0f, 0.5f, dl);
            drawCell(x, y, 0.0f, green, 0.0f, 20, c); // Body
        }
        c++;
    }
}

void drawApple() {
    drawCell(appleX, appleY, 1.0f, 0.0f, 0.0f, 20, 0, false);
}

void updateWindowTitle() {
    string title = "Snake | Score: " + to_string(score);
    if (gameOver) {
        title += " | GAME OVER (Press R to restart)";
    } else if (paused) {
        title += " | PAUSED";
    }
    glfwSetWindowTitle(window, title.c_str());
}

void moveSnake() {
    auto head = snake.front();
    int nx = (head.first + dirX + COLS) % COLS;
    int ny = (head.second + dirY + ROWS) % ROWS;

    // Check self-collision (skip head)
    auto it = snake.begin();
    it++; // Skip head
    for (; it != snake.end(); it++) {
        if (it->first == nx && it->second == ny) {
            gameOver = true;
            return;
        }
    }

    snake.push_front({nx, ny});

    if (nx == appleX && ny == appleY) {
        placeApple();
        score += 10;
    } else {
        snake.pop_back();
    }
}

void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS) return;

    if (!paused && !gameOver) {
        if (key == GLFW_KEY_DOWN && dirY != 1)    { dirX = 0; dirY = -1; }
        else if (key == GLFW_KEY_UP && dirY != -1) { dirX = 0; dirY = 1; }
        else if (key == GLFW_KEY_LEFT && dirX != 1)  { dirX = -1; dirY = 0; }
        else if (key == GLFW_KEY_RIGHT && dirX != -1) { dirX = 1; dirY = 0; }
    }

    if (key == GLFW_KEY_SPACE) {
        paused = !paused;
    }

    if (key == GLFW_KEY_R && gameOver) {
        snake = {{10, 10}};
        dirX = 1; dirY = 0;
        score = 0;
        gameOver = false;
        paused = false;
        placeApple();
    }
    updateWindowTitle();
}

int main() {
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
    if (glewInit() != GLEW_OK) {
        cerr << "Failed to initialize GLEW\n";
        glfwTerminate();
        return -1;
    }

    setup();
    srand(static_cast<unsigned>(time(NULL)));
    placeApple(); // Ensure valid initial apple position

    glfwSetKeyCallback(window, key_callback);
    updateWindowTitle();

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1f, 0.25f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        drawApple();
        drawSnake();

        glfwSwapBuffers(window);
        glfwPollEvents();

        if (!paused && !gameOver) {
            moveSnake();
            updateWindowTitle();
            this_thread::sleep_for(chrono::milliseconds(150));
        }
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(mainShader);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
