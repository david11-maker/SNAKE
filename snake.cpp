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
#include <queue>

#define mix(a,b,t) ((b)*(t)+(a)*(1-(t)))

using namespace std;

bool canTurn = true;
queue<uint8_t> moves = {};

int dirXLoc=1, dirYLoc=0;

// --------------------------------------------------------------- FIXED SHADER HANDLING ---------------------------------------------------------------
GLfloat Vert[] = {
    0.0f, 0.0f,  0.0f, 0.0f, 
    1.0f, 0.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 1.0f, 1.0f,
    0.0f, 1.0f, 0.0f, 1.0f
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
GLuint colorShader, gradientShader, textShader, fontAtlasTex;
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
    
    // Position attribute (2 floats)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Texture coordinate attribute (2 floats)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), (void*)(2*sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
    
    glBindVertexArray(0);
    
    colorShader    = createShaderProgram("plain.vert",  "plain.frag");
    gradientShader = createShaderProgram("main.vert",   "main.frag");
    textShader     = createShaderProgram("text.vert",   "text.frag");
}


const int WIDTH = 800, HEIGHT = 800;
const int CELL_CONST_SIZE = 20;
const int COLS = WIDTH / CELL_CONST_SIZE;
const int ROWS = HEIGHT / CELL_CONST_SIZE;

deque<pair<int, int>> snake = {{11,10},{10, 10},{9,10}};
int dirX = 1, dirY = 0;

int appleX = rand() % COLS;
int appleY = rand() % ROWS;

bool paused = false;
bool gameOver = false;
int score = 0;
GLFWwindow* window;
/*--------------------------------------------------------Text writing----------------------------------------------------------*/

pair<pair<float,float>,pair<float,float>> getCharUv(char c) {
    const int atlasWidth  = 128;  // például 128 px széles
    const int atlasHeight = 64;  // és 128 px magas
    const int cellW       = 8;    // cella szélessége
    const int cellH       = 8;    // cella magassága
    const int cols        = 16;   // egy sorban ennyi cella
    const int firstChar   = 65;   // ASCII kód az első cellához

    int index = int(c) - firstChar;
    if (index < 0) index = 0;  // határkezelés
    // oszlop és sor (0‑tól indulnak)
    int col = index % cols;
    int row = index / cols;
    
    // OpenGL UV‑ben az (0,0) a textúra bal‑alsó sarka, a BMP‑dől az első cella viszont
    // a bal‑felsőben van, ezért a V‑t meg kell fordítani:
    float u1 = (col * cellW) / float(atlasWidth);
    float v2 = 1.0f - (row * cellH) / float(atlasHeight);
    float u2 = ((col+1) * cellW) / float(atlasWidth);
    float v1 = 1.0f - ((row+1) * cellH) / float(atlasHeight);
    
    // Visszaadjuk: (u1,v1) az alsó‑bal sarok, (u2,v2) a felső‑jobb
    return { {u1, v1}, {u2, v2} };
}    
void drawCell(int x, int y, float r, float g, float b, int CELL_SIZE, int c, bool issnake = true) {
    
    float fx = (x * CELL_SIZE) / (float)WIDTH * 2 - 1;
    float fy = (y * CELL_SIZE) / (float)HEIGHT * 2 - 1;
    float sizeX = CELL_SIZE / (float)WIDTH * 2;
    float sizeY = CELL_SIZE / (float)HEIGHT * 2;
    
    // Update all 4 vertices (position + UV)
    Vert[0] = fx;         Vert[1] = fy;         // Vertex 0 position
    Vert[2] = 0.0f;       Vert[3] = 0.0f;       // Vertex 0 UV
    
    Vert[4] = fx+sizeX;   Vert[5] = fy;         // Vertex 1 position
    Vert[6] = 1.0f;       Vert[7] = 0.0f;       // Vertex 1 UV
    
    Vert[8] = fx+sizeX;   Vert[9] = fy+sizeY;   // Vertex 2 position
    Vert[10] = 1.0f;      Vert[11] = 1.0f;      // Vertex 2 UV
    
    Vert[12] = fx;        Vert[13] = fy+sizeY;   // Vertex 3 position
    Vert[14] = 0.0f;      Vert[15] = 1.0f;      // Vertex 3 UV

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vert), Vert);


    if (issnake) {
        glUseProgram(gradientShader);
        glUniform1i(glGetUniformLocation(gradientShader, "isSnake"), 1);
        glUniform1i(glGetUniformLocation(gradientShader, "currentIndex"), c);
        glUniform1i(glGetUniformLocation(gradientShader, "snakeLength"), snake.size());
        glUniform3f(glGetUniformLocation(gradientShader, "mainColor"), r, g, b);
    } else {
        glUseProgram(colorShader);
        glUniform3f(glGetUniformLocation(colorShader, "mainColor"), r, g, b);
    }
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}    

