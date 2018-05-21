#version 330 core
	
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in VOUT {
	vec3 vColor;
	vec3 vOffset;
	flat highp int vFace;
	flat highp int vDepth;
} vin[];

out vec2 txCoord;
out vec3 fColor;	
flat out highp int fFace;


uniform mat4 MVP;
uniform bool Debug;

// For use in calculating ST coordinates
int wl = (0x01 << (3 - vin[0].vDepth));
float segl = 1.0 / (wl * 1.0);

vec2 getST(vec4 pos, int face)
{
	if (Debug) {
		segl = 1.0;
	}
	
	pos = (pos + vec4(0.5, 0.5, 0.5, 1.0)) * segl;

	// All the (1.0 * segl) - pos.n stuff is to flip ST coordinates across an axis without relying on wrapping
	switch (face) {
		case 0:
			pos.y = (1.0 * segl) - pos.y;
			return vec2(pos.x, pos.y);
		case 1:
			pos.x = (1.0 * segl) - pos.x;
			pos.y = (1.0 * segl) - pos.y;
			return vec2(pos.x, pos.y);
		case 2:
			pos.y = (1.0 * segl) - pos.y;
			return vec2(pos.z, pos.y);
		case 3:
			pos.y = (1.0 * segl) - pos.y;
			pos.z = (1.0 * segl) - pos.z;
			return vec2(pos.z, pos.y);
		case 4:
			pos.z = (1.0 * segl) - pos.z;
		case 5:
			return vec2(pos.x, pos.z);
	}
}


void main()
{
	// Color is unnecessary but useful for debugging, can be removed in the final product
	fColor = vin[0].vColor;
	//fColor = vec3(1.0, 0.5, 0.5)
	
	//TODO: We can optimize these values out since we know what face we're drawing a quad for, we don't need
	// to precalculate these CPU side.  We can just switch on them here based on the face and set the values
	// as appropriate.
	float x = vin[0].vOffset.x;
	float y = vin[0].vOffset.y;
	float z = vin[0].vOffset.z;

	vec4 position;
	// TODO: I'm sure there's a better way to check which coordinates to use
	if (y > 0.0) {
		// Calculate the position of the vetex to be emitted
		position = gl_in[0].gl_Position + vec4(-x, -y, -z, 0.0);

		// Translate against MVP matrix
		gl_Position = MVP * position;

		// Set our ST coords (x/y just refer to vector position 0/1, not literally x/y coords)
		txCoord = getST(position, vin[0].vFace);

		// Tell the frag shader what face we are (and what texture to use)
		fFace = vin[0].vFace;
		EmitVertex(); 			 
					  			 
		// Rinse, repeat
		position = gl_in[0].gl_Position + vec4( x, -y, z, 0.0);
		gl_Position = MVP * position;
		txCoord = getST(position, vin[0].vFace);
		fFace = vin[0].vFace;
		EmitVertex(); 			 
					  			 
		position = gl_in[0].gl_Position + vec4(-x,  y, -z, 0.0);
		gl_Position = MVP * position;
		txCoord = getST(position, vin[0].vFace);
		fFace = vin[0].vFace;
		EmitVertex(); 			 
								 
		position = gl_in[0].gl_Position + vec4( x,  y, z, 0.0);
		gl_Position = MVP * position;
		txCoord = getST(position, vin[0].vFace);
		fFace = vin[0].vFace;
		EmitVertex();

	} else {

		//TODO: The only difference in this branch is we have -z -z z z instead of -z z -z z in our offsets
		// Can we consolidate this somehow?

		position = gl_in[0].gl_Position + vec4(-x, -y, -z, 0.0);
		gl_Position = MVP * position;
		txCoord = getST(position, vin[0].vFace);
		fFace = vin[0].vFace;
		EmitVertex(); 			 
					  			 
		position = gl_in[0].gl_Position + vec4( x, -y, -z, 0.0);
		gl_Position = MVP * position;
		txCoord = getST(position, vin[0].vFace);
		fFace = vin[0].vFace;
		EmitVertex(); 			 
					  			 
		position = gl_in[0].gl_Position + vec4(-x,  y, z, 0.0);
		gl_Position = MVP * position;
		txCoord = getST(position, vin[0].vFace);
		fFace = vin[0].vFace;
		EmitVertex(); 			 
								 
		position = gl_in[0].gl_Position + vec4( x,  y, z, 0.0);
		gl_Position = MVP * position;
		txCoord = getST(position, vin[0].vFace);
		fFace = vin[0].vFace;
		EmitVertex();
	}

	EndPrimitive();
}