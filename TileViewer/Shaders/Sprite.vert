#version 330
in vec3 position;
in vec2 dimensions;
in vec2 texturePosition;
in vec2 textureDimensions;
in vec2 rotationAxis;
in float rotationAngle;
in vec4 color;

out VertexData {
    vec2 halfDimensions;
    vec2 texturePosition;
    vec2 textureDimensions;
    vec2 rotationAxis;
    float rotationAngle;
    vec4 color;
} outData;

void main() {
    gl_Position.z = position.z;
    gl_Position.xy = position.xy;
    gl_Position.w = 1.0;

    outData.halfDimensions = dimensions/2;
    outData.texturePosition = texturePosition;
    outData.textureDimensions = textureDimensions;
    outData.rotationAxis = rotationAxis;
    outData.rotationAngle = rotationAngle;
    outData.color = color;
}