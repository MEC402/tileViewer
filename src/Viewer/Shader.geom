#version 330 core
	
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in VOUT {
	vec3 vColor;
	vec3 vOffset;
	float vFace;
} vin[];

out vec2 txCoord;
out vec3 fColor;	
out float fFace;


uniform mat4 MVP;
uniform vec2 TxScaling;

vec2 getTxCoord(vec4 pos, float face)
{
	pos = pos + vec4(0.5, 0.5, 0.5, 1.0);
	if (face == 0.0) { // Front
		return TxScaling * vec2(pos.x, -pos.y);
	}
	if (face == 1.0) { // Back
		return TxScaling * vec2(-pos.x, -pos.y);
	}
	if (face == 2.0) { // Right
		return TxScaling * vec2(pos.z, -pos.y);
	}
	if (face == 3.0) { // Left
		return TxScaling * vec2(-pos.z, -pos.y);
	}
	if (face == 4.0) { // Top
		return TxScaling * vec2(pos.x, -pos.z);
	}
	if (face == 5.0) { // Bottom
		return TxScaling * vec2(pos.x, pos.z);
	}
}

void main()
{
	// Color is unnecessary but useful for debugging, can be removed in the final product
	fColor = vin[0].vColor;
	//fFace = vin[0].vFace;
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
		txCoord = getTxCoord(position, vin[0].vFace);
		fFace = vin[0].vFace;
		EmitVertex(); 			 
					  			 
		// Rinse, repeat
		position = gl_in[0].gl_Position + vec4( x, -y, z, 0.0);
		gl_Position = MVP * position;
		txCoord = getTxCoord(position, vin[0].vFace);
		fFace = vin[0].vFace;
		EmitVertex(); 			 
					  			 
		position = gl_in[0].gl_Position + vec4(-x,  y, -z, 0.0);
		gl_Position = MVP * position;
		txCoord = getTxCoord(position, vin[0].vFace);
		fFace = vin[0].vFace;
		EmitVertex(); 			 
								 
		position = gl_in[0].gl_Position + vec4( x,  y, z, 0.0);
		gl_Position = MVP * position;
		txCoord = getTxCoord(position, vin[0].vFace);
		fFace = vin[0].vFace;
		EmitVertex();

	} else {
		position = gl_in[0].gl_Position + vec4(-x, -y, -z, 0.0);
		gl_Position = MVP * position;
		txCoord = getTxCoord(position, vin[0].vFace);
		fFace = vin[0].vFace;
		EmitVertex(); 			 
					  			 
		position = gl_in[0].gl_Position + vec4( x, -y, -z, 0.0);
		gl_Position = MVP * position;
		txCoord = getTxCoord(position, vin[0].vFace);
		fFace = vin[0].vFace;
		EmitVertex(); 			 
					  			 
		position = gl_in[0].gl_Position + vec4(-x,  y, z, 0.0);
		gl_Position = MVP * position;
		txCoord = getTxCoord(position, vin[0].vFace);
		fFace = vin[0].vFace;
		EmitVertex(); 			 
								 
		position = gl_in[0].gl_Position + vec4( x,  y, z, 0.0);
		gl_Position = MVP * position;
		txCoord = getTxCoord(position, vin[0].vFace);
		fFace = vin[0].vFace;
		EmitVertex();
	}

	EndPrimitive();
}