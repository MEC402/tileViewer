#version 330
in vec2 uv;
in vec4 color;

out vec4 outputColor;

uniform sampler2D baseTexture;

vec4 textureColor = texture(baseTexture, uv);

void main() {
    outputColor = (textureColor * color);
}