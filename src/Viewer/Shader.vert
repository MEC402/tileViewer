#version 330 core

in vec3 pos;
in vec3 color;
in vec3 offset;

out vec3 vColor;
out vec3 vOffset;

void main() 
{
	
	//vec4 newPos = vec4(pos, 1.0) * rotMat;
	//vec4 newPos = MVP * (vec4(pos, 1.0) * rotMat);
	//vec4 newOffset = vec4(offset, 1.0) * rotMat; 
	//vec4 newOffset = Scaling * (vec4(offset, 1.0) * rotMat); 
	gl_Position = vec4(pos, 1.0);
	vColor = color;
	vOffset = offset;
}