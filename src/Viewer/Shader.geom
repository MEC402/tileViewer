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


// Calculate these things ahead of time so we don't repeat them 4x

// F/B want X/Y, R/L want Z/Y, U/D want X/Z
// This allows us to swap the correct value out and return a vec2
int idx = ((vin[0].vFace + 4) % 6) / 2;

// Only flip coordinates on Odd faces (Except if it's 4/5, then flip 4 and not 5)
int flip = (vin[0].vFace >> 2) ^ (vin[0].vFace & 1);

// Get the coordinate to flip (0 for face 1, 2 for faces 3/4)
int n =  ((( (vin[0].vFace&2) | (vin[0].vFace&4)) >> vin[0].vFace-2) << 1) * flip;

vec2 getST(vec4 pos, int face)
{	
	pos = (pos + vec4(0.5, 0.5, 0.5, 1.0)) * segl;

	pos[idx] = 0.0;
	 // If flip, flip.  Otherwise retain the original pos[n] value.
	pos[n] = (((segl * 1.0) - pos[n])*flip) + ((flip^1) * pos[n]);

	// Everything flips Y, if it doesn't use Y then it will be replaced anyways
	pos[1] = (segl * 1.0) - pos[1];
	pos[idx] = pos[2];
	return vec2(pos);
}


void main()
{
	highp int face = vin[0].vFace;

	vec3 Shift = vec3(TileWidth, TileWidth, TileWidth);
	Shift[idx] = 0.0;
	float x = Shift.x;
	float y = Shift.y;
	float z = Shift.z;

	if (Debug) {
		segl = 1.0;
	}

	vec4 position;
	// TODO: I'm sure there's a better way to check which coordinates to use
	if (y > 0.0) {
		// Calculate the position of the vetex to be emitted
		position = gl_in[0].gl_Position + vec4(-x, -y, -z, 0.0);

		// Translate against MVP matrix
		gl_Position = MVP * position;

		// Set our ST coords
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