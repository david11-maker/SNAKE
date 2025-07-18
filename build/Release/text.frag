#version 460 core

in vec2 TexCoord;
uniform sampler2D atlas;
out vec4 fragColor;

void main() {
    vec3 c = texture(atlas, TexCoord).rgb;
    // apply gamma correction so your font looks crisp
    fragColor = vec4(0.5f,0.5f,0.5f,1.0f);
}
