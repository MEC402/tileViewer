#version 330 core

//uniform sampler2D image;

in vec2 f_UV;
in vec3 f_normal;
in vec3 f_color;
in vec3 f_pos;

out vec4 outColor;

void main()
{
	vec2 sampleCoords = vec2(f_UV.x, 1-f_UV.y);
	//outColor = texture(image, sampleCoords).rgba + color;

	vec3 norm = normalize(f_normal);
	vec3 lightDir = normalize(vec3(1,0,1) - f_pos);

	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * f_color;

	float ambientStrength = 1.0;
	vec3 ambient = ambientStrength * f_color;

	vec3 result = (ambient + diffuse) * vec3(1,1,1);
	outColor = vec4(result, 1.0);
}