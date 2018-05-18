#version 330
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in VertexData {
    vec2 halfDimensions;
    vec2 texturePosition;
    vec2 textureDimensions;
    vec2 rotationAxis;
    float rotationAngle;
    vec4 color;
} inData[];

out vec2 uv;
out vec4 color;

uniform mat4 perspective;

vec2 rotate(vec2 p, float angle, vec2 rp) {
    float s = sin(angle);
    float c = cos(angle);
    // translate point back to origin:
    p.x -= rp.x;
    p.y -= rp.y;
    // rotate point
    float xnew = p.x * c - p.y * s;
    float ynew = p.x * s + p.y * c;
    // translate point back:
    p.x = xnew + rp.x;
    p.y = ynew + rp.y;
    return p;
}

vec2 corner;

void main() {
    for (int i = 0; i < gl_in.length(); ++i) {
        
        color = inData[i].color;
        corner.x = gl_in[i].gl_Position.x - inData[i].halfDimensions.x;
        corner.y = gl_in[i].gl_Position.y - inData[i].halfDimensions.y;
        gl_Position = perspective * vec4(
            rotate(corner, inData[i].rotationAngle, inData[i].rotationAxis),
            gl_in[i].gl_Position.z,
            1.0
        );
        uv.x = inData[i].texturePosition.x;
        uv.y = inData[i].texturePosition.y + inData[i].textureDimensions.y;
        EmitVertex();
        
        color = inData[i].color;
        corner.x = gl_in[i].gl_Position.x - inData[i].halfDimensions.x;
        corner.y = gl_in[i].gl_Position.y + inData[i].halfDimensions.y;
        gl_Position = perspective * vec4(
            rotate(corner, inData[i].rotationAngle, inData[i].rotationAxis),
            gl_in[i].gl_Position.z,
            1.0
        );
        uv = inData[i].texturePosition;
        EmitVertex();
        
        color = inData[i].color;
        corner.x = gl_in[i].gl_Position.x + inData[i].halfDimensions.x;
        corner.y = gl_in[i].gl_Position.y - inData[i].halfDimensions.y;
        gl_Position = perspective * vec4(
            rotate(corner, inData[i].rotationAngle, inData[i].rotationAxis),
            gl_in[i].gl_Position.z,
            1.0
        );
        uv.x = inData[i].texturePosition.x + inData[i].textureDimensions.x;
        uv.y = inData[i].texturePosition.y + inData[i].textureDimensions.y;
        EmitVertex();

        color = inData[i].color;
        corner.x = gl_in[i].gl_Position.x + inData[i].halfDimensions.x;
        corner.y = gl_in[i].gl_Position.y + inData[i].halfDimensions.y;
        gl_Position = perspective * vec4(
            rotate(corner, inData[i].rotationAngle, inData[i].rotationAxis),
            gl_in[i].gl_Position.z,
            1.0
        );
        uv.x = inData[i].texturePosition.x + inData[i].textureDimensions.x;
        uv.y = inData[i].texturePosition.y;
        EmitVertex();
    }
}