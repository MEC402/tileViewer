#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 offset;
layout (location = 2) in vec3 color;
layout (location = 3) in float face;
layout (location = 4) in float depth;

out VOUT {
	vec3 vColor;
	vec3 vOffset;
	flat highp int vFace;
	flat highp int vDepth;
} vout;

void main() 
{
	gl_Position = vec4(pos, 1.0);
	vout.vColor = color;
	vout.vOffset = offset;
	vout.vFace = int(face);
	vout.vDepth = int(depth);
}