#version 330 core

in vec3 pos;
in vec3 offset;
in vec3 color;
in float face;
in float depth;

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