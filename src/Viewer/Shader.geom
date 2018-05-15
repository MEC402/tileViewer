#version 330 core
	
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in VOUT {
	vec3 vColor;
	vec3 vOffset;
} vin[];

out vec2 txCoord;
out vec3 fColor;

uniform mat4 MVP;
uniform vec2 TxScaling;

vec2 getTxCoord(vec4 pos, float x, float y)
{
	pos = (pos + vec4(1.0, 1.0, 1.0, 1.0)) / 2.0;
	if (y != 0.0) {
		if (x != 0.0) {
			return TxScaling * vec2(pos.x, pos.y);
		} else {
			return TxScaling * vec2(pos.z, pos.y);
		}
	} else {
		return TxScaling * vec2(pos.x, pos.z);
	}
}

void main()
{
	// Color is unnecessary but useful for debugging, can be removed in the final product
	fColor = vin[0].vColor;
	//fColor = vec3(1.0, 1.0, 1.0);
	

	float x = vin[0].vOffset.x;
	float y = vin[0].vOffset.y;
	float z = vin[0].vOffset.z;

	vec4 position;
	// TODO: I'm sure there's a better way to check which coordinates to use
	if (y > 0.0) {
		// Calculate the position of the vetex to be emitted
		position = gl_in[0].gl_Position + vec4(-x, -y, -z, 0.0);
		// Translate it against the MVP matrix
		gl_Position = MVP * position;
		txCoord = getTxCoord(position, x, y);
		EmitVertex(); 			 
					  			 
		// Rinse, repeat
		position = gl_in[0].gl_Position + vec4( x, -y, z, 0.0);
		gl_Position = MVP * position;
		txCoord = getTxCoord(position, x, y);
		EmitVertex(); 			 
					  			 
		position = gl_in[0].gl_Position + vec4(-x,  y, -z, 0.0);
		gl_Position = MVP * position;
		txCoord = getTxCoord(position, x, y);
		EmitVertex(); 			 
								 
		position = gl_in[0].gl_Position + vec4( x,  y, z, 0.0);
		gl_Position = MVP * position;
		txCoord = getTxCoord(position, x, y);
		EmitVertex();

	} else {
		position = gl_in[0].gl_Position + vec4(-x, -y, -z, 0.0);
		gl_Position = MVP * position;
		txCoord = getTxCoord(position, x, y);
		EmitVertex(); 			 
					  			 
		position = gl_in[0].gl_Position + vec4( x, -y, -z, 0.0);
		gl_Position = MVP * position;
		txCoord = getTxCoord(position, x, y);
		EmitVertex(); 			 
					  			 
		position = gl_in[0].gl_Position + vec4(-x,  y, z, 0.0);
		gl_Position = MVP * position;
		txCoord = getTxCoord(position, x, y);
		EmitVertex(); 			 
								 
		position = gl_in[0].gl_Position + vec4( x,  y, z, 0.0);
		gl_Position = MVP * position;
		txCoord = getTxCoord(position, x, y);
		EmitVertex();
	}

	EndPrimitive();
}