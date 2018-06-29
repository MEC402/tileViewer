#version 330 core

uniform sampler2D image;

// Interpolated values from the vertex shaders
in vec2 fragmentUV;

// Ouput data
out vec4 outColor;

void main()
{
	vec2 sampleCoords = vec2(fragmentUV.x, 1-fragmentUV.y);
	outColor = texture(image, sampleCoords).rgba;
}