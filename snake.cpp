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
/*
W.I.P.
// --------------------------------------------------------------- TRYING TO MAKE SHADERS WORK ---------------------------------------------------------------
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
        std::cerr << "Shader Compilation Failed:\n" << infoLog << std::endl;
    }

    return shader;
}

GLuint createShaderProgram(const char* vertexPath, const char* fragmentPath) {
    // Load shaders
    GLuint vertexShader = loadShader(vertexPath, GL_VERTEX_SHADER);
    GLuint fragmentShader = loadShader(fragmentPath, GL_FRAGMENT_SHADER);

    // Create shader program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Check for linking errors
    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader Program Linking Failed:\n" << infoLog << std::endl;
    }

    // Delete shaders as they're already linked
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
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,2*sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    mainShader = createShaderProgram("main.vert","main.frag");
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------------
*/

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
/*
void drawCell(int x, int y, int r, int g, int b, int CELL_SIZE, int c, bool issnake = true) {
    float fx = (x * CELL_SIZE) / (float)WIDTH * 2 - 1;
    float fy = (y * CELL_SIZE) / (float)HEIGHT * 2 - 1;
    float sizeX = CELL_SIZE / (float)WIDTH * 2;
    float sizeY = CELL_SIZE / (float)HEIGHT * 2;

    Vert[0] = fx;            Vert[1] = fy;
    Vert[2] = fx + sizeX;     Vert[3] = fy;
    Vert[4] = fx + sizeX;     Vert[5] = fy + sizeY;
    Vert[6] = fx;            Vert[7] = fy + sizeY;

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vert), Vert);

    glUseProgram(mainShader);

    GLuint colorAdr = glGetUniformLocation(mainShader,"mainColor");
    glUniform3f(colorAdr,float(r)/255,float(g)/255,float(b)/255);
    GLuint posAdr = glGetUniformLocation(mainShader,"cellPos");
    glUniform2i(posAdr,x,y);
    GLuint snakeLenAdr = glGetUniformLocation(mainShader,"snakeLength");
    glUniform1i(snakeLenAdr,snake.size());
    GLuint snakeAdr = glGetUniformLocation(mainShader,"isSnake");
    glUniform1i(snakeAdr,issnake);
    GLuint currAdr = glGetUniformLocation(mainShader,"currentIndex");
    glUniform1i(currAdr,c);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,0);

    /*glColor3f(r, g, b);
    glBegin(GL_QUADS);
    glVertex2f(fx, fy);
    glVertex2f(fx + sizeX, fy);
    glVertex2f(fx + sizeX, fy + sizeY);
    glVertex2f(fx, fy + sizeY);
    glEnd();*//*
}
*/

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

void defeat() {
    //cout<<"TEMP_DEFEAT";
}

void drawSnake() {
    int c;
    c=0;
    for (auto& [x, y] : snake) {
        const float l = (snake.size()>1?(float(c)/(snake.size()-1)):0);
        const float dl = 1-((1-l)*(1-l));
        //if(c==0){drawCell(x, y, 0.5f, 0.0f, 0.5f, 20); drawCell(x+2, y+2, 0.0f, 0.0f, 0.0f, 2);}
        //else drawCell(x, y, mix(0.7f,0.0f,dl), 0.0f, mix(0.7f,0.0f,dl), 20);
        if(c==0){drawCell(x, y, 0.0f, 0.8f, 0.0f, 20); /*drawCell(x+2, y+2, 0.0f, 0.0f, 0.0f, 2);*/}
        else drawCell(x, y, 0.0f, mix(1.0f,0.5f,dl), 0.0f, 20);
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
            defeat();
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
    //setup();
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
        glClearColor(0.1f, 0.25f, 0.15f, 1.0f);
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