void drawCell(float x, float y, float x2, float y2, float r=0.5f, float g=0.5f, float b=0.5f, string texture="") {
    Vert[0]=x;   Vert[1]=y;
    Vert[2]=0;   Vert[3]=0;       // uv resets
    Vert[4]=x2;  Vert[5]=y;
    Vert[6]=0;   Vert[7]=0;
    Vert[8]=x2;  Vert[9]=y2;
    Vert[10]=0;  Vert[11]=0;
    Vert[12]=x;  Vert[13]=y2;
    Vert[14]=0;  Vert[15]=0;
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vert), Vert);

    glUseProgram(colorShader);
    glUniform3f(glGetUniformLocation(colorShader, "mainColor"), 0.5f,0.5f,0.5f);
    
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}    

void drawCell(float x, float y, float x2, float y2, char c) {
    // 1) position coords
    Vert[0]  = x;   Vert[1]  = y;
    Vert[4]  = x2;  Vert[5]  = y;
    Vert[8]  = x2;  Vert[9]  = y2;
    Vert[12] = x;   Vert[13] = y2;

    // 2) UV coords
    auto uv = getCharUv(c);
    Vert[2]  = uv.first.first;  Vert[3]  = uv.first.second;
    Vert[6]  = uv.second.first; Vert[7]  = uv.first.second;
    Vert[10] = uv.second.first; Vert[11] = uv.second.second;
    Vert[14] = uv.first.first;  Vert[15] = uv.second.second;

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vert), Vert);

    glUseProgram(textShader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fontAtlasTex);
    glUniform1i(glGetUniformLocation(textShader, "atlas"), 0);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}


void drawCell(float x, float y, float x2, float y2, string texture) {
    drawCell(x, y, x2, y2, 0.5f, 0.5f, 0.5f, texture);
}    

GLuint loadTexture(const char* filename) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    ifstream file(string(filename) + ".bmp", ios::binary);
    stringstream ss; ss << file.rdbuf();
    string str = ss.str();
    const char* buffer = str.c_str();

    uint32_t offset, width;
    int32_t height;
    memcpy(&offset, buffer + 10, 4);
    memcpy(&width,  buffer + 18, 4);
    memcpy(&height, &buffer[22], 4);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
                GL_RGB, GL_UNSIGNED_BYTE, buffer + offset);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    return textureID;
}

void write(string text, float size, float posX, float posY) {
    for(int i=0; i<text.size(); i++) {
        drawCell(posX+(i*(size*1.2)),posY,posX+size+(i*(size*1.2)),posY+size, text[i]);
    }    
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
    if(moves.size()) {
        switch(moves.front()) {
        case 0:
            dirXLoc = 0; dirYLoc = -1;  
        break;
        case 1:
			dirXLoc = 0; dirYLoc = 1;
        break;
        case 2:
            dirXLoc = -1; dirYLoc = 0;
        break;
        case 3:
            dirXLoc = 1; dirYLoc = 0;
        break;
	}
	moves.pop();
    }

    auto head = snake.front();
    int nx = (head.first + dirXLoc + COLS) % COLS;
    int ny = (head.second + dirYLoc + ROWS) % ROWS;

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
        if (key == GLFW_KEY_DOWN && dirY != 1)    { moves.push(0);  dirX = 0; dirY = -1; }
        else if (key == GLFW_KEY_UP && dirY != -1) { moves.push(1); dirX = 0; dirY = 1; }
        else if (key == GLFW_KEY_LEFT && dirX != 1)  { moves.push(2); dirX = -1; dirY = 0; }
        else if (key == GLFW_KEY_RIGHT && dirX != -1) { moves.push(3); dirX = 1; dirY = 0; }
    }

    if (key == GLFW_KEY_SPACE) {
        paused = !paused;
    }

    if (key == GLFW_KEY_R && gameOver) {
        snake = {{11,10},{10, 10},{9,10}};
        dirX = 1; dirY = 0;
        dirXLoc = 1; dirYLoc = 0;
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

    fontAtlasTex = loadTexture("fontatlas");  // instead of passing a shader handle


    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1f, 0.25f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        drawApple();
        drawSnake();

        glfwSwapBuffers(window);
        glfwPollEvents();

        if (!paused && !gameOver) {
            string txt = "NUMS" /* we don't have numbers... to_string(score)*/;
            write(txt, 0.1f,-0.8f,0.8f);
            moveSnake();
            updateWindowTitle();
            this_thread::sleep_for(chrono::milliseconds(150));
        }
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
