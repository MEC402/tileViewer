#version 330 core

in vec3 pos;
in vec3 offset;
in vec3 color;
in float face;

out VOUT {
	vec3 vColor;
	vec3 vOffset;
	float vFace;
} vout;

void main() 
{
	gl_Position = vec4(pos, 1.0);
	vout.vColor = color;
	vout.vOffset = offset;
	vout.vFace = face;
}