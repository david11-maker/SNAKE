#version 460 core

out vec4 fragColor;

uniform vec3 mainColor;

void main() {
    fragColor = vec4(mainColor,1);
}
