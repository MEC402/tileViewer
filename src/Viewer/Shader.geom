#version 330 core
	
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

//in vec3 vColor[];
//in vec3 vOffset[];

in VOUT {
	vec3 vColor;
	//vec2 vTxStart;
	//vec2 vTxEnd;
	vec3 vOffset;
} vin[];

out vec2 txCoord;
out vec3 fColor;

uniform mat4 MVP;
uniform float Scaling;

void main()
{
	fColor = vin[0].vColor;
	//fColor = vec3(1.0, 1.0, 1.0);
	

	float x = vin[0].vOffset.x;
	float y = vin[0].vOffset.y;
	float z = vin[0].vOffset.z;

	//vec2 vTxStart = vin[0].vTxStart;
	//vec2 vTxEnd = vin[0].vTxEnd;


	vec4 position;
	// TODO: I'm sure there's a better way to check which coordinates to use
	if (y > 0.0) {
		position = gl_in[0].gl_Position + vec4(-x, -y, -z, 0.0);
		gl_Position = MVP * position;
		if (x > 0.0) {
			txCoord = vec2(position.x, position.y);//vTxStart;
		} else {
			txCoord = vec2(position.z, position.y);
		}
		EmitVertex(); 			 
					  			 
		position = gl_in[0].gl_Position + vec4( x, -y, z, 0.0);
		gl_Position = MVP * position;
		if (x > 0.0) {
			txCoord = vec2(position.x, position.y);//vTxStart;
		} else {
			txCoord = vec2(position.z, position.y);
		}
		EmitVertex(); 			 
					  			 
		position = gl_in[0].gl_Position + vec4(-x,  y, -z, 0.0);
		gl_Position = MVP * position;
		if (x > 0.0) {
			txCoord = vec2(position.x, position.y);//vTxStart;
		} else {
			txCoord = vec2(position.z, position.y);
		}
		EmitVertex(); 			 
								 
		position = gl_in[0].gl_Position + vec4( x,  y, z, 0.0);
		gl_Position = MVP * position;
		if (x > 0.0) {
			txCoord = vec2(position.x, position.y);//vTxStart;
		} else {
			txCoord = vec2(position.z, position.y);
		}
		EmitVertex();

	} else {
		
		position = gl_in[0].gl_Position + vec4(-x, -y, -z, 0.0);
		gl_Position = MVP * position;
		txCoord = vec2(position.x, position.z);
		EmitVertex(); 			 
					  			 
		position = gl_in[0].gl_Position + vec4( x, -y, -z, 0.0);
		gl_Position = MVP * position;
		txCoord = vec2(position.x, position.z);
		EmitVertex(); 			 
					  			 
		position = gl_in[0].gl_Position + vec4(-x,  y, z, 0.0);
		gl_Position = MVP * position;
		txCoord = vec2(position.x, position.z);
		EmitVertex(); 			 
								 
		position = gl_in[0].gl_Position + vec4( x,  y, z, 0.0);
		gl_Position = MVP * position;
		txCoord = vec2(position.x, position.z);
		EmitVertex();
	}

	EndPrimitive();
}