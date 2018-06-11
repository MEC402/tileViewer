#version 330 core

uniform sampler2D image;

// Interpolated values from the vertex shaders
in vec2 fragmentUV;

// Ouput data
out vec3 color;

void main()
{
	vec2 sampleCoords = vec2(fragmentUV.x, -fragmentUV.y);
	color = texture(image, sampleCoords).rgb;
}