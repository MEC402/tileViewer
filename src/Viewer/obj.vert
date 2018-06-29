#version 330 core

layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 color;
layout(location = 3) in vec2 UV;

out vec3 f_color;
out vec3 f_normal;
out vec2 f_UV;
out vec3 f_pos;

uniform mat4 MVP;
uniform mat4 Model;

void main(){	
	gl_Position =  MVP * vec4(vertPos, 1.0);
	f_UV = UV;
	f_color = color;
	f_normal = normal;
	f_pos = vec3(Model * vec4(vertPos, 1.0));
}

