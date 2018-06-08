#version 330 core
	
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in VOUT {
	flat highp int vFace;
	flat highp int vDepth;
} vin[];

out vec2 txCoord;
flat out highp int fFace;


uniform mat4 MVP;
uniform float TileWidth;
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

	// F/B want X/Y, R/L want Z/Y, U/D want X/Z
	// This allows us to swap the correct value out and return a vec2
	int idx = ((face + 4) % 6) / 2;

	// Only flip coordinates on Odd faces (Except if it's 4/5, then flip 4 and not 5)
	int i = (face >> 2) ^ (face & 1);

	pos[idx] = 0.0;
	if (i != 0) {
		// Face 1 flips X, Face 3/4 flip Z
		int n =  (( (face&2) | (face&4)) >> face-2) << 1; // Bitwise voodoo
		pos[n] = (segl * 1.0) - pos[n];
	}
	// Everything flips Y, if it doesn't use Y then it will be overriden
	pos[1] = (segl * 1.0) - pos[1];
	pos[idx] = pos[2];
	pos[2] = 0.0;
	return vec2(pos);
}


void main()
{
	highp int face = vin[0].vFace;

	// Front/Back need to set z to 0, Right/Left X to 0, Top/Bot Y to 0
	// Indicies 2, 0, 1 respectively.  Below mod math rotates around values around correctly.
	int idx = ((face + 4) % 6) / 2;
	vec3 Shift = vec3(TileWidth, TileWidth, TileWidth);
	Shift[idx] = 0.0;
	float x = Shift.x;
	float y = Shift.y;
	float z = Shift.z;

	vec4 position;
	// TODO: I'm sure there's a better way to check which coordinates to use
	if (y > 0.0) {
		// Calculate the position of the vetex to be emitted
		position = gl_in[0].gl_Position + vec4(-x, -y, -z, 0.0);

		// Translate against MVP matrix
		gl_Position = MVP * position;

		// Set our ST coords (x/y just refer to vector position 0/1, not literally x/y coords)
		txCoord = getST(position, face);

		// Tell the frag shader what face we are (and what texture to use)
		fFace = face;
		EmitVertex(); 			 
					  			 
		// Rinse, repeat
		position = gl_in[0].gl_Position + vec4( x, -y, z, 0.0);
		gl_Position = MVP * position;
		txCoord = getST(position, face);
		fFace = face;
		EmitVertex(); 			 
					  			 
		position = gl_in[0].gl_Position + vec4(-x,  y, -z, 0.0);
		gl_Position = MVP * position;
		txCoord = getST(position, face);
		fFace = face;
		EmitVertex(); 			 
								 
		position = gl_in[0].gl_Position + vec4( x,  y, z, 0.0);
		gl_Position = MVP * position;
		txCoord = getST(position, face);
		fFace = face;
		EmitVertex();

	} else {

		//TODO: The only difference in this branch is we have -z -z z z instead of -z z -z z in our offsets
		// Can we consolidate this somehow?

		position = gl_in[0].gl_Position + vec4(-x, -y, -z, 0.0);
		gl_Position = MVP * position;
		txCoord = getST(position, face);
		fFace = face;
		EmitVertex(); 			 
					  			 
		position = gl_in[0].gl_Position + vec4( x, -y, -z, 0.0);
		gl_Position = MVP * position;
		txCoord = getST(position, face);
		fFace = face;
		EmitVertex(); 			 
					  			 
		position = gl_in[0].gl_Position + vec4(-x,  y, z, 0.0);
		gl_Position = MVP * position;
		txCoord = getST(position, face);
		fFace = face;
		EmitVertex(); 			 
								 
		position = gl_in[0].gl_Position + vec4( x,  y, z, 0.0);
		gl_Position = MVP * position;
		txCoord = getST(position, face);
		fFace = face;
		EmitVertex();
	}

	EndPrimitive();
}