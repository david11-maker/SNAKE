#version 460 core

layout(location = 0) in vec2 vertCoord;

void main() {
    gl_Position = vec4(vertCoord,0,1);
}
