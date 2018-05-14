#version 330 core

in vec3 fColor;
in vec2 txCoord;
out vec4 outColor;

uniform sampler2D txUniform;

void main() 
{
	//outColor = vec4(fColor, 1.0);
	outColor = vec4(fColor, 1.0) * texture(txUniform, txCoord);

}