#version 330 core

in vec3 pos;
in vec3 offset;
//in vec2 txStart;
//in vec2 txEnd;
in vec3 color;

out VOUT {
	vec3 vColor;
	//vec2 vTxStart;
	//vec2 vTxEnd;
	vec3 vOffset;
} vout;

//out vec3 vColor;
//out vec3 vOffset;

void main() 
{
	
	//vec4 newPos = vec4(pos, 1.0) * rotMat;
	//vec4 newPos = MVP * (vec4(pos, 1.0) * rotMat);
	//vec4 newOffset = vec4(offset, 1.0) * rotMat; 
	//vec4 newOffset = Scaling * (vec4(offset, 1.0) * rotMat); 

	gl_Position = vec4(pos, 1.0);
	vout.vColor = color;
	//vout.vTxStart = txStart;
	//vout.vTxEnd = txEnd;
	vout.vOffset = offset;

	//vColor = color;
	//vOffset = offset;
}