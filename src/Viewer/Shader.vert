#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in float face;
layout (location = 2) in float depth;

out VOUT {
	flat highp int vFace;
	flat highp int vDepth;
} vout;

void main() 
{
	gl_Position = vec4(pos, 1.0);
	vout.vFace = int(face);
	vout.vDepth = int(depth);
}