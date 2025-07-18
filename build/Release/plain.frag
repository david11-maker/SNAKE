#version 460 core
in vec2 TexCoord;
out vec4 FragColor;

uniform vec3 mainColor;

void main() {
    FragColor = vec4(mainColor, 1.0);
}